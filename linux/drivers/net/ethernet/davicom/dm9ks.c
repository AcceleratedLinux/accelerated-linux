/*

  dm9ks.c: Version 2.03 2005/10/17 
  
        A Davicom DM9000A/DM9010 ISA NIC fast Ethernet driver for Linux.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.


  (C)Copyright 1997-2005 DAVICOM Semiconductor,Inc. All Rights Reserved.

	
V1.00	10/13/2004	Add new function Early transmit & IP/TCP/UDP Checksum
			offload enable & flow control is default
V1.1	12/29/2004	Add Two packet mode & modify RX function
V1.2	01/14/2005	Add Early transmit mode 
V1.3	03/02/2005	Support kernel 2.6
v1.33   06/08/2005	#define DM9KS_MDRAL		0xf4
			#define DM9KS_MDRAH		0xf5
			
V2.00 Spenser - 01/10/2005
			- Modification for PXA270 MAINSTONE.
			- Modified dmfe_tx_done().
			- Add dmfe_timeout().
V2.01	10/07/2005	Modified dmfe_timer()
			Dected network speed 10/100M
V2.02	10/12/2005	Use link change to chage db->Speed
			dmfe_open() wait for Link OK  
V2.03	11/22/2005	Power-off and Power-on PHY in dmfe_init()
			support IOL
*/

#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

#include <linux/init.h>				
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <asm/dma.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>

#ifdef CONFIG_ARCH_MAINSTONE
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#endif

#ifdef CONFIG_EXCALIBUR
#include <asm/nios.h>
#include <asm/io.h>
#include <asm/irq.h>
#endif
#if defined(na_DM9000A) && !defined(na_dm9000)   // defs for DE2
#define na_dm9000 na_DM9000A
#define na_dm9000_irq na_DM9000A_irq
#endif

// #define ETRANS
#define CHECKSUM

/* Board/System/Debug information/definition ---------------- */

#define DM9KS_ID		0x90000A46
#define DM9010_ID		0x90100A46
/*-------register name-----------------------*/
#define DM9KS_NCR		0x00	/* Network control Reg.*/
#define DM9KS_NSR		0x01	/* Network Status Reg.*/
#define DM9KS_TCR		0x02	/* TX control Reg.*/
#define DM9KS_RXCR		0x05	/* RX control Reg.*/
#define DM9KS_BPTR		0x08
#define DM9KS_EPCR		0x0b
#define DM9KS_EPAR		0x0c
#define DM9KS_EPDRL		0x0d
#define DM9KS_EPDRH		0x0e
#define DM9KS_GPR		0x1f	/* General purpose register */
#define DM9KS_TCR2		0x2d
#define DM9KS_SMCR		0x2f 	/* Special Mode Control Reg.*/
#define DM9KS_ETXCSR		0x30	/* Early Transmit control/status Reg.*/
#define	DM9KS_TCCR		0x31	/* Checksum cntrol Reg. */
#define DM9KS_RCSR		0x32	/* Receive Checksum status Reg.*/
#define DM9KS_MRCMDX		0xf0
#define DM9KS_MRCMD		0xf2
#define DM9KS_MDRAL		0xf4
#define DM9KS_MDRAH		0xf5
#define DM9KS_MWCMD		0xf8
#define DM9KS_TXPLL		0xfc
#define DM9KS_TXPLH		0xfd
#define DM9KS_ISR		0xfe
#define DM9KS_IMR		0xff
/*---------------------------------------------*/
#define DM9KS_REG05		0x30	/* SKIP_CRC/SKIP_LONG */ 
#define DM9KS_REGFF		0xA3	/* IMR */
#define DM9KS_DISINTR		0x80

#define DM9KS_PHY		0x40	/* PHY address 0x01 */
#define DM9KS_PKT_RDY		0x01	/* Packet ready to receive */

/* Added for PXA of MAINSTONE */
#ifdef CONFIG_ARCH_MAINSTONE
#include <asm/arch/mainstone.h>
#define DM9KS_MIN_IO		(MST_ETH_PHYS + 0x300)
#define DM9KS_MAX_IO            (MST_ETH_PHYS + 0x370)
#define DM9K_IRQ		MAINSTONE_IRQ(3)
#elif defined(CONFIG_EXCALIBUR)
#define DM9KS_MIN_IO            (na_dm9000)
#define DM9KS_MAX_IO            (DM9KS_MIN_IO + 7)
#define DM9K_IRQ	        (na_dm9000_irq)
#else
#define DM9KS_MIN_IO		0x300
#define DM9KS_MAX_IO		0x370
#define DM9K_IRQ		3
#endif

#define DM9KS_VID_L		0x28
#define DM9KS_VID_H		0x29
#define DM9KS_PID_L		0x2A
#define DM9KS_PID_H		0x2B

#define DM9KS_RX_INTR		0x01
#define DM9KS_TX_INTR		0x02
#define DM9KS_LINK_INTR		0x20

#define DM9KS_DWORD_MODE	1
#define DM9KS_BYTE_MODE		2
#define DM9KS_WORD_MODE		0

#define TRUE			1
#define FALSE			0
/* Number of continuous Rx packets */
#define CONT_RX_PKT_CNT	10 

#define DMFE_TIMER_WUT  jiffies+(HZ*5)	/* timer wakeup time : 5 second */

#if defined(DM9KS_DEBUG)
#define DMFE_DBUG(dbug_now, msg, vaule)\
if (dmfe_debug||dbug_now) printk(KERN_ERR "dmfe: %s %x\n", msg, vaule)
#else
#define DMFE_DBUG(dbug_now, msg, vaule)\
if (dbug_now) printk(KERN_ERR "dmfe: %s %x\n", msg, vaule)
#endif

typedef struct _RX_DESC
{
	u8 rxbyte;
	u8 status;
	u16 length;
} __attribute__((__packed__)) RX_DESC;

typedef union{
	u8 buf[4];
	RX_DESC desc;
} __attribute__((__packed__)) rx_t;

enum DM9KS_PHY_mode {
	DM9KS_10MHD   = 0, 
	DM9KS_100MHD  = 1, 
	DM9KS_10MFD   = 4,
	DM9KS_100MFD  = 5, 
	DM9KS_AUTO    = 8, 
};

/* Structure/enum declaration ------------------------------- */
typedef struct board_info {
 
	u32 reset_counter;		/* counter: RESET */ 
	u32 reset_tx_timeout;		/* RESET caused by TX Timeout */ 

	u32 io_addr;			/* Register I/O base address */
	u32 io_data;			/* Data I/O address */
	int tx_pkt_cnt;

	u8 op_mode;			/* PHY operation mode */
	u8 io_mode;			/* 0:word, 2:byte */
	u8 device_wait_reset;		/* device state */
	u8 Speed;			/* current speed */

	int cont_rx_pkt_cnt;/* current number of continuos rx packets  */
	struct timer_list timer;
	struct net_device_stats stats;
	unsigned char srom[128];
	spinlock_t lock;

} board_info_t;
/* Global variable declaration ----------------------------- */
/* static int dmfe_debug = 1; */
static struct net_device * dmfe_dev = NULL;

/* For module input parameter */
static int mode       = DM9KS_AUTO;
static int media_mode = DM9KS_AUTO;
static u8  irq        = DM9K_IRQ;
static u32 iobase     = DM9KS_MIN_IO;

/* function declaration ------------------------------------- */
int dmfe_probe(struct net_device *);
static int dmfe_open(struct net_device *);
static int dmfe_start_xmit(struct sk_buff *, struct net_device *);
static void dmfe_tx_done(unsigned long);
static void dmfe_packet_receive(struct net_device *);
static int dmfe_stop(struct net_device *);
static struct net_device_stats * dmfe_get_stats(struct net_device *); 
static int dmfe_do_ioctl(struct net_device *, struct ifreq *, int);
static irqreturn_t dmfe_interrupt(int , void *);
static void dmfe_timer(unsigned long);
static void dmfe_init_dm9000(struct net_device *);
static unsigned long cal_CRC(unsigned char *, unsigned int, u8);
static u8 ior(board_info_t *, int);
static void iow(board_info_t *, int, u8);
static u16 phy_read(board_info_t *, int);
static void phy_write(board_info_t *, int, u16);
static u16 read_srom_word(board_info_t *, int);
static void dm9000_hash_table(struct net_device *);
static void dmfe_timeout(struct net_device *);
static void dmfe_reset(struct net_device *);

#if defined(CHECKSUM)
static u8 check_rx_ready(u8);
#endif

//DECLARE_TASKLET(dmfe_tx_tasklet,dmfe_tx_done,0);

/* DM9000 network baord routine ---------------------------- */

/*
  Search DM9000 board, allocate space and register it
*/

struct net_device * __init dm9ks_probe(void)
{
	struct net_device *dev;
	int err;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	dev = init_etherdev(NULL, sizeof(struct board_info));
	ether_setup(dev);		
#else
	dev= alloc_etherdev(sizeof(struct board_info));
#endif

	if(!dev)
		return ERR_PTR(-ENOMEM);

     	SET_MODULE_OWNER(dev);
	err = dmfe_probe(dev);
	if (err)
		goto out;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	err = register_netdev(dev);
	if (err)
		goto out1;
#endif
	return dev;
out1:
	release_region(dev->base_addr,2);
out:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	kfree(dev);
#else
	free_netdev(dev);
#endif
	return ERR_PTR(err);
}

int __init dmfe_probe(struct net_device *dev)
{
	struct board_info *db;    /* Point a board information structure */
	u32 id_val;
	u16 i, dm9000_found = FALSE;

	DMFE_DBUG(0, "dmfe_probe()",0);

	/* Search All DM9000 serial NIC */
	do {
		outb(DM9KS_VID_L, iobase);
		id_val = inb(iobase + 4);
		outb(DM9KS_VID_H, iobase);
		id_val |= inb(iobase + 4) << 8;
		outb(DM9KS_PID_L, iobase);
		id_val |= inb(iobase + 4) << 16;
		outb(DM9KS_PID_H, iobase);
		id_val |= inb(iobase + 4) << 24;

		if (id_val == DM9KS_ID || id_val == DM9010_ID) {
			
			/* Request IO from system */
			if(!request_region(iobase, 2, dev->name))
				return -ENODEV;

			printk("<DM9KS> I/O: %x, VID: %x \n",iobase, id_val);
			dm9000_found = TRUE;

			/* Allocated board information structure */
			memset(dev->priv, 0, sizeof(struct board_info));
			db = (board_info_t *)dev->priv;
			dmfe_dev    = dev;
			db->io_addr  = iobase;
			db->io_data = iobase + 4;
			/* driver system function */
				
			dev->base_addr 		= iobase;
			dev->irq 		= irq;
			dev->open 		= &dmfe_open;
			dev->hard_start_xmit 	= &dmfe_start_xmit;
			dev->watchdog_timeo	= HZ;	
			dev->tx_timeout		= dmfe_timeout;
			dev->stop 		= &dmfe_stop;
			dev->get_stats 		= &dmfe_get_stats;
			dev->set_multicast_list = &dm9000_hash_table;
			dev->do_ioctl 		= &dmfe_do_ioctl;

#if defined(CHECKSUM)
			dev->features = dev->features | NETIF_F_NO_CSUM;
#endif
#ifdef CONFIG_EXCALIBUR
			{
			  extern unsigned char *excalibur_enet_hwaddr;

			  memcpy(dev->dev_addr, excalibur_enet_hwaddr, 6);
			}
#else
			/* Read SROM content */
			for (i=0; i<64; i++)
				((u16 *)db->srom)[i] = read_srom_word(db, i);

			/* Set Node Address */
			for (i=0; i<6; i++)
				dev->dev_addr[i] = db->srom[i];
#endif
		}//end of if()
		iobase += 0x10;
	}while(!dm9000_found && iobase <= DM9KS_MAX_IO);

	return dm9000_found ? 0:-ENODEV;
}


/*
  Open the interface.
  The interface is opened whenever "ifconfig" actives it.
*/
static int dmfe_open(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	u8 reg_nsr;
	int i;
	DMFE_DBUG(0, "dmfe_open", 0);

	if (request_irq(dev->irq,&dmfe_interrupt,SA_SHIRQ,dev->name,dev)) 
		return -EAGAIN;

	/* Initilize DM910X board */
	dmfe_init_dm9000(dev);
 
	/* Init driver variable */
	db->reset_counter 	= 0;
	db->reset_tx_timeout 	= 0;
	db->cont_rx_pkt_cnt	= 0;
	
	/* check link state and media speed */
	db->Speed =10;
	i=0;
	do {
		reg_nsr = ior(db,0x1);
		if(reg_nsr & 0x40) /* link OK!! */
		{
			/* wait for detected Speed */
			mdelay(200);
			reg_nsr = ior(db,0x1);
			if(reg_nsr & 0x80)
				db->Speed =10;
			else
				db->Speed =100;
			break;
		}
		i++;
		mdelay(1);
	}while(i<3000);	/* wait 3 second  */
	//printk("i=%d  Speed=%d\n",i,db->Speed);	
	/* set and active a timer process */
	init_timer(&db->timer);
	db->timer.expires 	= DMFE_TIMER_WUT * 2;
	db->timer.data 		= (unsigned long)dev;
	db->timer.function 	= &dmfe_timer;
	add_timer(&db->timer);	//Move to DM9000 initiallization was finished.
 	
	netif_start_queue(dev);

	return 0;
}

/* Set PHY operationg mode
*/
static void set_PHY_mode(board_info_t *db)
{
	u16 phy_reg0 = 0x1200;		/* Auto-negotiation & Restart Auto-negotiation */
	u16 phy_reg4 = 0x01e1;		/* Default flow control disable*/

	if ( !(db->op_mode & DM9KS_AUTO) ) // op_mode didn't auto sense */
	{ 
		switch(db->op_mode) {
			case DM9KS_10MHD:  phy_reg4 = 0x21; 
                        	           phy_reg0 = 0x1000;
					   break;
			case DM9KS_10MFD:  phy_reg4 = 0x41; 
					   phy_reg0 = 0x1100;
                                	   break;
			case DM9KS_100MHD: phy_reg4 = 0x81; 
					   phy_reg0 = 0x3000;
				    	   break;
			case DM9KS_100MFD: phy_reg4 = 0x101; 
					   phy_reg0 = 0x3100;
				   	   break;
			default: 
					   break;
		} // end of switch
	} // end of if
	phy_write(db, 0, phy_reg0);
	phy_write(db, 4, phy_reg4);
}

/* 
	Initilize dm9000 board
*/
static void dmfe_init_dm9000(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_init_dm9000()", 0);

	/* set the internal PHY power-on, GPIOs normal, and wait 2ms */
	iow(db, DM9KS_GPR, 1); 	/* Power-Down PHY */
	udelay(500);
	iow(db, DM9KS_GPR, 0);	/* GPR (reg_1Fh)bit GPIO0=0 pre-activate PHY */
	udelay(20);		/* wait 2ms for PHY power-on ready */

	/* do a software reset and wait 20us */
	iow(db, DM9KS_NCR, 3);
	udelay(20);		/* wait 20us at least for software reset ok */
	iow(db, DM9KS_NCR, 3);	/* NCR (reg_00h) bit[0] RST=1 & Loopback=1, reset on */
	udelay(20);		/* wait 20us at least for software reset ok */

	/* I/O mode */
	db->io_mode = ior(db, DM9KS_ISR) >> 6; /* ISR bit7:6 keeps I/O mode */

	/* Set PHY */
	db->op_mode = media_mode;
	set_PHY_mode(db);

	/* Program operating register */
	iow(db, DM9KS_NCR, 0);
	iow(db, DM9KS_TCR, 0);		/* TX Polling clear */
	iow(db, DM9KS_BPTR, 0x3f);	/* Less 3kb, 600us */
	iow(db, DM9KS_SMCR, 0);		/* Special Mode */
	iow(db, DM9KS_NSR, 0x2c);	/* clear TX status */
	iow(db, DM9KS_ISR, 0x0f); 	/* Clear interrupt status */

	/* Added by jackal at 03/29/2004 */
#if defined(CHECKSUM)
	iow(db, DM9KS_TCCR, 0x07);	/* TX UDP/TCP/IP checksum enable */
	iow(db, DM9KS_RCSR, 0x02);	/*Receive checksum enable */
#endif

#if defined(ETRANS)
	iow(db, DM9KS_ETXCSR, 0x83);
#endif
 
	/* Set address filter table */
	dm9000_hash_table(dev);

	/* Activate DM9000A/DM9010 */
	iow(db, DM9KS_RXCR, DM9KS_REG05 | 1);	/* RX enable */
	iow(db, DM9KS_IMR, DM9KS_REGFF); 	// Enable TX/RX interrupt mask
 
	/* Init Driver variable */
	db->tx_pkt_cnt 		= 0;
		
	netif_carrier_on(dev);
	spin_lock_init(&db->lock);

}

/*
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
static int dmfe_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	char * data_ptr;
	int i, tmplen;
	if(db->Speed == 10)
		{if (db->tx_pkt_cnt >= 1) return 1;}
	else
		{if (db->tx_pkt_cnt >= 2) return 1;}
	
	/* packet counting */
	db->tx_pkt_cnt++;

	db->stats.tx_packets++;
	db->stats.tx_bytes+=skb->len;
	if (db->Speed == 10)
		{if (db->tx_pkt_cnt >= 1) netif_stop_queue(dev);}
	else
		{if (db->tx_pkt_cnt >= 2) netif_stop_queue(dev);}

	/* Disable all interrupt */
	iow(db, DM9KS_IMR, DM9KS_DISINTR);

	/* Set TX length to reg. 0xfc & 0xfd */
	iow(db, DM9KS_TXPLL, (skb->len & 0xff));
	iow(db, DM9KS_TXPLH, (skb->len >> 8) & 0xff);

	/* Move data to TX SRAM */
	data_ptr = (char *)skb->data;
	
	outb(DM9KS_MWCMD, db->io_addr); // Write data into SRAM trigger
	switch(db->io_mode)
	{
		case DM9KS_BYTE_MODE:
			for (i = 0; i < skb->len; i++)
				outb((data_ptr[i] & 0xff), db->io_data);
			break;
		case DM9KS_WORD_MODE:
			tmplen = (skb->len + 1) / 2;
			for (i = 0; i < tmplen; i++)
         			outw(((u16 *)data_ptr)[i], db->io_data);
         		break;
         	case DM9KS_DWORD_MODE:
         		tmplen = (skb->len + 3) / 4;			
			for (i = 0; i< tmplen; i++)
				outl(((u32 *)data_ptr)[i], db->io_data);
			break;
	}
	
#if !defined(ETRANS)
	/* Issue TX polling command */
	iow(db, DM9KS_TCR, 0x1); /* Cleared after TX complete*/
#endif

	/* Saved the time stamp */
	dev->trans_start = jiffies;
	db->cont_rx_pkt_cnt =0;

	/* Free this SKB */
	dev_kfree_skb(skb);

	/* Re-enable interrupt */
	iow(db, DM9KS_IMR, DM9KS_REGFF);

	return 0;
}

/*
  Stop the interface.
  The interface is stopped when it is brought.
*/
static int dmfe_stop(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_stop", 0);

	/* deleted timer */
	del_timer(&db->timer);

	netif_stop_queue(dev); 

	/* free interrupt */
	free_irq(dev->irq, dev);

	/* RESET devie */
	phy_write(db, 0x00, 0x8000);	/* PHY RESET */
	iow(db, DM9KS_GPR, 0x01); 	/* Power-Down PHY */
	iow(db, DM9KS_IMR, DM9KS_DISINTR);	/* Disable all interrupt */
	iow(db, DM9KS_RXCR, 0x00);	/* Disable RX */

	/* Dump Statistic counter */
#if FALSE
	printk("\nRX FIFO OVERFLOW %lx\n", db->stats.rx_fifo_errors);
	printk("RX CRC %lx\n", db->stats.rx_crc_errors);
	printk("RX LEN Err %lx\n", db->stats.rx_length_errors);
	printk("RESET %x\n", db->reset_counter);
	printk("RESET: TX Timeout %x\n", db->reset_tx_timeout);
	printk("g_TX_nsr %x\n", g_TX_nsr);
#endif

	return 0;
}

static void dmfe_tx_done(unsigned long unused)
{
	struct net_device *dev = dmfe_dev;
	board_info_t *db = (board_info_t *)dev->priv;
	int  nsr;

	DMFE_DBUG(0, "dmfe_tx_done()", 0);
	
	nsr = ior(db, DM9KS_NSR);
	if(nsr & 0x04) db->tx_pkt_cnt--;
	if(nsr & 0x08) db->tx_pkt_cnt--;

	if (db->tx_pkt_cnt < 0)
	{
		printk("[dmfe_tx_done] tx_pkt_cnt ERROR!!\n");
		db->tx_pkt_cnt =0;
	}
	if (db->Speed == 10)
		{if(db->tx_pkt_cnt < 1 ) netif_wake_queue(dev);}
	else		
		{if(db->tx_pkt_cnt < 2 ) netif_wake_queue(dev);}
	
	return;
}

/*
  DM9000 insterrupt handler
  receive the packet to upper layer, free the transmitted packet
*/
static irqreturn_t dmfe_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	board_info_t *db;
	int int_status,i;
	u8 reg_save;

	DMFE_DBUG(0, "dmfe_interrupt()", 0);

	/* A real interrupt coming */
	db = (board_info_t *)dev->priv;
	spin_lock(&db->lock);

	/* Save previous register address */
	reg_save = inb(db->io_addr);

	/* Disable all interrupt */
	iow(db, DM9KS_IMR, DM9KS_DISINTR); 

	/* Got DM9000A/DM9010 interrupt status */
	int_status = ior(db, DM9KS_ISR);		/* Got ISR */
	iow(db, DM9KS_ISR, int_status);		/* Clear ISR status */ 

	/* Link status change */
	if (int_status & DM9KS_LINK_INTR) 
	{
		netif_stop_queue(dev);
		for(i=0; i<500; i++) /*wait link OK, waiting time =0.5s */
		{
			phy_read(db,0x1);
			if(phy_read(db,0x1) & 0x4) /*Link OK*/
			{
				/* wait for detected Speed */
				for(i=0; i<200;i++)
					udelay(1000);
				/* set media speed */
				if(phy_read(db,0)&0x2000) db->Speed =100;
				else db->Speed =10;
				break;
			}
			udelay(1000);
		}
		netif_wake_queue(dev);
		//printk("[INTR]i=%d speed=%d\n",i, (int)(db->Speed));	
	}
	/* Received the coming packet */
	if (int_status & DM9KS_RX_INTR) 
		dmfe_packet_receive(dev);

	/* Trnasmit Interrupt check */
	if (int_status & DM9KS_TX_INTR)
		dmfe_tx_done(0);
	
	if (db->cont_rx_pkt_cnt>=CONT_RX_PKT_CNT)
	{
		iow(db, DM9KS_IMR, 0xa2);
	}
	else
	{
		/* Re-enable interrupt mask */ 
		iow(db, DM9KS_IMR, DM9KS_REGFF);
	}
	
	/* Restore previous register address */
	outb(reg_save, db->io_addr); 

	spin_unlock(&db->lock); 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	return IRQ_HANDLED;
#endif
}

/*
  Get statistics from driver.
*/
static struct net_device_stats * dmfe_get_stats(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_get_stats", 0);
	return &db->stats;
}

/*
  Process the upper socket ioctl command
*/
static int dmfe_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	DMFE_DBUG(0, "dmfe_do_ioctl()", 0);
	return 0;
}

/* Our watchdog timed out. Called by the networking layer */
static void
dmfe_timeout(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;

	DMFE_DBUG(0, "dmfe_TX_timeout()", 0);
	printk("TX time-out -- dmfe_timeout().\n");
	db->reset_tx_timeout++;
	db->stats.tx_errors++;
#if FALSE
	printk("TX packet count = %d\n", db->tx_pkt_cnt);	
	printk("TX timeout = %d\n", db->reset_tx_timeout);	
	printk("22H=0x%02x  23H=0x%02x\n",ior(db,0x22),ior(db,0x23));
	printk("faH=0x%02x  fbH=0x%02x\n",ior(db,0xfa),ior(db,0xfb));
#endif
	dmfe_reset(dev);
	
}

static void dmfe_reset(struct net_device * dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	u8 reg_save;
	int i;
	/* Save previous register address */
	reg_save = inb(db->io_addr);

	netif_stop_queue(dev); 
	db->reset_counter++;
	dmfe_init_dm9000(dev);
	
	db->Speed =10;
	for(i=0; i<1000; i++) /*wait link OK, waiting time=1 second */
	{
		if(phy_read(db,0x1) & 0x4) /*Link OK*/
		{
			if(phy_read(db,0)&0x2000) db->Speed =100;
			else db->Speed =10;
			break;
		}
		udelay(1000);
	}
	
	netif_wake_queue(dev);
	
	/* Restore previous register address */
	outb(reg_save, db->io_addr);

}
/*
  A periodic timer routine
*/
static void dmfe_timer(unsigned long data)
{
	struct net_device * dev = (struct net_device *)data;
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_timer()", 0);
	
	if (db->cont_rx_pkt_cnt>=CONT_RX_PKT_CNT)
	{
		db->cont_rx_pkt_cnt=0;
		iow(db, DM9KS_IMR, DM9KS_REGFF);
	}
	/* Set timer again */
	db->timer.expires = DMFE_TIMER_WUT;
	add_timer(&db->timer);
	
	return;
}

#if !defined(CHECKSUM)
#define check_rx_ready(a)	((a) == 0x01)
#else
inline u8 check_rx_ready(u8 rxbyte)
{
	if (!(rxbyte & 0x01))
		return 0;
	return ((rxbyte >> 4) | 0x01);
}
#endif

/*
  Received a packet and pass to upper layer
*/
static void dmfe_packet_receive(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	struct sk_buff *skb;
	u8 rxbyte, val;
	u16 i, GoodPacket, tmplen = 0, MDRAH, MDRAL;
	u32 tmpdata;

	rx_t rx;

	u16 * ptr = (u16*)&rx;
	u8* rdptr;

	DMFE_DBUG(0, "dmfe_packet_receive()", 0);

	do {
		/*store the value of Memory Data Read address register*/
		MDRAH=ior(db, DM9KS_MDRAH);
		MDRAL=ior(db, DM9KS_MDRAL);
		
		ior(db, DM9KS_MRCMDX);		/* Dummy read */
		rxbyte = inb(db->io_data);	/* Got most updated data */

		/* packet ready to receive check */
		if(!(val = check_rx_ready(rxbyte))) break;

		/* A packet ready now  & Get status/length */
		GoodPacket = TRUE;
		outb(DM9KS_MRCMD, db->io_addr);

		/* Read packet status & length */
		switch (db->io_mode) 
			{
			  case DM9KS_BYTE_MODE: 
 				    *ptr = inb(db->io_data) + 
				               (inb(db->io_data) << 8);
				    *(ptr+1) = inb(db->io_data) + 
					    (inb(db->io_data) << 8);
				    break;
			  case DM9KS_WORD_MODE:
				    *ptr = inw(db->io_data);
				    *(ptr+1)    = inw(db->io_data);
				    break;
			  case DM9KS_DWORD_MODE:
				    tmpdata  = inl(db->io_data);
				    *ptr = tmpdata;
				    *(ptr+1)    = tmpdata >> 16;
				    break;
			  default:
				    break;
			}

		/* Packet status check */
		if (rx.desc.status & 0xbf)
		{
			GoodPacket = FALSE;
			if (rx.desc.status & 0x01) 
			{
				db->stats.rx_fifo_errors++;
				printk("<RX FIFO error>\n");
			}
			if (rx.desc.status & 0x02) 
			{
				db->stats.rx_crc_errors++;
				printk("<RX CRC error>\n");
			}
			if (rx.desc.status & 0x80) 
			{
				db->stats.rx_length_errors++;
				printk("<RX Length error>\n");
			}
			if (rx.desc.status & 0x08)
				printk("<Physical Layer error>\n");
		}

		if (!GoodPacket)
		{
			// drop this packet!!!
			switch (db->io_mode)
			{
				case DM9KS_BYTE_MODE:
			 		for (i=0; i<rx.desc.length; i++)
						inb(db->io_data);
					break;
				case DM9KS_WORD_MODE:
					tmplen = (rx.desc.length + 1) / 2;
					for (i = 0; i < tmplen; i++)
						inw(db->io_data);
					break;
				case DM9KS_DWORD_MODE:
					tmplen = (rx.desc.length + 3) / 4;
					for (i = 0; i < tmplen; i++)
						inl(db->io_data);
					break;
			}
			continue;/*next the packet*/
		}
		
		skb = dev_alloc_skb(rx.desc.length+4);
		if (skb == NULL )
		{	
			printk(KERN_INFO "%s: Memory squeeze.\n", dev->name);
			/*re-load the value into Memory data read address register*/
			iow(db,DM9KS_MDRAH,MDRAH);
			iow(db,DM9KS_MDRAL,MDRAL);
			return;
		}
		else
		{
			/* Move data from DM9000 */
			skb->dev = dev;
			skb_reserve(skb, 2);
			rdptr = (u8*)skb_put(skb, rx.desc.length - 4);
			
			/* Read received packet from RX SARM */
			switch (db->io_mode)
			{
				case DM9KS_BYTE_MODE:
			 		for (i=0; i<rx.desc.length; i++)
						rdptr[i]=inb(db->io_data);
					break;
				case DM9KS_WORD_MODE:
					tmplen = (rx.desc.length + 1) / 2;
					for (i = 0; i < tmplen; i++)
						((u16 *)rdptr)[i] = inw(db->io_data);
					break;
				case DM9KS_DWORD_MODE:
					tmplen = (rx.desc.length + 3) / 4;
					for (i = 0; i < tmplen; i++)
						((u32 *)rdptr)[i] = inl(db->io_data);
					break;
			}
		
			/* Pass to upper layer */
			skb->protocol = eth_type_trans(skb,dev);
#if defined(CHECKSUM)
			if (val == 0x01)
				skb->ip_summed = CHECKSUM_UNNECESSARY;
#endif
			netif_rx(skb);
			dev->last_rx=jiffies;
			db->stats.rx_packets++;
			db->stats.rx_bytes += rx.desc.length;
			db->cont_rx_pkt_cnt++;
				
			if (db->cont_rx_pkt_cnt>=CONT_RX_PKT_CNT)
			{
				dmfe_tx_done(0);
				break;
			}
		}
			
	}while((rxbyte & 0x01) == DM9KS_PKT_RDY);
	DMFE_DBUG(0, "[END]dmfe_packet_receive()", 0);
	
}

/*
  Read a word data from SROM
*/
static u16 read_srom_word(board_info_t *db, int offset)
{
	iow(db, DM9KS_EPAR, offset);
	iow(db, DM9KS_EPCR, 0x4);
	udelay(200);
	iow(db, DM9KS_EPCR, 0x0);
	return (ior(db, DM9KS_EPDRL) + (ior(db, DM9KS_EPDRH) << 8) );
}

/*
  Set DM9000A/DM9010 multicast address
*/
static void dm9000_hash_table(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	struct dev_mc_list *mcptr = dev->mc_list;
	int mc_cnt = dev->mc_count;
	u32 hash_val;
	u16 i, oft, hash_table[4];

	DMFE_DBUG(0, "dm9000_hash_table()", 0);

	/* Set Node address */
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		iow(db, oft, dev->dev_addr[i]);

	/* Clear Hash Table */
	for (i = 0; i < 4; i++)
		hash_table[i] = 0x0;

	/* broadcast address */
	hash_table[3] = 0x8000;

	/* the multicast address in Hash Table : 64 bits */
	for (i = 0; i < mc_cnt; i++, mcptr = mcptr->next) {
		hash_val = cal_CRC((char *)mcptr->dmi_addr, 6, 0) & 0x3f; 
		hash_table[hash_val / 16] |= (u16) 1 << (hash_val % 16);
	}

	/* Write the hash table to MAC MD table */
	for (i = 0, oft = 0x16; i < 4; i++) {
		iow(db, oft++, hash_table[i] & 0xff);
		iow(db, oft++, (hash_table[i] >> 8) & 0xff);
	}
}

/*
  Calculate the CRC valude of the Rx packet
  flag = 1 : return the reverse CRC (for the received packet CRC)
         0 : return the normal CRC (for Hash Table index)
*/
static unsigned long cal_CRC(unsigned char * Data, unsigned int Len, u8 flag)
{
	
	u32 crc = ether_crc_le(Len, Data);

	if (flag) 
		return ~crc;
		
	return crc;
	 
}

/*
   Read a byte from I/O port
*/
static u8 ior(board_info_t *db, int reg)
{
	outb(reg, db->io_addr);
	return inb(db->io_data);
}

/*
   Write a byte to I/O port
*/
static void iow(board_info_t *db, int reg, u8 value)
{
	outb(reg, db->io_addr);
	outb(value, db->io_data);
}

/*
   Read a word from phyxcer
*/
static u16 phy_read(board_info_t *db, int reg)
{
	/* Fill the phyxcer register into REG_0C */
	iow(db, DM9KS_EPAR, DM9KS_PHY | reg);

	iow(db, DM9KS_EPCR, 0xc); 	/* Issue phyxcer read command */
	udelay(100);			/* Wait read complete */
	iow(db, DM9KS_EPCR, 0x0); 	/* Clear phyxcer read command */

	/* The read data keeps on REG_0D & REG_0E */
	return ( ior(db, DM9KS_EPDRH) << 8 ) | ior(db, DM9KS_EPDRL);
	
}

/*
   Write a word to phyxcer
*/
static void phy_write(board_info_t *db, int reg, u16 value)
{
	/* Fill the phyxcer register into REG_0C */
	iow(db, DM9KS_EPAR, DM9KS_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	iow(db, DM9KS_EPDRL, (value & 0xff));
	iow(db, DM9KS_EPDRH, ( (value >> 8) & 0xff));

	iow(db, DM9KS_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);			/* Wait write complete */
	iow(db, DM9KS_EPCR, 0x0);	/* Clear phyxcer write command */
}


#ifdef MODULE

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Davicom DM9000A/DM9010 ISA/uP Fast Ethernet Driver");
MODULE_PARM(mode, "i");
MODULE_PARM(irq, "i");
MODULE_PARM(iobase, "i");
MODULE_PARM_DESC(mode,"Media Speed, 0:10MHD, 1:10MFD, 4:100MHD, 5:100MFD");
MODULE_PARM_DESC(irq,"EtherLink IRQ number");
MODULE_PARM_DESC(iobase, "EtherLink I/O base address");

/* Description: 
   when user used insmod to add module, system invoked init_module()
   to initilize and register.
*/
int init_module(void)
{
	switch(mode) {
		case DM9KS_10MHD:
		case DM9KS_100MHD:
		case DM9KS_10MFD:
		case DM9KS_100MFD:
			media_mode = mode;
			break;
		default:
			media_mode = DM9KS_AUTO;
	}
	dmfe_dev = dm9ks_probe();
	if(IS_ERR(dmfe_dev))
		return PTR_ERR(dmfe_dev);
	return 0;
}
/* Description: 
   when user used rmmod to delete module, system invoked clean_module()
   to  un-register DEVICE.
*/
void cleanup_module(void)
{
	struct net_device *dev = dmfe_dev;
	DMFE_DBUG(0, "clean_module()", 0);

	unregister_netdev(dmfe_dev);
	release_region(dev->base_addr, 2);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	kfree(dev);
#else
	free_netdev(dev);
#endif
	
	DMFE_DBUG(0, "clean_module() exit", 0);
}
#endif

