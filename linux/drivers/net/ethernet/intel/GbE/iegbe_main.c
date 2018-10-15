/************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Embedded.L.1.0.3-144

  Contact Information:

  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226

**************************************************************/
/**************************************************************************
 * @ingroup IEGBE_GENERAL
 *
 * @file iegbe_main.c
 *
 * @description
 *   This module contains the upper-edge routines of the driver
 *   interface that handle initialization, resets, and shutdowns.
 *
 **************************************************************************/

#include "iegbe.h"
#include "gcu_if.h"

char iegbe_driver_name[] = "iegbe";
char iegbe_driver_string[] = "Gigabit Ethernet Controller Driver";
#ifndef CONFIG_E1000_NAPI
#define DRIVERNAPI
#else
#define DRIVERNAPI "-NAPI"
#endif
#define DRV_VERSION "1.0.3"DRIVERNAPI
char iegbe_driver_version[] = DRV_VERSION;
char iegbe_copyright[] = "Copyright (c) 1999-2007 Intel Corporation.";

#define E1000_FIFO_HDR            0x10
#define E1000_82547_PAD_LEN        0x3E0
#define MINIMUM_DHCP_PACKET_SIZE 282
#define TXD_USE_COUNT(S, X) (((S) >> (X)) + 1 )
#define E1000_CTRL_EN_PHY_PWR_MGMT 0x00200000

/* iegbe_pci_tbl - PCI Device ID Table
 *
 * Last entry must be all 0s
 *
 * Macro expands to...
 *   {PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}
 */
static struct pci_device_id iegbe_pci_tbl[] = {
	INTEL_E1000_ETHERNET_DEVICE(0x5040),
	INTEL_E1000_ETHERNET_DEVICE(0x5041),
	INTEL_E1000_ETHERNET_DEVICE(0x5042),
	INTEL_E1000_ETHERNET_DEVICE(0x5043),
  	INTEL_E1000_ETHERNET_DEVICE(0x5044),
	INTEL_E1000_ETHERNET_DEVICE(0x5045),
	INTEL_E1000_ETHERNET_DEVICE(0x5046),
	INTEL_E1000_ETHERNET_DEVICE(0x5047),
  	INTEL_E1000_ETHERNET_DEVICE(0x5048),
	INTEL_E1000_ETHERNET_DEVICE(0x5049),
	INTEL_E1000_ETHERNET_DEVICE(0x504A),
	INTEL_E1000_ETHERNET_DEVICE(0x504B),
	/* required last entry */
	{0,}
};

MODULE_DEVICE_TABLE(pci, iegbe_pci_tbl);

DEFINE_SPINLOCK(print_lock);

int iegbe_up(struct iegbe_adapter *adapter);
void iegbe_down(struct iegbe_adapter *adapter);
void iegbe_reset(struct iegbe_adapter *adapter);
int iegbe_set_spd_dplx(struct iegbe_adapter *adapter, uint16_t spddplx);
int iegbe_setup_all_tx_resources(struct iegbe_adapter *adapter);
int iegbe_setup_all_rx_resources(struct iegbe_adapter *adapter);
void iegbe_free_all_tx_resources(struct iegbe_adapter *adapter);
void iegbe_free_all_rx_resources(struct iegbe_adapter *adapter);
int iegbe_setup_tx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_tx_ring *txdr);
int iegbe_setup_rx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_rx_ring *rxdr);
void iegbe_free_tx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_tx_ring *tx_ring);
void iegbe_free_rx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_rx_ring *rx_ring);
void iegbe_update_stats(struct iegbe_adapter *adapter);

static int iegbe_init_module(void);
static void iegbe_exit_module(void);
static int iegbe_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit iegbe_remove(struct pci_dev *pdev);
static int iegbe_alloc_queues(struct iegbe_adapter *adapter);
#ifdef CONFIG_E1000_MQ
static void iegbe_setup_queue_mapping(struct iegbe_adapter *adapter);
#endif
static int iegbe_sw_init(struct iegbe_adapter *adapter);
static int iegbe_open(struct net_device *netdev);
static int iegbe_close(struct net_device *netdev);
static void iegbe_configure_tx(struct iegbe_adapter *adapter);
static void iegbe_configure_rx(struct iegbe_adapter *adapter);
static void iegbe_setup_rctl(struct iegbe_adapter *adapter);
static void iegbe_clean_all_tx_rings(struct iegbe_adapter *adapter);
static void iegbe_clean_all_rx_rings(struct iegbe_adapter *adapter);
static void iegbe_clean_tx_ring(struct iegbe_adapter *adapter,
                                struct iegbe_tx_ring *tx_ring);
static void iegbe_clean_rx_ring(struct iegbe_adapter *adapter,
                                struct iegbe_rx_ring *rx_ring);
static void iegbe_set_multi(struct net_device *netdev);
static void iegbe_update_phy_info(unsigned long data);
static void iegbe_watchdog(unsigned long data);
static void iegbe_82547_tx_fifo_stall(unsigned long data);
static int iegbe_xmit_frame(struct sk_buff *skb, struct net_device *netdev);
static struct net_device_stats * iegbe_get_stats(struct net_device *netdev);
static int iegbe_change_mtu(struct net_device *netdev, int new_mtu);
static int iegbe_set_mac(struct net_device *netdev, void *p);
#ifdef CONFIG_IRQ_PASS_REGS
static irqreturn_t iegbe_intr(int irq, void *data, struct pt_regs *regs);
#else
static irqreturn_t iegbe_intr(int irq, void *data);
#endif

void iegbe_tasklet(unsigned long);

static boolean_t iegbe_clean_tx_irq(struct iegbe_adapter *adapter,
                                    struct iegbe_tx_ring *tx_ring);

#ifdef CONFIG_E1000_NAPI
static int iegbe_clean(struct net_device *poll_dev, int *budget);
static boolean_t iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                                    struct iegbe_rx_ring *rx_ring,
                                    int *work_done, int work_to_do);
static boolean_t iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                                       struct iegbe_rx_ring *rx_ring,
                                       int *work_done, int work_to_do);
#else
static boolean_t iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                                    struct iegbe_rx_ring *rx_ring);
static boolean_t iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                                       struct iegbe_rx_ring *rx_ring);
#endif

static void iegbe_alloc_rx_buffers(struct iegbe_adapter *adapter,
                                   struct iegbe_rx_ring *rx_ring);
static void iegbe_alloc_rx_buffers_ps(struct iegbe_adapter *adapter,
                                      struct iegbe_rx_ring *rx_ring);

static int iegbe_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
#ifdef SIOCGMIIPHY
static int iegbe_mii_ioctl(struct net_device *netdev, struct ifreq *ifr,
			   int cmd);
#endif
void set_ethtool_ops(struct net_device *netdev);
extern int ethtool_ioctl(struct ifreq *ifr);
static void iegbe_enter_82542_rst(struct iegbe_adapter *adapter);
static void iegbe_leave_82542_rst(struct iegbe_adapter *adapter);
static void iegbe_tx_timeout(struct net_device *dev);
#ifdef INITWORK21
static void iegbe_tx_timeout_task(struct work_struct *work);
#else
static void iegbe_tx_timeout_task(struct net_device *dev);
#endif
static void iegbe_smartspeed(struct iegbe_adapter *adapter);

#ifdef NETIF_F_HW_VLAN_TX
static bool iegbe_vlan_used(struct iegbe_adapter *adapter);
static void iegbe_vlan_filter_on_off(struct iegbe_adapter *adapter,
				     bool filter_on);
static void iegbe_vlan_rx_add_vid(struct net_device *netdev, uint16_t vid);
static void iegbe_vlan_rx_kill_vid(struct net_device *netdev, uint16_t vid);
static void iegbe_restore_vlan(struct iegbe_adapter *adapter);
#endif

static int iegbe_notify_reboot(struct notifier_block *,
                               unsigned long event,
                               void *ptr);
static int iegbe_suspend(struct pci_dev *pdev, pm_message_t state);
#ifdef CONFIG_PM
static int iegbe_resume(struct pci_dev *pdev);
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/* for netdump / net console */
static void iegbe_netpoll (struct net_device *netdev);
#endif

#ifdef CONFIG_E1000_MQ
/* for multiple Rx queues */
void iegbe_rx_schedule(void *data);
#endif

struct notifier_block iegbe_notifier_reboot = {
	.notifier_call	= iegbe_notify_reboot,
	.next		= NULL,
	.priority	= 0
};

/* Exported from other modules */

extern void iegbe_check_options(struct iegbe_adapter *adapter);

static struct pci_driver iegbe_driver = {
	.name     = iegbe_driver_name,
	.id_table = iegbe_pci_tbl,
	.probe    = iegbe_probe,
	.remove   = __devexit_p(iegbe_remove),
	/* Power Managment Hooks */
#ifdef CONFIG_PM
	.suspend  = iegbe_suspend,
	.resume   = iegbe_resume
#endif
};

MODULE_AUTHOR("Intel(R) Corporation");
MODULE_DESCRIPTION("Gigabit Ethernet Controller Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

static int debug = NETIF_MSG_DRV | NETIF_MSG_PROBE;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

static uint8_t gcu_suspend = 0;
static uint8_t gcu_resume = 0;
struct pci_dev *gcu = NULL;

unsigned long tasklet_data;
DECLARE_TASKLET(iegbe_reset_tasklet, iegbe_tasklet, (unsigned long) &tasklet_data);

/**
 * iegbe_iegbe_tasklet -*
 **/
void iegbe_tasklet(unsigned long data)
{
   char* err_msg = "TEST";
	uint32_t *icr = (uint32_t*) data;
	uint32_t gbe = *icr & 0x000000FF;
	if( *icr & E1000_ICR_RX_DESC_FIFO_PAR) { /* 21 */
		 err_msg = "DMA Transmit Descriptor 2-bit ECC Error!";
	}
	if( *icr & E1000_ICR_TX_DESC_FIFO_PAR) { /* 20 */
		 err_msg = "DMA Receive Descriptor 2-bit ECC Error!";
	}
	if( *icr & E1000_ICR_PB) { /* 23 */
		 err_msg = "DMA Packet Buffer 2-bit ECC Error!";
	}
	if( *icr & E1000_ICR_CPP_TARGET) { /* 27 */
		 err_msg = "Statistic Register ECC Error!";
	}
	if( *icr & E1000_ICR_CPP_MASTER) {
		 err_msg = "CPP Error!";
	}
	   spin_lock(&print_lock);
		printk("IEGBE%d: System Reset due to: %s\n", gbe, err_msg);
		dump_stack();
	   spin_unlock(&print_lock);
		panic(err_msg);
	return;
}
/**
 * iegbe_init_module - Driver Registration Routine
 *
 * iegbe_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/

static int __init
iegbe_init_module(void)
{
	int ret;

    printk(KERN_INFO "%s - version %s\n",
	       iegbe_driver_string, iegbe_driver_version);

	printk(KERN_INFO "%s\n", iegbe_copyright);

	ret = pci_module_init(&iegbe_driver);
	if(ret >= 0) {
		register_reboot_notifier(&iegbe_notifier_reboot);
	}
	return ret;
}

module_init(iegbe_init_module);

/**
 * iegbe_exit_module - Driver Exit Cleanup Routine
 *
 * iegbe_exit_module is called just before the driver is removed
 * from memory.
 **/

static void __exit
iegbe_exit_module(void)
{

	unregister_reboot_notifier(&iegbe_notifier_reboot);
	pci_unregister_driver(&iegbe_driver);
}

module_exit(iegbe_exit_module);

/**
 * iegbe_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 **/

static inline void
iegbe_irq_disable(struct iegbe_adapter *adapter)
{

	atomic_inc(&adapter->irq_sem);
	E1000_WRITE_REG(&adapter->hw, IMC, ~0);
	E1000_WRITE_FLUSH(&adapter->hw);
	synchronize_irq(adapter->pdev->irq);
}

/**
 * iegbe_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 **/

static inline void
iegbe_irq_enable(struct iegbe_adapter *adapter)
{

	if(likely(atomic_dec_and_test(&adapter->irq_sem))) {
		E1000_WRITE_REG(&adapter->hw, IMS, IMS_ENABLE_MASK);
		E1000_WRITE_FLUSH(&adapter->hw);
	}
}
#ifdef NETIF_F_HW_VLAN_TX
void
iegbe_update_mng_vlan(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	uint16_t vid = adapter->hw.mng_cookie.vlan_id;
	uint16_t old_vid = adapter->mng_vlan_id;

	if (!iegbe_vlan_used(adapter))
		return;

	if (!test_bit(vid, adapter->active_vlans)) {
		if(adapter->hw.mng_cookie.status &
			E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) {
			iegbe_vlan_rx_add_vid(netdev, vid);
			adapter->mng_vlan_id = vid;
		} else {
			adapter->mng_vlan_id = E1000_MNG_VLAN_NONE;
		}

		if((old_vid != (uint16_t)E1000_MNG_VLAN_NONE) &&
		    (vid != old_vid) &&
		    !test_bit(old_vid, adapter->active_vlans)) {
			iegbe_vlan_rx_kill_vid(netdev, old_vid);
		}
	}
}
#endif

int
iegbe_up(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int i, err;
	uint16_t pci_cmd;

	/* hardware has been reset, we need to reload some things */

	/* Reset the PHY if it was previously powered down */
	if(adapter->hw.media_type == iegbe_media_type_copper
        || (adapter->hw.media_type == iegbe_media_type_oem
            && iegbe_oem_phy_is_copper(&adapter->hw))) {
		uint16_t mii_reg;
		iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &mii_reg);
        if(mii_reg & MII_CR_POWER_DOWN){
			iegbe_phy_reset(&adapter->hw);
	}
    }

	iegbe_set_multi(netdev);

#ifdef NETIF_F_HW_VLAN_TX
	iegbe_restore_vlan(adapter);
#endif

	iegbe_configure_tx(adapter);
	iegbe_setup_rctl(adapter);
	iegbe_configure_rx(adapter);

	for (i = 0; i < adapter->num_queues; i++)
		adapter->alloc_rx_buf(adapter, &adapter->rx_ring[i]);

#ifdef CONFIG_PCI_MSI
	if(adapter->hw.mac_type > iegbe_82547_rev_2
	   || adapter->hw.mac_type == iegbe_icp_xxxx) {
		adapter->have_msi = TRUE;
		if((err = pci_enable_msi(adapter->pdev))) {
			DPRINTK(PROBE, ERR,
			 "Unable to allocate MSI interrupt Error: %d\n", err);
			adapter->have_msi = FALSE;
		}
	}
        pci_read_config_word(adapter->pdev, PCI_COMMAND, &pci_cmd);
        pci_write_config_word(adapter->pdev, PCI_COMMAND, 
		                      pci_cmd | IEGBE_INTD_DISABLE);

#endif
	if((err = request_irq(adapter->pdev->irq, iegbe_intr,
		              SA_SHIRQ | SA_SAMPLE_RANDOM,
		              netdev->name, netdev))) {
		DPRINTK(PROBE, ERR,
		    "Unable to allocate interrupt Error: %d\n", err);
		return err;
	}

	mod_timer(&adapter->watchdog_timer, jiffies);

#ifdef CONFIG_E1000_NAPI
	netif_poll_enable(netdev);
#endif
	iegbe_irq_enable(adapter);

	return 0;
}

void
iegbe_down(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;

	iegbe_irq_disable(adapter);
#ifdef CONFIG_E1000_MQ
    while (atomic_read(&adapter->rx_sched_call_data.count) != 0) { };
#endif
	free_irq(adapter->pdev->irq, netdev);
#ifdef CONFIG_PCI_MSI
	if((adapter->hw.mac_type > iegbe_82547_rev_2
		|| adapter->hw.mac_type == iegbe_icp_xxxx)
       && adapter->have_msi == TRUE) {
		pci_disable_msi(adapter->pdev);
    }
#endif
	del_timer_sync(&adapter->tx_fifo_stall_timer);
	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

#ifdef CONFIG_E1000_NAPI
	netif_poll_disable(netdev);
#endif
	adapter->link_speed = 0;
	adapter->link_duplex = 0;
	netif_carrier_off(netdev);
	netif_stop_queue(netdev);

	iegbe_reset(adapter);
	iegbe_clean_all_tx_rings(adapter);
	iegbe_clean_all_rx_rings(adapter);

#if 0
	/* If WoL is not enabled and management mode is not IAMT
	 * or if WoL is not enabled and OEM PHY is copper based,
	 * power down the PHY so no link is implied when interface is down */
	if(!adapter->wol
	   && ((adapter->hw.mac_type >= iegbe_82540
	        && adapter->hw.media_type == iegbe_media_type_copper
	        && !iegbe_check_mng_mode(&adapter->hw)
	        && !(E1000_READ_REG(&adapter->hw, MANC) & E1000_MANC_SMBUS_EN))
	      || (adapter->hw.media_type == iegbe_media_type_oem
              && iegbe_oem_phy_is_copper(&adapter->hw)))){

		uint16_t mii_reg;
		iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &mii_reg);
		mii_reg |= MII_CR_POWER_DOWN;
		iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, mii_reg);
		mdelay(1);
	}
#endif
}

void
iegbe_reset(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	uint32_t pba, manc;
	uint16_t fc_high_water_mark = E1000_FC_HIGH_DIFF;
	uint16_t fc_low_water_mark = E1000_FC_LOW_DIFF;


	/* Repartition Pba for greater than 9k mtu
	 * To take effect CTRL.RST is required.
	 */

	switch (adapter->hw.mac_type) {
	case iegbe_82547:
	case iegbe_82547_rev_2:
		pba = E1000_PBA_30K;
		break;
	case iegbe_82571:
	case iegbe_82572:
		pba = E1000_PBA_38K;
		break;
	case iegbe_82573:
		pba = E1000_PBA_12K;
		break;
	default:
		pba = E1000_PBA_48K;
		break;
	}

	if((adapter->hw.mac_type != iegbe_82573) &&
	   (adapter->rx_buffer_len > E1000_RXBUFFER_8192)) {
        pba -= 0x8; /* allocate more FIFO for Tx */
		/* send an XOFF when there is enough space in the
		 * Rx FIFO to hold one extra full size Rx packet
		*/
		fc_high_water_mark = netdev->mtu + ENET_HEADER_SIZE +
                    ETHERNET_FCS_SIZE + 0x1;
        fc_low_water_mark = fc_high_water_mark + 0x8;
	}


	if(adapter->hw.mac_type == iegbe_82547) {
		adapter->tx_fifo_head = 0;
		adapter->tx_head_addr = pba << E1000_TX_HEAD_ADDR_SHIFT;
		adapter->tx_fifo_size =
			(E1000_PBA_40K - pba) << E1000_PBA_BYTES_SHIFT;
		atomic_set(&adapter->tx_fifo_stall, 0);
	}

	E1000_WRITE_REG(&adapter->hw, PBA, pba);

	/* flow control settings */
	adapter->hw.fc_high_water = (pba << E1000_PBA_BYTES_SHIFT) -
				    fc_high_water_mark;
	adapter->hw.fc_low_water = (pba << E1000_PBA_BYTES_SHIFT) -
				   fc_low_water_mark;
	adapter->hw.fc_pause_time = E1000_FC_PAUSE_TIME;
	adapter->hw.fc_send_xon = 1;
	adapter->hw.fc = adapter->hw.original_fc;

	/* Allow time for pending master requests to run */
	iegbe_reset_hw(&adapter->hw);
    if(adapter->hw.mac_type >= iegbe_82544){
		E1000_WRITE_REG(&adapter->hw, WUC, 0);
    }
    if(iegbe_init_hw(&adapter->hw)) {
		DPRINTK(PROBE, ERR, "Hardware Error\n");
    }
#ifdef NETIF_F_HW_VLAN_TX
	iegbe_update_mng_vlan(adapter);
#endif
	/* Enable h/w to recognize an 802.1Q VLAN Ethernet packet */
	E1000_WRITE_REG(&adapter->hw, VET, ETHERNET_IEEE_VLAN_TYPE);

	iegbe_reset_adaptive(&adapter->hw);
	iegbe_phy_get_info(&adapter->hw, &adapter->phy_info);
	if(adapter->en_mng_pt) {
		manc = E1000_READ_REG(&adapter->hw, MANC);
		manc |= (E1000_MANC_ARP_EN | E1000_MANC_EN_MNG2HOST);
		E1000_WRITE_REG(&adapter->hw, MANC, manc);
	}
}

#ifdef	USE_NETDEV_OPS
static const struct net_device_ops iegbe_netdev_ops = {
	.ndo_open		= iegbe_open,
	.ndo_stop		= iegbe_close,
	.ndo_start_xmit		= iegbe_xmit_frame,
	.ndo_tx_timeout		= iegbe_tx_timeout,
	.ndo_get_stats		= iegbe_get_stats,
	.ndo_set_rx_mode	= iegbe_set_multi,
	.ndo_set_mac_address	= iegbe_set_mac,
	.ndo_change_mtu		= iegbe_change_mtu,
	.ndo_do_ioctl		= iegbe_ioctl,
#ifdef NETIF_F_HW_VLAN_TX
	.ndo_vlan_rx_add_vid	= iegbe_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= iegbe_vlan_rx_kill_vid,
#endif
};
#endif

/**
 * iegbe_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in iegbe_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * iegbe_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/

static int
iegbe_probe(struct pci_dev *pdev,
            const struct pci_device_id *ent)
{
	struct net_device *netdev;
	struct iegbe_adapter *adapter;
	unsigned long mmio_start, mmio_len;
	uint32_t ctrl_ext;
	uint32_t swsm;

	static int cards_found = 0;

	int i, err, pci_using_dac;
	uint16_t eeprom_data = 0;
	uint16_t eeprom_apme_mask = E1000_EEPROM_APME;


    if((err = pci_enable_device(pdev))) {
		return err;
    }
	if(!(err = pci_set_dma_mask(pdev, PCI_DMA_64BIT))) {
		pci_using_dac = 1;
	} else {
		if((err = pci_set_dma_mask(pdev, PCI_DMA_32BIT))) {
			E1000_ERR("No usable DMA configuration, aborting\n");
			return err;
		}
		pci_using_dac = 0;
	}

    if((err = pci_request_regions(pdev, iegbe_driver_name))) {
		return err;
    }
	pci_set_master(pdev);

	netdev = alloc_etherdev(sizeof(struct iegbe_adapter));
	if(!netdev) {
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	SET_MODULE_OWNER(netdev);
	SET_NETDEV_DEV(netdev, &pdev->dev);

	pci_set_drvdata(pdev, netdev);
	adapter = netdev_priv(netdev);
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	adapter->hw.back = adapter;
    adapter->msg_enable = (0x1 << debug) - 0x1;

	mmio_start = pci_resource_start(pdev, BAR_0);
	mmio_len = pci_resource_len(pdev, BAR_0);

	adapter->hw.hw_addr = ioremap(mmio_start, mmio_len);
	if(!adapter->hw.hw_addr) {
		err = -EIO;
		goto err_ioremap;
	}

	for(i = BAR_1; i <= BAR_5; i++) {
        if(pci_resource_len(pdev, i) == 0) {
			continue;
        }
		if(pci_resource_flags(pdev, i) & IORESOURCE_IO) {
			adapter->hw.io_base = pci_resource_start(pdev, i);
			break;
		}
	}

#ifdef USE_NETDEV_OPS
	netdev->netdev_ops = &iegbe_netdev_ops;
#else
	netdev->open = &iegbe_open;
	netdev->stop = &iegbe_close;
	netdev->hard_start_xmit = &iegbe_xmit_frame;
	netdev->get_stats = &iegbe_get_stats;
	netdev->set_multicast_list = &iegbe_set_multi;
	netdev->set_mac_address = &iegbe_set_mac;
	netdev->change_mtu = &iegbe_change_mtu;
	netdev->do_ioctl = &iegbe_ioctl;
#endif
	set_ethtool_ops(netdev);
#ifdef HAVE_TX_TIMEOUT
#ifndef USE_NETDEV_OPS
	netdev->tx_timeout = &iegbe_tx_timeout;
#endif
	netdev->watchdog_timeo = 0x5 * HZ;
#endif
#ifdef CONFIG_E1000_NAPI
	netdev->poll = &iegbe_clean;
    netdev->weight = 0x40;
#endif
#if defined(NETIF_F_HW_VLAN_TX) && !defined(USE_NETDEV_OPS)
	netdev->vlan_rx_add_vid = iegbe_vlan_rx_add_vid;
	netdev->vlan_rx_kill_vid = iegbe_vlan_rx_kill_vid;
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
	netdev->poll_controller = iegbe_netpoll;
#endif
	strcpy(netdev->name, pci_name(pdev));

	netdev->mem_start = mmio_start;
	netdev->mem_end = mmio_start + mmio_len;
	netdev->base_addr = adapter->hw.io_base;

	adapter->bd_number = cards_found;

	/* setup the private structure */

    if((err = iegbe_sw_init(adapter))) {
		goto err_sw_init;
    }
    if((err = iegbe_check_phy_reset_block(&adapter->hw))) {
		DPRINTK(PROBE, INFO, "PHY reset is blocked due to SOL/IDER session.\n");
    }
#ifdef MAX_SKB_FRAGS
	if(adapter->hw.mac_type >= iegbe_82543) {
#ifdef NETIF_F_HW_VLAN_TX
		netdev->features = NETIF_F_SG |
				   NETIF_F_HW_CSUM |
				   NETIF_F_HW_VLAN_TX |
				   NETIF_F_HW_VLAN_RX |
				   NETIF_F_HW_VLAN_FILTER;
#else
		netdev->features = NETIF_F_SG | NETIF_F_HW_CSUM;
#endif
	}

#ifdef NETIF_F_TSO
	if((adapter->hw.mac_type >= iegbe_82544) &&
       (adapter->hw.mac_type != iegbe_82547)) {
		netdev->features |= NETIF_F_TSO;
    }
#ifdef NETIF_F_TSO_IPV6
    if(adapter->hw.mac_type > iegbe_82547_rev_2) {
		netdev->features |= NETIF_F_TSO_IPV6;
    }
#endif
#endif
    if(pci_using_dac) {
		netdev->features |= NETIF_F_HIGHDMA;
    }
#endif
#ifdef NETIF_F_LLTX
	netdev->features |= NETIF_F_LLTX;
#endif

	adapter->en_mng_pt = iegbe_enable_mng_pass_thru(&adapter->hw);

	/* before reading the EEPROM, reset the controller to
	 * put the device in a known good starting state */

	iegbe_reset_hw(&adapter->hw);

	/* make sure the EEPROM is good */
	if(iegbe_validate_eeprom_checksum(&adapter->hw) < 0) {
		DPRINTK(PROBE, ERR, "The EEPROM Checksum Is Not Valid\n");
		err = -EIO;
		goto err_eeprom;
	}

	/* copy the MAC address out of the EEPROM */

    if(iegbe_read_mac_addr(&adapter->hw)) {
		DPRINTK(PROBE, ERR, "EEPROM Read Error\n");
    }
	memcpy(netdev->dev_addr, adapter->hw.mac_addr, netdev->addr_len);

	if(!is_valid_ether_addr(netdev->dev_addr)) {
		DPRINTK(PROBE, ERR, "Invalid MAC Address\n");
		err = -EIO;
		goto err_eeprom;
	}

	iegbe_read_part_num(&adapter->hw, &(adapter->part_num));

	iegbe_get_bus_info(&adapter->hw);

	init_timer(&adapter->tx_fifo_stall_timer);
	adapter->tx_fifo_stall_timer.function = &iegbe_82547_tx_fifo_stall;
	adapter->tx_fifo_stall_timer.data = (unsigned long) adapter;

	init_timer(&adapter->watchdog_timer);
	adapter->watchdog_timer.function = &iegbe_watchdog;
	adapter->watchdog_timer.data = (unsigned long) adapter;

	init_timer(&adapter->phy_info_timer);
	adapter->phy_info_timer.function = &iegbe_update_phy_info;
	adapter->phy_info_timer.data = (unsigned long) adapter;

	_INIT_WORK(&adapter->tx_timeout_task, iegbe_tx_timeout_task, netdev);

	/* we're going to reset, so assume we have no link for now */

	netif_carrier_off(netdev);
	netif_stop_queue(netdev);

	iegbe_check_options(adapter);

	/* Initial Wake on LAN setting
	 * If APM wake is enabled in the EEPROM,
	 * enable the ACPI Magic Packet filter
	 */

	switch(adapter->hw.mac_type) {
	case iegbe_82542_rev2_0:
	case iegbe_82542_rev2_1:
	case iegbe_82543:
		break;
	case iegbe_82544:
		iegbe_read_eeprom(&adapter->hw,
			EEPROM_INIT_CONTROL2_REG, 1, &eeprom_data);
		eeprom_apme_mask = E1000_EEPROM_82544_APM;
		break;
    case iegbe_icp_xxxx:
		iegbe_read_eeprom(&adapter->hw,
			EEPROM_INIT_CONTROL3_ICP_xxxx(adapter->bd_number),
			1, &eeprom_data);
		eeprom_apme_mask = EEPROM_CTRL3_APME_ICP_xxxx;
		break;
	case iegbe_82546:
	case iegbe_82546_rev_3:
		if((E1000_READ_REG(&adapter->hw, STATUS) & E1000_STATUS_FUNC_1)
		   && (adapter->hw.media_type == iegbe_media_type_copper)) {
			iegbe_read_eeprom(&adapter->hw,
				EEPROM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);
			break;
		}
		/* Fall Through */
	default:
		iegbe_read_eeprom(&adapter->hw,
			EEPROM_INIT_CONTROL3_PORT_A, 1, &eeprom_data);
		break;
	}
    if(eeprom_data & eeprom_apme_mask) {
		adapter->wol |= E1000_WUFC_MAG;
    }
	/* reset the hardware with the new settings */
	iegbe_reset(adapter);

	/* Let firmware know the driver has taken over */
	switch(adapter->hw.mac_type) {
	case iegbe_82571:
	case iegbe_82572:
		ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
		E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
				ctrl_ext | E1000_CTRL_EXT_DRV_LOAD);
		break;
	case iegbe_82573:
		swsm = E1000_READ_REG(&adapter->hw, SWSM);
		E1000_WRITE_REG(&adapter->hw, SWSM,
				swsm | E1000_SWSM_DRV_LOAD);
		break;
	default:
		break;
	}

    /* The ICP_xxxx device has multiple, duplicate interrupt
     * registers, so disable all but the first one
     */
    if(adapter->hw.mac_type == iegbe_icp_xxxx) {
        int offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_ST)
                     + PCI_ST_SMIA_OFFSET;
        pci_write_config_dword(adapter->pdev, offset, 0x00000006);
        E1000_WRITE_REG(&adapter->hw, IMC1, ~0UL);
        E1000_WRITE_REG(&adapter->hw, IMC2, ~0UL);
    }

	strcpy(netdev->name, "eth%d");
    if((err = register_netdev(netdev))) {
		goto err_register;
    }
    DPRINTK(PROBE, INFO, "Intel(R) PRO/1000 Network Connection\n");

	cards_found++;
	return 0;

err_register:
err_sw_init:
err_eeprom:
	iounmap(adapter->hw.hw_addr);
err_ioremap:
	free_netdev(netdev);
err_alloc_etherdev:
	pci_release_regions(pdev);
	return err;
}

/**
 * iegbe_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * iegbe_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/

static void __devexit
iegbe_remove(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	uint32_t ctrl_ext;
	uint32_t manc, swsm;
#ifdef CONFIG_E1000_NAPI
	int i;
#endif

	if(adapter->hw.mac_type >= iegbe_82540
	   && adapter->hw.mac_type != iegbe_icp_xxxx
	   && adapter->hw.media_type == iegbe_media_type_copper) {
		manc = E1000_READ_REG(&adapter->hw, MANC);
		if(manc & E1000_MANC_SMBUS_EN) {
			manc |= E1000_MANC_ARP_EN;
			E1000_WRITE_REG(&adapter->hw, MANC, manc);
		}
	}

	switch(adapter->hw.mac_type) {
	case iegbe_82571:
	case iegbe_82572:
		ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
		E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
				ctrl_ext & ~E1000_CTRL_EXT_DRV_LOAD);
		break;
	case iegbe_82573:
		swsm = E1000_READ_REG(&adapter->hw, SWSM);
		E1000_WRITE_REG(&adapter->hw, SWSM,
				swsm & ~E1000_SWSM_DRV_LOAD);
		break;

	default:
		break;
	}

	unregister_netdev(netdev);
#ifdef CONFIG_E1000_NAPI
	for (i = 0; i < adapter->num_queues; i++)
		dev_put(&adapter->polling_netdev[i]);
#endif

    if(!iegbe_check_phy_reset_block(&adapter->hw)) {
		iegbe_phy_hw_reset(&adapter->hw);
    }
	kfree(adapter->tx_ring);
	kfree(adapter->rx_ring);
#ifdef CONFIG_E1000_NAPI
	kfree(adapter->polling_netdev);
#endif

	iounmap(adapter->hw.hw_addr);
	pci_release_regions(pdev);

#ifdef CONFIG_E1000_MQ
	free_percpu(adapter->cpu_netdev);
	free_percpu(adapter->cpu_tx_ring);
#endif
	free_netdev(netdev);
}

/**
 * iegbe_sw_init - Initialize general software structures (struct iegbe_adapter)
 * @adapter: board private structure to initialize
 *
 * iegbe_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/

static int
iegbe_sw_init(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
#ifdef CONFIG_E1000_NAPI
	int i;
#endif

	/* PCI config space info */

	hw->vendor_id = pdev->vendor;
	hw->device_id = pdev->device;
	hw->subsystem_vendor_id = pdev->subsystem_vendor;
	hw->subsystem_id = pdev->subsystem_device;

	pci_read_config_byte(pdev, PCI_REVISION_ID, &hw->revision_id);

	pci_read_config_word(pdev, PCI_COMMAND, &hw->pci_cmd_word);

	adapter->rx_buffer_len = E1000_RXBUFFER_2048;
	adapter->rx_ps_bsize0 = E1000_RXBUFFER_256;
	hw->max_frame_size = netdev->mtu +
			     ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;
	hw->min_frame_size = MINIMUM_ETHERNET_FRAME_SIZE;

	/* identify the MAC */

	if(iegbe_set_mac_type(hw)) {
		DPRINTK(PROBE, ERR, "Unknown MAC Type\n");
		return -EIO;
	}

	/* initialize eeprom parameters */

	if(iegbe_init_eeprom_params(hw)) {
		E1000_ERR("EEPROM initialization failed\n");
		return -EIO;
	}

	switch(hw->mac_type) {
	default:
		break;
	case iegbe_82541:
	case iegbe_82547:
	case iegbe_82541_rev_2:
	case iegbe_82547_rev_2:
        hw->phy_init_script = 0x1;
		break;
	}

	iegbe_set_media_type(hw);

  	hw->wait_autoneg_complete = FALSE;
	hw->tbi_compatibility_en = TRUE;
	hw->adaptive_ifs = TRUE;

	/* Copper options */

	if(hw->media_type == iegbe_media_type_copper
        || (hw->media_type == iegbe_media_type_oem
            && iegbe_oem_phy_is_copper(&adapter->hw))) {
		hw->mdix = AUTO_ALL_MODES;
		hw->disable_polarity_correction = FALSE;
		hw->master_slave = E1000_MASTER_SLAVE;
	}

#ifdef CONFIG_E1000_MQ
	/* Number of supported queues */
	switch (hw->mac_type) {
	case iegbe_82571:
	case iegbe_82572:
        adapter->num_queues = 0x2;
		break;
	default:
        adapter->num_queues = 0x1;
		break;
	}
	adapter->num_queues = min(adapter->num_queues, num_online_cpus());
#else
    adapter->num_queues = 0x1;
#endif

	if (iegbe_alloc_queues(adapter)) {
		DPRINTK(PROBE, ERR, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

#ifdef CONFIG_E1000_NAPI
	for (i = 0; i < adapter->num_queues; i++) {
		adapter->polling_netdev[i].priv = adapter;
		adapter->polling_netdev[i].poll = &iegbe_clean;
        adapter->polling_netdev[i].weight = 0x40;
		dev_hold(&adapter->polling_netdev[i]);
		set_bit(__LINK_STATE_START, &adapter->polling_netdev[i].state);
	}
#endif

#ifdef CONFIG_E1000_MQ
	iegbe_setup_queue_mapping(adapter);
#endif

        /*
	 * for ICP_XXXX style controllers, it is necessary to keep
	 * track of the last known state of the link to determine if
	 * the link experienced a change in state when iegbe_watchdog
	 * fires
	 */
	adapter->hw.icp_xxxx_is_link_up = FALSE;

	atomic_set(&adapter->irq_sem, 1);
	spin_lock_init(&adapter->stats_lock);

	return 0;
}

/**
 * iegbe_alloc_queues - Allocate memory for all rings
 * @adapter: board private structure to initialize
 *
 * We allocate one ring per queue at run-time since we don't know the
 * number of queues at compile-time.  The polling_netdev array is
 * intended for Multiqueue, but should work fine with a single queue.
 **/

static int
iegbe_alloc_queues(struct iegbe_adapter *adapter)
{
	int size;

	size = sizeof(struct iegbe_tx_ring) * adapter->num_queues;
	adapter->tx_ring = kmalloc(size, GFP_KERNEL);
    if (!adapter->tx_ring){
		return -ENOMEM;
    }
	memset(adapter->tx_ring, 0, size);

	size = sizeof(struct iegbe_rx_ring) * adapter->num_queues;
	adapter->rx_ring = kmalloc(size, GFP_KERNEL);
	if (!adapter->rx_ring) {
		kfree(adapter->tx_ring);
		return -ENOMEM;
	}
	memset(adapter->rx_ring, 0, size);

#ifdef CONFIG_E1000_NAPI
	size = sizeof(struct net_device) * adapter->num_queues;
	adapter->polling_netdev = kmalloc(size, GFP_KERNEL);
	if (!adapter->polling_netdev) {
		kfree(adapter->tx_ring);
		kfree(adapter->rx_ring);
		return -ENOMEM;
	}
	memset(adapter->polling_netdev, 0, size);
#endif

	return E1000_SUCCESS;
}

#ifdef CONFIG_E1000_MQ
static void
iegbe_setup_queue_mapping(struct iegbe_adapter *adapter)
{
	int i, cpu;

	adapter->rx_sched_call_data.func = iegbe_rx_schedule;
	adapter->rx_sched_call_data.info = adapter->netdev;
	cpus_clear(adapter->rx_sched_call_data.cpumask);

	adapter->cpu_netdev = alloc_percpu(struct net_device *);
	adapter->cpu_tx_ring = alloc_percpu(struct iegbe_tx_ring *);

	lock_cpu_hotplug();
	i = 0;
	for_each_online_cpu(cpu) {
        *per_cpu_ptr(adapter->cpu_tx_ring, cpu) =
                  &adapter->tx_ring[i % adapter->num_queues];
		/* This is incomplete because we'd like to assign separate
		 * physical cpus to these netdev polling structures and
		 * avoid saturating a subset of cpus.
		 */
		if (i < adapter->num_queues) {
            *per_cpu_ptr(adapter->cpu_netdev, cpu) =
                &adapter->polling_netdev[i];
			adapter->cpu_for_queue[i] = cpu;
        } else {
			*per_cpu_ptr(adapter->cpu_netdev, cpu) = NULL;
        }
		i++;
	}
	unlock_cpu_hotplug();
}
#endif

/**
 * iegbe_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 **/

static int
iegbe_open(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	int err;


	/* allocate receive descriptors */

    if ((err = iegbe_setup_all_rx_resources(adapter))) {
		goto err_setup_rx;
    }
	/* allocate transmit descriptors */
    if ((err = iegbe_setup_all_tx_resources(adapter))) {
		goto err_setup_tx;
    }
    if ((err = iegbe_up(adapter))) {
		goto err_up;
    }
#ifdef NETIF_F_HW_VLAN_TX
	adapter->mng_vlan_id = E1000_MNG_VLAN_NONE;
	if ((adapter->hw.mng_cookie.status &
			  E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT)) {
		iegbe_update_mng_vlan(adapter);
	}
#endif

	return E1000_SUCCESS;

err_up:
	iegbe_free_all_tx_resources(adapter);
err_setup_tx:
	iegbe_free_all_rx_resources(adapter);
err_setup_rx:
	iegbe_reset(adapter);

	return err;
}

/**
 * iegbe_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the drivers control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/

static int
iegbe_close(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);

	iegbe_down(adapter);

	iegbe_free_all_tx_resources(adapter);
	iegbe_free_all_rx_resources(adapter);

#ifdef NETIF_F_HW_VLAN_TX
	if((adapter->hw.mng_cookie.status &
			  E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) &&
	    !test_bit(adapter->mng_vlan_id, adapter->active_vlans)) {
		iegbe_vlan_rx_kill_vid(netdev, adapter->mng_vlan_id);
	}
#endif
	return 0;
}

/**
 * iegbe_check_64k_bound - check that memory doesn't cross 64kB boundary
 * @adapter: address of board private structure
 * @start: address of beginning of memory
 * @len: length of memory
 **/
static inline boolean_t
iegbe_check_64k_bound(struct iegbe_adapter *adapter,
		      void *start, unsigned long len)
{
	unsigned long begin = (unsigned long) start;
	unsigned long end = begin + len;

	/* First rev 82545 and 82546 need to not allow any memory
	 * write location to cross 64k boundary due to errata 23 */
	if(adapter->hw.mac_type == iegbe_82545 ||
	    adapter->hw.mac_type == iegbe_82546) {
        return ((begin ^ (end - 1)) >> 0x10) != 0 ? FALSE : TRUE;
	}

	return TRUE;
}

/**
 * iegbe_setup_tx_resources - allocate Tx resources (Descriptors)
 * @adapter: board private structure
 * @txdr:    tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 **/

int
iegbe_setup_tx_resources(struct iegbe_adapter *adapter,
                         struct iegbe_tx_ring *txdr)
{
	struct pci_dev *pdev = adapter->pdev;
	int size;

	size = sizeof(struct iegbe_buffer) * txdr->count;
	txdr->buffer_info = vmalloc(size);
	if (!txdr->buffer_info) {
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the transmit descriptor ring\n");
		return -ENOMEM;
	}
	memset(txdr->buffer_info, 0, size);
	memset(&txdr->previous_buffer_info, 0, sizeof(struct iegbe_buffer));

	/* round up to nearest 4K */

	txdr->size = txdr->count * sizeof(struct iegbe_tx_desc);
    E1000_ROUNDUP(txdr->size, 0x1000);

	txdr->desc = pci_alloc_consistent(pdev, txdr->size, &txdr->dma);
	if (!txdr->desc) {
setup_tx_desc_die:
		vfree(txdr->buffer_info);
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the transmit descriptor ring\n");
		return -ENOMEM;
	}

	/* Fix for errata 23, can't cross 64kB boundary */
	if (!iegbe_check_64k_bound(adapter, txdr->desc, txdr->size)) {
		void *olddesc = txdr->desc;
		dma_addr_t olddma = txdr->dma;
		DPRINTK(TX_ERR, ERR, "txdr align check failed: %u bytes "
				     "at %p\n", txdr->size, txdr->desc);
		/* Try again, without freeing the previous */
		txdr->desc = pci_alloc_consistent(pdev, txdr->size, &txdr->dma);
		/* Failed allocation, critical failure */
		if (!txdr->desc) {
			pci_free_consistent(pdev, txdr->size, olddesc, olddma);
			goto setup_tx_desc_die;
		}

		if (!iegbe_check_64k_bound(adapter, txdr->desc, txdr->size)) {
			/* give up */
			pci_free_consistent(pdev, txdr->size, txdr->desc,
					    txdr->dma);
			pci_free_consistent(pdev, txdr->size, olddesc, olddma);
			DPRINTK(PROBE, ERR,
				"Unable to allocate aligned memory "
				"for the transmit descriptor ring\n");
			vfree(txdr->buffer_info);
			return -ENOMEM;
		} else {
			/* Free old allocation, new allocation was successful */
			pci_free_consistent(pdev, txdr->size, olddesc, olddma);
		}
	}
	memset(txdr->desc, 0, txdr->size);

	txdr->next_to_use = 0;
	txdr->next_to_clean = 0;
	spin_lock_init(&txdr->tx_lock);

	return 0;
}

/**
 * iegbe_setup_all_tx_resources - wrapper to allocate Tx resources
 * 				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * If this function returns with an error, then it's possible one or
 * more of the rings is populated (while the rest are not).  It is the
 * callers duty to clean those orphaned rings.
 *
 * Return 0 on success, negative on failure
 **/

int
iegbe_setup_all_tx_resources(struct iegbe_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_queues; i++) {
		err = iegbe_setup_tx_resources(adapter, &adapter->tx_ring[i]);
		if (err) {
			DPRINTK(PROBE, ERR,
				"Allocation for Tx Queue %u failed\n", i);
			break;
		}
	}

	return err;
}

/**
 * iegbe_configure_tx - Configure 8254x Transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/

static void
iegbe_configure_tx(struct iegbe_adapter *adapter)
{
	uint64_t tdba;
	struct iegbe_hw *hw = &adapter->hw;
	uint32_t tdlen, tctl, tipg, tarc;

	/* Setup the HW Tx Head and Tail descriptor pointers */

	switch (adapter->num_queues) {
    case 0x2:
        tdba = adapter->tx_ring[0x1].dma;
        tdlen = adapter->tx_ring[0x1].count *
			sizeof(struct iegbe_tx_desc);
		E1000_WRITE_REG(hw, TDBAL1, (tdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, TDBAH1, (tdba >> 0x20));
		E1000_WRITE_REG(hw, TDLEN1, tdlen);
		E1000_WRITE_REG(hw, TDH1, 0);
		E1000_WRITE_REG(hw, TDT1, 0);
        adapter->tx_ring[0x1].tdh = E1000_TDH1;
        adapter->tx_ring[0x1].tdt = E1000_TDT1;
		/* Fall Through */
    case 0x1:
	default:
		tdba = adapter->tx_ring[0].dma;
		tdlen = adapter->tx_ring[0].count *
			sizeof(struct iegbe_tx_desc);
		E1000_WRITE_REG(hw, TDBAL, (tdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, TDBAH, (tdba >> 0x20));
		E1000_WRITE_REG(hw, TDLEN, tdlen);
		E1000_WRITE_REG(hw, TDH, 0);
		E1000_WRITE_REG(hw, TDT, 0);
		adapter->tx_ring[0].tdh = E1000_TDH;
		adapter->tx_ring[0].tdt = E1000_TDT;
		break;
	}

	/* Set the default values for the Tx Inter Packet Gap timer */

	switch (hw->mac_type) {
	case iegbe_82542_rev2_0:
	case iegbe_82542_rev2_1:
		tipg = DEFAULT_82542_TIPG_IPGT;
		tipg |= DEFAULT_82542_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
		tipg |= DEFAULT_82542_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
		break;
	default:
		switch(hw->media_type) {
		case iegbe_media_type_fiber:
		case iegbe_media_type_internal_serdes:
			tipg = DEFAULT_82543_TIPG_IPGT_FIBER;
			break;
		case iegbe_media_type_copper:
			tipg = DEFAULT_82543_TIPG_IPGT_COPPER;
			break;
		case iegbe_media_type_oem:
		default:
            tipg =  (0xFFFFFFFFUL >> (sizeof(tipg)*0x8 -
                E1000_TIPG_IPGR1_SHIFT))
				& iegbe_oem_get_tipg(&adapter->hw);
			break;
		}
		tipg |= DEFAULT_82543_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
		tipg |= DEFAULT_82543_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
	}
	E1000_WRITE_REG(hw, TIPG, tipg);

	/* Set the Tx Interrupt Delay register */

	E1000_WRITE_REG(hw, TIDV, adapter->tx_int_delay);
    if (hw->mac_type >= iegbe_82540) {
		E1000_WRITE_REG(hw, TADV, adapter->tx_abs_int_delay);
    }
	/* Program the Transmit Control Register */

	tctl = E1000_READ_REG(hw, TCTL);

	tctl &= ~E1000_TCTL_CT;
	tctl |= E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_RTLC |
		(E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);

	E1000_WRITE_REG(hw, TCTL, tctl);

	if (hw->mac_type == iegbe_82571 || hw->mac_type == iegbe_82572) {
		tarc = E1000_READ_REG(hw, TARC0);
        tarc |= ((0x1 << 0x19) | (0x1 << 0x15));
		E1000_WRITE_REG(hw, TARC0, tarc);
		tarc = E1000_READ_REG(hw, TARC1);
        tarc |= (0x1 << 0x19);
        if (tctl & E1000_TCTL_MULR) {
            tarc &= ~(0x1 << 0x1c);
        } else {
            tarc |= (0x1 << 0x1c);
        }
		E1000_WRITE_REG(hw, TARC1, tarc);
	}

	iegbe_config_collision_dist(hw);

	/* Setup Transmit Descriptor Settings for eop descriptor */
	adapter->txd_cmd = E1000_TXD_CMD_IDE | E1000_TXD_CMD_EOP |
		E1000_TXD_CMD_IFCS;

    if (hw->mac_type < iegbe_82543) {
		adapter->txd_cmd |= E1000_TXD_CMD_RPS;
    } else {
		adapter->txd_cmd |= E1000_TXD_CMD_RS;
    }
	/* Cache if we're 82544 running in PCI-X because we'll
	 * need this to apply a workaround later in the send path. */
	if (hw->mac_type == iegbe_82544 &&
        hw->bus_type == iegbe_bus_type_pcix) {
        adapter->pcix_82544 = 0x1;
     }
}

/**
 * iegbe_setup_rx_resources - allocate Rx resources (Descriptors)
 * @adapter: board private structure
 * @rxdr:    rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/

int
iegbe_setup_rx_resources(struct iegbe_adapter *adapter,
                         struct iegbe_rx_ring *rxdr)
{
	struct pci_dev *pdev = adapter->pdev;
	int size, desc_len;

	size = sizeof(struct iegbe_buffer) * rxdr->count;
	rxdr->buffer_info = vmalloc(size);
	if (!rxdr->buffer_info) {
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the receive descriptor ring\n");
		return -ENOMEM;
	}
	memset(rxdr->buffer_info, 0, size);

	size = sizeof(struct iegbe_ps_page) * rxdr->count;
	rxdr->ps_page = kmalloc(size, GFP_KERNEL);
	if (!rxdr->ps_page) {
		vfree(rxdr->buffer_info);
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the receive descriptor ring\n");
		return -ENOMEM;
	}
	memset(rxdr->ps_page, 0, size);

	size = sizeof(struct iegbe_ps_page_dma) * rxdr->count;
	rxdr->ps_page_dma = kmalloc(size, GFP_KERNEL);
	if (!rxdr->ps_page_dma) {
		vfree(rxdr->buffer_info);
		kfree(rxdr->ps_page);
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the receive descriptor ring\n");
		return -ENOMEM;
	}
	memset(rxdr->ps_page_dma, 0, size);

    if (adapter->hw.mac_type <= iegbe_82547_rev_2) {
		desc_len = sizeof(struct iegbe_rx_desc);
    } else {
		desc_len = sizeof(union iegbe_rx_desc_packet_split);
    }
	/* Round up to nearest 4K */

	rxdr->size = rxdr->count * desc_len;
    E1000_ROUNDUP(rxdr->size, 0x1000);

	rxdr->desc = pci_alloc_consistent(pdev, rxdr->size, &rxdr->dma);

	if (!rxdr->desc) {
		DPRINTK(PROBE, ERR,
		"Unable to allocate memory for the receive descriptor ring\n");
setup_rx_desc_die:
		vfree(rxdr->buffer_info);
		kfree(rxdr->ps_page);
		kfree(rxdr->ps_page_dma);
		return -ENOMEM;
	}

	/* Fix for errata 23, can't cross 64kB boundary */
	if (!iegbe_check_64k_bound(adapter, rxdr->desc, rxdr->size)) {
		void *olddesc = rxdr->desc;
		dma_addr_t olddma = rxdr->dma;
		DPRINTK(RX_ERR, ERR, "rxdr align check failed: %u bytes "
				     "at %p\n", rxdr->size, rxdr->desc);
		/* Try again, without freeing the previous */
		rxdr->desc = pci_alloc_consistent(pdev, rxdr->size, &rxdr->dma);
		/* Failed allocation, critical failure */
		if (!rxdr->desc) {
			pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
			DPRINTK(PROBE, ERR,
				"Unable to allocate memory "
				"for the receive descriptor ring\n");
			goto setup_rx_desc_die;
		}

		if (!iegbe_check_64k_bound(adapter, rxdr->desc, rxdr->size)) {
			/* give up */
			pci_free_consistent(pdev, rxdr->size, rxdr->desc,
					    rxdr->dma);
			pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
			DPRINTK(PROBE, ERR,
				"Unable to allocate aligned memory "
				"for the receive descriptor ring\n");
			goto setup_rx_desc_die;
		} else {
			/* Free old allocation, new allocation was successful */
			pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
		}
	}
	memset(rxdr->desc, 0, rxdr->size);

	rxdr->next_to_clean = 0;
	rxdr->next_to_use = 0;

	return 0;
}

/**
 * iegbe_setup_all_rx_resources - wrapper to allocate Rx resources
 * 				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * If this function returns with an error, then it's possible one or
 * more of the rings is populated (while the rest are not).  It is the
 * callers duty to clean those orphaned rings.
 *
 * Return 0 on success, negative on failure
 **/

int
iegbe_setup_all_rx_resources(struct iegbe_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_queues; i++) {
		err = iegbe_setup_rx_resources(adapter, &adapter->rx_ring[i]);
		if (err) {
			DPRINTK(PROBE, ERR,
				"Allocation for Rx Queue %u failed\n", i);
			break;
		}
	}

	return err;
}

/**
 * iegbe_setup_rctl - configure the receive control registers
 * @adapter: Board private structure
 **/
#define PAGE_USE_COUNT(S) (((S) >> PAGE_SHIFT) + \
			(((S) & (PAGE_SIZE - 1)) ? 1 : 0))
static void
iegbe_setup_rctl(struct iegbe_adapter *adapter)
{
	uint32_t rctl, rfctl;
	uint32_t psrctl = 0;
#ifdef CONFIG_E1000_PACKET_SPLIT
	uint32_t pages = 0;
#endif

	rctl = E1000_READ_REG(&adapter->hw, RCTL);

    rctl &= ~(0x3 << E1000_RCTL_MO_SHIFT);

	rctl |= E1000_RCTL_EN | E1000_RCTL_BAM |
		E1000_RCTL_LBM_NO | E1000_RCTL_RDMTS_HALF |
		(adapter->hw.mc_filter_type << E1000_RCTL_MO_SHIFT);

    if(adapter->hw.tbi_compatibility_on == 0x1) {
		rctl |= E1000_RCTL_SBP;
    } else {
		rctl &= ~E1000_RCTL_SBP;
    }
    if(adapter->netdev->mtu <= ETH_DATA_LEN) {
		rctl &= ~E1000_RCTL_LPE;
    } else {
		rctl |= E1000_RCTL_LPE;
    }
	/* Setup buffer sizes */
	if(adapter->hw.mac_type >= iegbe_82571) {
		/* We can now specify buffers in 1K increments.
		 * BSIZE and BSEX are ignored in this case. */
		rctl |= adapter->rx_buffer_len << 0x11;
	} else {
		rctl &= ~E1000_RCTL_SZ_4096;
		rctl |= E1000_RCTL_BSEX;
		switch (adapter->rx_buffer_len) {
		case E1000_RXBUFFER_2048:
		default:
			rctl |= E1000_RCTL_SZ_2048;
			rctl &= ~E1000_RCTL_BSEX;
			break;
		case E1000_RXBUFFER_4096:
			rctl |= E1000_RCTL_SZ_4096;
			break;
		case E1000_RXBUFFER_8192:
			rctl |= E1000_RCTL_SZ_8192;
			break;
		case E1000_RXBUFFER_16384:
			rctl |= E1000_RCTL_SZ_16384;
			break;
		}
	}

#ifdef CONFIG_E1000_PACKET_SPLIT
	/* 82571 and greater support packet-split where the protocol
	 * header is placed in skb->data and the packet data is
	 * placed in pages hanging off of skb_shinfo(skb)->nr_frags.
	 * In the case of a non-split, skb->data is linearly filled,
	 * followed by the page buffers.  Therefore, skb->data is
	 * sized to hold the largest protocol header.
	 */
	pages = PAGE_USE_COUNT(adapter->netdev->mtu);
    if ((adapter->hw.mac_type > iegbe_82547_rev_2) && (pages <= 0x3) &&
        PAGE_SIZE <= 0x4000) {
		adapter->rx_ps_pages = pages;
    } else {
		adapter->rx_ps_pages = 0;
    }
#endif
	if (adapter->rx_ps_pages) {
		/* Configure extra packet-split registers */
		rfctl = E1000_READ_REG(&adapter->hw, RFCTL);
		rfctl |= E1000_RFCTL_EXTEN;
		/* disable IPv6 packet split support */
		rfctl |= E1000_RFCTL_IPV6_DIS;
		E1000_WRITE_REG(&adapter->hw, RFCTL, rfctl);

		rctl |= E1000_RCTL_DTYP_PS | E1000_RCTL_SECRC;

		psrctl |= adapter->rx_ps_bsize0 >>
			E1000_PSRCTL_BSIZE0_SHIFT;

		switch (adapter->rx_ps_pages) {
        case 0x3:
			psrctl |= PAGE_SIZE <<
				E1000_PSRCTL_BSIZE3_SHIFT;
        case 0x2:
			psrctl |= PAGE_SIZE <<
				E1000_PSRCTL_BSIZE2_SHIFT;
        case 0x1:
			psrctl |= PAGE_SIZE >>
				E1000_PSRCTL_BSIZE1_SHIFT;
			break;
		}

		E1000_WRITE_REG(&adapter->hw, PSRCTL, psrctl);
	}

	E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
}

/**
 * iegbe_configure_rx - Configure 8254x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/

static void
iegbe_configure_rx(struct iegbe_adapter *adapter)
{
	uint64_t rdba;
	struct iegbe_hw *hw = &adapter->hw;
	uint32_t rdlen, rctl, rxcsum, ctrl_ext;
#ifdef CONFIG_E1000_MQ
	uint32_t reta, mrqc;
	int i;
#endif

	if (adapter->rx_ps_pages) {
		rdlen = adapter->rx_ring[0].count *
			sizeof(union iegbe_rx_desc_packet_split);
		adapter->clean_rx = iegbe_clean_rx_irq_ps;
		adapter->alloc_rx_buf = iegbe_alloc_rx_buffers_ps;
	} else {
		rdlen = adapter->rx_ring[0].count *
			sizeof(struct iegbe_rx_desc);
		adapter->clean_rx = iegbe_clean_rx_irq;
		adapter->alloc_rx_buf = iegbe_alloc_rx_buffers;
	}

	/* disable receives while setting up the descriptors */
	rctl = E1000_READ_REG(hw, RCTL);
	E1000_WRITE_REG(hw, RCTL, rctl & ~E1000_RCTL_EN);

	/* set the Receive Delay Timer Register */
	E1000_WRITE_REG(hw, RDTR, adapter->rx_int_delay);

	if (hw->mac_type >= iegbe_82540) {
		E1000_WRITE_REG(hw, RADV, adapter->rx_abs_int_delay);
        if(adapter->itr > 0x1) {
			E1000_WRITE_REG(hw, ITR,
                0x3b9aca00 / (adapter->itr * 0x100));
        }
	}

	if (hw->mac_type >= iegbe_82571) {
		/* Reset delay timers after every interrupt */
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_CANC;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
	}

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring */
	switch (adapter->num_queues) {
#ifdef CONFIG_E1000_MQ
    case 0x2:
        rdba = adapter->rx_ring[0x1].dma;
		E1000_WRITE_REG(hw, RDBAL1, (rdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, RDBAH1, (rdba >> 0x20));
		E1000_WRITE_REG(hw, RDLEN1, rdlen);
		E1000_WRITE_REG(hw, RDH1, 0);
		E1000_WRITE_REG(hw, RDT1, 0);
		adapter->rx_ring[1].rdh = E1000_RDH1;
		adapter->rx_ring[1].rdt = E1000_RDT1;
		/* Fall Through */
#endif
    case 0x1:
	default:
		rdba = adapter->rx_ring[0].dma;
		E1000_WRITE_REG(hw, RDBAL, (rdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, RDBAH, (rdba >> 0x20));
		E1000_WRITE_REG(hw, RDLEN, rdlen);
		E1000_WRITE_REG(hw, RDH, 0);
		E1000_WRITE_REG(hw, RDT, 0);
		adapter->rx_ring[0].rdh = E1000_RDH;
		adapter->rx_ring[0].rdt = E1000_RDT;
		break;
	}

#ifdef CONFIG_E1000_MQ
    if (adapter->num_queues > 0x1) {
        uint32_t random[0xa];

        get_random_bytes(&random[0], FORTY);

		if (hw->mac_type <= iegbe_82572) {
			E1000_WRITE_REG(hw, RSSIR, 0);
			E1000_WRITE_REG(hw, RSSIM, 0);
		}

		switch (adapter->num_queues) {
        case 0x2:
		default:
			reta = 0x00800080;
			mrqc = E1000_MRQC_ENABLE_RSS_2Q;
			break;
		}

		/* Fill out redirection table */
        for (i = 0; i < 0x20; i++)
			E1000_WRITE_REG_ARRAY(hw, RETA, i, reta);
		/* Fill out hash function seeds */
        for (i = 0; i < 0xa; i++)
			E1000_WRITE_REG_ARRAY(hw, RSSRK, i, random[i]);

		mrqc |= (E1000_MRQC_RSS_FIELD_IPV4 |
			 E1000_MRQC_RSS_FIELD_IPV4_TCP);
		E1000_WRITE_REG(hw, MRQC, mrqc);
	}

	/* Multiqueue and packet checksumming are mutually exclusive. */
	if (hw->mac_type >= iegbe_82571) {
		rxcsum = E1000_READ_REG(hw, RXCSUM);
		rxcsum |= E1000_RXCSUM_PCSD;
		E1000_WRITE_REG(hw, RXCSUM, rxcsum);
	}

#else

	/* Enable 82543 Receive Checksum Offload for TCP and UDP */
	if (hw->mac_type >= iegbe_82543) {
		rxcsum = E1000_READ_REG(hw, RXCSUM);
		if(adapter->rx_csum == TRUE) {
			rxcsum |= E1000_RXCSUM_TUOFL;

			/* Enable 82571 IPv4 payload checksum for UDP fragments
			 * Must be used in conjunction with packet-split. */
			if ((hw->mac_type >= iegbe_82571) &&
			   (adapter->rx_ps_pages)) {
				rxcsum |= E1000_RXCSUM_IPPCSE;
			}
		} else {
			rxcsum &= ~E1000_RXCSUM_TUOFL;
			/* don't need to clear IPPCSE as it defaults to 0 */
		}
		E1000_WRITE_REG(hw, RXCSUM, rxcsum);
	}
#endif /* CONFIG_E1000_MQ */

    if (hw->mac_type == iegbe_82573) {
		E1000_WRITE_REG(hw, ERT, 0x0100);
    }
	/* Enable Receives */
	E1000_WRITE_REG(hw, RCTL, rctl);
}

/**
 * iegbe_free_tx_resources - Free Tx Resources per Queue
 * @adapter: board private structure
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/

void
iegbe_free_tx_resources(struct iegbe_adapter *adapter,
                        struct iegbe_tx_ring *tx_ring)
{
	struct pci_dev *pdev = adapter->pdev;

	iegbe_clean_tx_ring(adapter, tx_ring);

	vfree(tx_ring->buffer_info);
	tx_ring->buffer_info = NULL;

	pci_free_consistent(pdev, tx_ring->size, tx_ring->desc, tx_ring->dma);

	tx_ring->desc = NULL;
}

/**
 * iegbe_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/

void
iegbe_free_all_tx_resources(struct iegbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_queues; i++)
		iegbe_free_tx_resources(adapter, &adapter->tx_ring[i]);
}

static inline void
iegbe_unmap_and_free_tx_resource(struct iegbe_adapter *adapter,
			struct iegbe_buffer *buffer_info)
{
	if(buffer_info->dma) {
		pci_unmap_page(adapter->pdev,
				buffer_info->dma,
				buffer_info->length,
				PCI_DMA_TODEVICE);
		buffer_info->dma = 0;
	}
	if(buffer_info->skb) {
		dev_kfree_skb_any(buffer_info->skb);
		buffer_info->skb = NULL;
	}
}

/**
 * iegbe_clean_tx_ring - Free Tx Buffers
 * @adapter: board private structure
 * @tx_ring: ring to be cleaned
 **/

static void
iegbe_clean_tx_ring(struct iegbe_adapter *adapter,
                    struct iegbe_tx_ring *tx_ring)
{
	struct iegbe_buffer *buffer_info;
	unsigned long size;
	unsigned int i;

	/* Free all the Tx ring sk_buffs */

	if (likely(tx_ring->previous_buffer_info.skb != NULL)) {
		iegbe_unmap_and_free_tx_resource(adapter,
				&tx_ring->previous_buffer_info);
	}

	for (i = 0; i < tx_ring->count; i++) {
		buffer_info = &tx_ring->buffer_info[i];
		iegbe_unmap_and_free_tx_resource(adapter, buffer_info);
	}

	size = sizeof(struct iegbe_buffer) * tx_ring->count;
	memset(tx_ring->buffer_info, 0, size);

	/* Zero out the descriptor ring */

	memset(tx_ring->desc, 0, tx_ring->size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;

	writel(0, adapter->hw.hw_addr + tx_ring->tdh);
	writel(0, adapter->hw.hw_addr + tx_ring->tdt);
}

/**
 * iegbe_clean_all_tx_rings - Free Tx Buffers for all queues
 * @adapter: board private structure
 **/

static void
iegbe_clean_all_tx_rings(struct iegbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_queues; i++)
		iegbe_clean_tx_ring(adapter, &adapter->tx_ring[i]);
}

/**
 * iegbe_free_rx_resources - Free Rx Resources
 * @adapter: board private structure
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/

void
iegbe_free_rx_resources(struct iegbe_adapter *adapter,
                        struct iegbe_rx_ring *rx_ring)
{
	struct pci_dev *pdev = adapter->pdev;

	iegbe_clean_rx_ring(adapter, rx_ring);

	vfree(rx_ring->buffer_info);
	rx_ring->buffer_info = NULL;
	kfree(rx_ring->ps_page);
	rx_ring->ps_page = NULL;
	kfree(rx_ring->ps_page_dma);
	rx_ring->ps_page_dma = NULL;

	pci_free_consistent(pdev, rx_ring->size, rx_ring->desc, rx_ring->dma);

	rx_ring->desc = NULL;
}

/**
 * iegbe_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 **/

void
iegbe_free_all_rx_resources(struct iegbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_queues; i++)
		iegbe_free_rx_resources(adapter, &adapter->rx_ring[i]);
}

/**
 * iegbe_clean_rx_ring - Free Rx Buffers per Queue
 * @adapter: board private structure
 * @rx_ring: ring to free buffers from
 **/

static void
iegbe_clean_rx_ring(struct iegbe_adapter *adapter,
                    struct iegbe_rx_ring *rx_ring)
{
	struct iegbe_buffer *buffer_info;
	struct iegbe_ps_page *ps_page;
	struct iegbe_ps_page_dma *ps_page_dma;
	struct pci_dev *pdev = adapter->pdev;
	unsigned long size;
	unsigned int i, j;

	/* Free all the Rx ring sk_buffs */

	for(i = 0; i < rx_ring->count; i++) {
		buffer_info = &rx_ring->buffer_info[i];
		if(buffer_info->skb) {
			ps_page = &rx_ring->ps_page[i];
			ps_page_dma = &rx_ring->ps_page_dma[i];
			pci_unmap_single(pdev,
					 buffer_info->dma,
					 buffer_info->length,
					 PCI_DMA_FROMDEVICE);

			dev_kfree_skb(buffer_info->skb);
			buffer_info->skb = NULL;

			for(j = 0; j < adapter->rx_ps_pages; j++) {
                if(!ps_page->ps_page[j]) { break; }
				pci_unmap_single(pdev,
						 ps_page_dma->ps_page_dma[j],
						 PAGE_SIZE, PCI_DMA_FROMDEVICE);
				ps_page_dma->ps_page_dma[j] = 0;
				put_page(ps_page->ps_page[j]);
				ps_page->ps_page[j] = NULL;
			}
		}
	}

	size = sizeof(struct iegbe_buffer) * rx_ring->count;
	memset(rx_ring->buffer_info, 0, size);
	size = sizeof(struct iegbe_ps_page) * rx_ring->count;
	memset(rx_ring->ps_page, 0, size);
	size = sizeof(struct iegbe_ps_page_dma) * rx_ring->count;
	memset(rx_ring->ps_page_dma, 0, size);

	/* Zero out the descriptor ring */

	memset(rx_ring->desc, 0, rx_ring->size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	writel(0, adapter->hw.hw_addr + rx_ring->rdh);
	writel(0, adapter->hw.hw_addr + rx_ring->rdt);
}

/**
 * iegbe_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 **/

static void
iegbe_clean_all_rx_rings(struct iegbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_queues; i++)
		iegbe_clean_rx_ring(adapter, &adapter->rx_ring[i]);
}

/* The 82542 2.0 (revision 2) needs to have the receive unit in reset
 * and memory write and invalidate disabled for certain operations
 */
static void
iegbe_enter_82542_rst(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	uint32_t rctl;

	iegbe_pci_clear_mwi(&adapter->hw);

	rctl = E1000_READ_REG(&adapter->hw, RCTL);
	rctl |= E1000_RCTL_RST;
	E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
	E1000_WRITE_FLUSH(&adapter->hw);
    mdelay(0x5);

    if(netif_running(netdev)) {
		iegbe_clean_all_rx_rings(adapter);
}
}

static void
iegbe_leave_82542_rst(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	uint32_t rctl;

	rctl = E1000_READ_REG(&adapter->hw, RCTL);
	rctl &= ~E1000_RCTL_RST;
	E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
	E1000_WRITE_FLUSH(&adapter->hw);
    mdelay(0x5);

    if(adapter->hw.pci_cmd_word & PCI_COMMAND_INVALIDATE) {
		iegbe_pci_set_mwi(&adapter->hw);
    }
	if(netif_running(netdev)) {
		iegbe_configure_rx(adapter);
	   iegbe_alloc_rx_buffers(adapter, &adapter->rx_ring[0]);	
	}
}

/**
 * iegbe_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/

static int
iegbe_set_mac(struct net_device *netdev, void *p)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct sockaddr *addr = p;

    if(!is_valid_ether_addr(addr->sa_data)) {
		return -EADDRNOTAVAIL;
    }
	/* 82542 2.0 needs to be in reset to write receive address registers */

    if(adapter->hw.mac_type == iegbe_82542_rev2_0) {
		iegbe_enter_82542_rst(adapter);
    }
	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	memcpy(adapter->hw.mac_addr, addr->sa_data, netdev->addr_len);

	iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr, 0);

	/* With 82571 controllers, LAA may be overwritten (with the default)
	 * due to controller reset from the other port. */
	if (adapter->hw.mac_type == iegbe_82571) {
		/* activate the work around */
        adapter->hw.laa_is_present = 0x1;

		/* Hold a copy of the LAA in RAR[14] This is done so that
		 * between the time RAR[0] gets clobbered  and the time it
		 * gets fixed (in iegbe_watchdog), the actual LAA is in one
		 * of the RARs and no incoming packets directed to this port
		 * are dropped. Eventaully the LAA will be in RAR[0] and
		 * RAR[14] */
		iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr,
                    E1000_RAR_ENTRIES - 0x1);
	}

    if(adapter->hw.mac_type == iegbe_82542_rev2_0) {
		iegbe_leave_82542_rst(adapter);
    }
	return 0;
}

/**
 * iegbe_set_multi - Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_multi entry point is called whenever the multicast address
 * list or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper multicast,
 * promiscuous mode, and all-multi behavior.
 **/

static void
iegbe_set_multi(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
	struct netdev_hw_addr *mc_ptr;
	uint32_t rctl;
	uint32_t hash_value;
	int i, rar_entries = E1000_RAR_ENTRIES;

	/* reserve RAR[14] for LAA over-write work-around */
    if (adapter->hw.mac_type == iegbe_82571) {
		rar_entries--;
    }
	/* Check for Promiscuous and All Multicast modes */

	rctl = E1000_READ_REG(hw, RCTL);

	if (netdev->flags & IFF_PROMISC) {
		rctl |= (E1000_RCTL_UPE | E1000_RCTL_MPE);
	} else if (netdev->flags & IFF_ALLMULTI) {
		rctl |= E1000_RCTL_MPE;
		rctl &= ~E1000_RCTL_UPE;
	} else {
		rctl &= ~(E1000_RCTL_UPE | E1000_RCTL_MPE);
	}

	E1000_WRITE_REG(hw, RCTL, rctl);

	/* 82542 2.0 needs to be in reset to write receive address registers */

    if (hw->mac_type == iegbe_82542_rev2_0) {
		iegbe_enter_82542_rst(adapter);
    }

	/* clear the old settings from the multicast hash table */
	for (i = 0; i < E1000_NUM_MTA_REGISTERS; i++)
		E1000_WRITE_REG_ARRAY(hw, MTA, i, 0);

	/* load the first 14 multicast address into the exact filters 1-14
	 * RAR 0 is used for the station MAC adddress
	 * if there are not 14 addresses, go ahead and clear the filters
	 * -- with 82571 controllers only 0-13 entries are filled here
	 */
	i = 1;
	netdev_for_each_mc_addr(mc_ptr, netdev) {
		if (i < rar_entries) {
			iegbe_rar_set(hw, mc_ptr->addr, i);
		} else {
			/* load any remaining addresses into the hash table */
			hash_value = iegbe_hash_mc_addr(hw, mc_ptr->addr);
			iegbe_mta_set(hw, hash_value);
		}
		i++;
	}

	for (; i < rar_entries; i++) {
            E1000_WRITE_REG_ARRAY(hw, RA, i << 0x1, 0);
            E1000_WRITE_REG_ARRAY(hw, RA, (i << 0x1) + 0x1, 0);
	}

    if (hw->mac_type == iegbe_82542_rev2_0) {
		iegbe_leave_82542_rst(adapter);
}
}

/* Need to wait a few seconds after link up to get diagnostic information from
 * the phy */

static void
iegbe_update_phy_info(unsigned long data)
{
	struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
	iegbe_phy_get_info(&adapter->hw, &adapter->phy_info);
}

/**
 * iegbe_82547_tx_fifo_stall - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/

static void
iegbe_82547_tx_fifo_stall(unsigned long data)
{
	struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
	struct net_device *netdev = adapter->netdev;
	uint32_t tctl;

	if(atomic_read(&adapter->tx_fifo_stall)) {
		if((E1000_READ_REG(&adapter->hw, TDT) ==
		    E1000_READ_REG(&adapter->hw, TDH)) &&
		   (E1000_READ_REG(&adapter->hw, TDFT) ==
		    E1000_READ_REG(&adapter->hw, TDFH)) &&
		   (E1000_READ_REG(&adapter->hw, TDFTS) ==
		    E1000_READ_REG(&adapter->hw, TDFHS))) {
			tctl = E1000_READ_REG(&adapter->hw, TCTL);
			E1000_WRITE_REG(&adapter->hw, TCTL,
					tctl & ~E1000_TCTL_EN);
			E1000_WRITE_REG(&adapter->hw, TDFT,
					adapter->tx_head_addr);
			E1000_WRITE_REG(&adapter->hw, TDFH,
					adapter->tx_head_addr);
			E1000_WRITE_REG(&adapter->hw, TDFTS,
					adapter->tx_head_addr);
			E1000_WRITE_REG(&adapter->hw, TDFHS,
					adapter->tx_head_addr);
			E1000_WRITE_REG(&adapter->hw, TCTL, tctl);
			E1000_WRITE_FLUSH(&adapter->hw);

			adapter->tx_fifo_head = 0;
			atomic_set(&adapter->tx_fifo_stall, 0);
			netif_wake_queue(netdev);
		} else {
            mod_timer(&adapter->tx_fifo_stall_timer, jiffies + 0x1);
		}
	}
}

/**
 * iegbe_watchdog - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/
static void
iegbe_watchdog(unsigned long data)
{
	struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
	struct net_device *netdev = adapter->netdev;
	struct iegbe_tx_ring *txdr = &adapter->tx_ring[0];
	uint32_t link;

   /*
    * Test the PHY for link status on icp_xxxx MACs.
    * If the link status is different than the last link status stored
    * in the adapter->hw structure, then set hw->get_link_status = 1
    */
    if(adapter->hw.mac_type == iegbe_icp_xxxx) {
        int isUp = 0;
        int32_t ret_val;

        ret_val = iegbe_oem_phy_is_link_up(&adapter->hw, &isUp);
        if(ret_val != E1000_SUCCESS) {
            isUp = 0;
    }
        if(isUp != adapter->hw.icp_xxxx_is_link_up) {
            adapter->hw.get_link_status = 0x1;
        }
    }

	iegbe_check_for_link(&adapter->hw);
	if (adapter->hw.mac_type == iegbe_82573) {
		iegbe_enable_tx_pkt_filtering(&adapter->hw);
#ifdef NETIF_F_HW_VLAN_TX
        if (adapter->mng_vlan_id != adapter->hw.mng_cookie.vlan_id) {
			iegbe_update_mng_vlan(adapter);
        }
#endif
	}

	if ((adapter->hw.media_type == iegbe_media_type_internal_serdes) &&
	   !(E1000_READ_REG(&adapter->hw, TXCW) & E1000_TXCW_ANE)) {
		link = !adapter->hw.serdes_link_down;
	} else {

		if(adapter->hw.mac_type != iegbe_icp_xxxx) {
			link = E1000_READ_REG(&adapter->hw, STATUS) & E1000_STATUS_LU;
		} else {
			int isUp = 0;
            if(iegbe_oem_phy_is_link_up(&adapter->hw, &isUp) != E1000_SUCCESS) {
				isUp = 0;
                }
			link = isUp;
		}
	}

	if (link) {
		if (!netif_carrier_ok(netdev)) {
			iegbe_get_speed_and_duplex(&adapter->hw,
			                           &adapter->link_speed,
			                           &adapter->link_duplex);

			DPRINTK(LINK, INFO, "NIC Link is Up %d Mbps %s\n",
			       adapter->link_speed,
			       adapter->link_duplex == FULL_DUPLEX ?
			       "Full Duplex" : "Half Duplex");

			netif_carrier_on(netdev);
			netif_wake_queue(netdev);
            mod_timer(&adapter->phy_info_timer, jiffies + 0x2 * HZ);
			adapter->smartspeed = 0;
		}
	} else {
		if (netif_carrier_ok(netdev)) {
			adapter->link_speed = 0;
			adapter->link_duplex = 0;
			DPRINTK(LINK, INFO, "NIC Link is Down\n");
			netif_carrier_off(netdev);
			netif_stop_queue(netdev);
            mod_timer(&adapter->phy_info_timer, jiffies + 0x2 * HZ);
		}

		iegbe_smartspeed(adapter);
	}

	iegbe_update_stats(adapter);

	adapter->hw.tx_packet_delta = adapter->stats.tpt - adapter->tpt_old;
	adapter->tpt_old = adapter->stats.tpt;
	adapter->hw.collision_delta = adapter->stats.colc - adapter->colc_old;
	adapter->colc_old = adapter->stats.colc;

	adapter->gorcl = adapter->stats.gorcl - adapter->gorcl_old;
	adapter->gorcl_old = adapter->stats.gorcl;
	adapter->gotcl = adapter->stats.gotcl - adapter->gotcl_old;
	adapter->gotcl_old = adapter->stats.gotcl;

	iegbe_update_adaptive(&adapter->hw);

    if (adapter->num_queues == 0x1 && !netif_carrier_ok(netdev)) {
        if (E1000_DESC_UNUSED(txdr) + 0x1 < txdr->count) {
			/* We've lost link, so the controller stops DMA,
			 * but we've got queued Tx work that's never going
			 * to get done, so reset controller to flush Tx.
			 * (Do the reset outside of interrupt context). */
			schedule_work(&adapter->tx_timeout_task);
		}
	}

	/* Dynamic mode for Interrupt Throttle Rate (ITR) */
    if (adapter->hw.mac_type >= iegbe_82540 && adapter->itr == 0x1) {
		/* Symmetric Tx/Rx gets a reduced ITR=2000; Total
		 * asymmetrical Tx or Rx gets ITR=8000; everyone
		 * else is between 2000-8000. */
        uint32_t goc = (adapter->gotcl + adapter->gorcl) / 0x2710;
		uint32_t dif = (adapter->gotcl > adapter->gorcl ?
			adapter->gotcl - adapter->gorcl :
            adapter->gorcl - adapter->gotcl) / 0x2710;
        uint32_t itr = goc > 0 ? (dif * 0x1770 / goc + 0x7d0) : 0x1f40;
        E1000_WRITE_REG(&adapter->hw, ITR, 0x3b9aca00 / (itr * 0x100));
	}

	/* Cause software interrupt to ensure rx ring is cleaned */
	E1000_WRITE_REG(&adapter->hw, ICS, E1000_ICS_RXDMT0);

	/* Force detection of hung controller every watchdog period */
	adapter->detect_tx_hung = TRUE;

	/* With 82571 controllers, LAA may be overwritten due to controller
	 * reset from the other port. Set the appropriate LAA in RAR[0] */
    if (adapter->hw.mac_type == iegbe_82571 && adapter->hw.laa_is_present) {
		iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr, 0);
    }
	/* Reset the timer */
      mod_timer(&adapter->watchdog_timer, jiffies + 0x2 * HZ);
}

#define E1000_TX_FLAGS_CSUM		0x00000001
#define E1000_TX_FLAGS_VLAN		0x00000002
#define E1000_TX_FLAGS_TSO		0x00000004
#define E1000_TX_FLAGS_IPV4		0x00000008
#define E1000_TX_FLAGS_VLAN_MASK	0xffff0000
#define E1000_TX_FLAGS_VLAN_SHIFT	16

static inline int
iegbe_tso(struct iegbe_adapter *adapter, struct iegbe_tx_ring *tx_ring,
          struct sk_buff *skb)
{
#ifdef NETIF_F_TSO
	struct iegbe_context_desc *context_desc;
	struct iphdr *iph = ip_hdr(skb);
	unsigned int i;
	uint32_t cmd_length = 0;
	uint16_t ipcse = 0, tucse, mss;
	uint8_t ipcss, ipcso, tucss, tucso, hdr_len;
	int err;

	if (skb_is_gso(skb)) {
		if (skb_header_cloned(skb)) {
			err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
            if (err) {
				return err;
		}
        }

        hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
		mss = skb_shinfo(skb)->gso_size;
		if (skb->protocol == htons(ETH_P_IP)) {
			iph->tot_len = 0;
			iph->check = 0;
			tcp_hdr(skb)->check =
				~csum_tcpudp_magic(iph->saddr,
						   iph->daddr,
						   0,
						   IPPROTO_TCP,
						   0);
			cmd_length = E1000_TXD_CMD_IP;
            ipcse = skb_transport_offset(skb) - 1;
#ifdef NETIF_F_TSO_IPV6
		} else if (skb->protocol == ntohs(ETH_P_IPV6)) {
			struct ipv6hdr *ipv6h = ipv6_hdr(skb);
			ipv6h->payload_len = 0;
			tcp_hdr(skb)->check =
				~csum_ipv6_magic(&ipv6h->saddr,
						 &ipv6h->daddr,
						 0,
						 IPPROTO_TCP,
						 0);
			ipcse = 0;
#endif
		}
		ipcss = skb_network_offset(skb);
		ipcso = (void *)&(iph->check) - (void *)skb->data;
		tucss = skb_transport_offset(skb);
		tucso = (void *)&(tcp_hdr(skb)->check) - (void *)skb->data;
		tucse = 0;

		cmd_length |= (E1000_TXD_CMD_DEXT | E1000_TXD_CMD_TSE |
			       E1000_TXD_CMD_TCP | (skb->len - (hdr_len)));

		i = tx_ring->next_to_use;
		context_desc = E1000_CONTEXT_DESC(*tx_ring, i);

		context_desc->lower_setup.ip_fields.ipcss  = ipcss;
		context_desc->lower_setup.ip_fields.ipcso  = ipcso;
		context_desc->lower_setup.ip_fields.ipcse  = cpu_to_le16(ipcse);
		context_desc->upper_setup.tcp_fields.tucss = tucss;
		context_desc->upper_setup.tcp_fields.tucso = tucso;
		context_desc->upper_setup.tcp_fields.tucse = cpu_to_le16(tucse);
		context_desc->tcp_seg_setup.fields.mss     = cpu_to_le16(mss);
		context_desc->tcp_seg_setup.fields.hdr_len = hdr_len;
		context_desc->cmd_and_length = cpu_to_le32(cmd_length);

        if (++i == tx_ring->count) { i = 0; }
		tx_ring->next_to_use = i;

		return TRUE;
	}
#endif

	return FALSE;
}

static inline boolean_t
iegbe_tx_csum(struct iegbe_adapter *adapter, struct iegbe_tx_ring *tx_ring,
              struct sk_buff *skb)
{
	struct iegbe_context_desc *context_desc;
	unsigned int i;
	uint8_t css;

	if (likely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		css = skb_transport_offset(skb);

		i = tx_ring->next_to_use;
		context_desc = E1000_CONTEXT_DESC(*tx_ring, i);

		context_desc->upper_setup.tcp_fields.tucss = css;
		context_desc->upper_setup.tcp_fields.tucso = css + skb->csum;
		context_desc->upper_setup.tcp_fields.tucse = 0;
		context_desc->tcp_seg_setup.data = 0;
		context_desc->cmd_and_length = cpu_to_le32(E1000_TXD_CMD_DEXT);

        if (unlikely(++i == tx_ring->count)) { i = 0; }
		tx_ring->next_to_use = i;

		return TRUE;
	}

	return FALSE;
}

#define E1000_MAX_TXD_PWR	12
#define E1000_MAX_DATA_PER_TXD	(1<<E1000_MAX_TXD_PWR)

static inline int
iegbe_tx_map(struct iegbe_adapter *adapter, struct iegbe_tx_ring *tx_ring,
             struct sk_buff *skb, unsigned int first, unsigned int max_per_txd,
             unsigned int nr_frags, unsigned int mss)
{
	struct iegbe_buffer *buffer_info;
	unsigned int len = skb->len;
	unsigned int offset = 0, size, count = 0, i;
#ifdef MAX_SKB_FRAGS
	unsigned int f;
	len -= skb->data_len;
#endif

	i = tx_ring->next_to_use;

	while(len) {
		buffer_info = &tx_ring->buffer_info[i];
		size = min(len, max_per_txd);
#ifdef NETIF_F_TSO
		/* Workaround for premature desc write-backs
		 * in TSO mode.  Append 4-byte sentinel desc */
        if(unlikely(mss && !nr_frags && size == len && size > 0x8)) {
            size -= 0x4;
        }
#endif
		/* work-around for errata 10 and it applies
		 * to all controllers in PCI-X mode
		 * The fix is to make sure that the first descriptor of a
		 * packet is smaller than 2048 - 16 - 16 (or 2016) bytes
		 */
		if(unlikely((adapter->hw.bus_type == iegbe_bus_type_pcix) &&
                        (size > 0x7df) && count == 0)) {
                size = 0x7df;
        }
		/* Workaround for potential 82544 hang in PCI-X.  Avoid
		 * terminating buffers within evenly-aligned dwords. */
		if(unlikely(adapter->pcix_82544 &&
           !((unsigned long)(skb->data + offset + size - 0x8) & 0x4) &&
           size > 0x4)) {
            size -= 0x4;
        }
		buffer_info->length = size;
		buffer_info->dma =
			pci_map_single(adapter->pdev,
				skb->data + offset,
				size,
				PCI_DMA_TODEVICE);
		buffer_info->time_stamp = jiffies;

		len -= size;
		offset += size;
		count++;
        if(unlikely(++i == tx_ring->count)) { i = 0; }
	}

#ifdef MAX_SKB_FRAGS
	for(f = 0; f < nr_frags; f++) {
		struct skb_frag_struct *frag;

		frag = &skb_shinfo(skb)->frags[f];
		len = frag->size;
		offset = frag->page_offset;

		while(len) {
			buffer_info = &tx_ring->buffer_info[i];
			size = min(len, max_per_txd);
#ifdef NETIF_F_TSO
			/* Workaround for premature desc write-backs
			 * in TSO mode.  Append 4-byte sentinel desc */
            if(unlikely(mss && f == (nr_frags-0x1) &&
                    size == len && size > 0x8)) {
                size -= 0x4;
            }
#endif
			/* Workaround for potential 82544 hang in PCI-X.
			 * Avoid terminating buffers within evenly-aligned
			 * dwords. */
			if(unlikely(adapter->pcix_82544 &&
               !((unsigned long)(frag->page.p+offset+size-0x1) & 0x4) &&
               size > 0x4)) {
                size -= 0x4;
            }
			buffer_info->length = size;
			buffer_info->dma =
				pci_map_page(adapter->pdev,
					frag->page.p,
					offset,
					size,
					PCI_DMA_TODEVICE);
			buffer_info->time_stamp = jiffies;

			len -= size;
			offset += size;
			count++;
            if(unlikely(++i == tx_ring->count))  { i = 0; }
		}
	}
#endif

    i = (i == 0) ? tx_ring->count - 0x1 : i - 0x1;
	tx_ring->buffer_info[i].skb = skb;
	tx_ring->buffer_info[first].next_to_watch = i;

	return count;
}

static inline void
iegbe_tx_queue(struct iegbe_adapter *adapter, struct iegbe_tx_ring *tx_ring,
               int tx_flags, int count)
{
	struct iegbe_tx_desc *tx_desc = NULL;
	struct iegbe_buffer *buffer_info;
	uint32_t txd_upper = 0, txd_lower = E1000_TXD_CMD_IFCS;
	unsigned int i;

	if(likely(tx_flags & E1000_TX_FLAGS_TSO)) {
		txd_lower |= E1000_TXD_CMD_DEXT | E1000_TXD_DTYP_D |
		             E1000_TXD_CMD_TSE;
        txd_upper |= E1000_TXD_POPTS_TXSM << 0x8;

        if(likely(tx_flags & E1000_TX_FLAGS_IPV4)) {
            txd_upper |= E1000_TXD_POPTS_IXSM << 0x8;
        }
	}

	if(likely(tx_flags & E1000_TX_FLAGS_CSUM)) {
		txd_lower |= E1000_TXD_CMD_DEXT | E1000_TXD_DTYP_D;
        txd_upper |= E1000_TXD_POPTS_TXSM << 0x8;
	}

	if(unlikely(tx_flags & E1000_TX_FLAGS_VLAN)) {
		txd_lower |= E1000_TXD_CMD_VLE;
		txd_upper |= (tx_flags & E1000_TX_FLAGS_VLAN_MASK);
	}

	i = tx_ring->next_to_use;

	while(count--) {
		buffer_info = &tx_ring->buffer_info[i];
		tx_desc = E1000_TX_DESC(*tx_ring, i);
		tx_desc->buffer_addr = cpu_to_le64(buffer_info->dma);
		tx_desc->lower.data =
			cpu_to_le32(txd_lower | buffer_info->length);
		tx_desc->upper.data = cpu_to_le32(txd_upper);
        if(unlikely(++i == tx_ring->count)) { i = 0; }
	}
    if(tx_desc != NULL) {
		tx_desc->lower.data |= cpu_to_le32(adapter->txd_cmd);
    }
	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64). */
	wmb();

	tx_ring->next_to_use = i;
	writel(i, adapter->hw.hw_addr + tx_ring->tdt);
}

static inline int
iegbe_transfer_dhcp_info(struct iegbe_adapter *adapter, struct sk_buff *skb)
{
	struct iegbe_hw *hw =  &adapter->hw;
	uint16_t length, offset;
#ifdef NETIF_F_HW_VLAN_TX
	if(vlan_tx_tag_present(skb)) {
		if(!((vlan_tx_tag_get(skb) == adapter->hw.mng_cookie.vlan_id) &&
			( adapter->hw.mng_cookie.status &
              E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT)) ) {
			return 0;
	}
    }
#endif
	if(htons(ETH_P_IP) == skb->protocol) {
		const struct iphdr *ip = ip_hdr(skb);
		if(IPPROTO_UDP == ip->protocol) {
			struct udphdr *udp = udp_hdr(skb);
            if(ntohs(udp->dest) == 0x43) {      /* 0x43 = 67 */
                offset = (uint8_t *)udp + 0x8 - skb->data;
				length = skb->len - offset;

				return iegbe_mng_write_dhcp_info(hw,
                        (uint8_t *)udp + 0x8, length);
			}
		}
	} else if((skb->len > MINIMUM_DHCP_PACKET_SIZE) && (!skb->protocol)) {
		struct ethhdr *eth = (struct ethhdr *) skb->data;
		if((htons(ETH_P_IP) == eth->h_proto)) {
			const struct iphdr *ip =
                (struct iphdr *)((uint8_t *)skb->data+0xe);
			if(IPPROTO_UDP == ip->protocol) {
				struct udphdr *udp =
					(struct udphdr *)((uint8_t *)ip +
                        (ip->ihl << 0x2));
                if(ntohs(udp->dest) == 0x43) {
                    offset = (uint8_t *)udp + 0x8 - skb->data;
					length = skb->len - offset;

					return iegbe_mng_write_dhcp_info(hw,
                            (uint8_t *)udp + 0x8,
							length);
				}
			}
		}
	}
	return 0;
}

static int
iegbe_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_tx_ring *tx_ring;
	unsigned int first, max_per_txd = E1000_MAX_DATA_PER_TXD;
	unsigned int max_txd_pwr = E1000_MAX_TXD_PWR;
	unsigned int tx_flags = 0;
	unsigned int len = skb->len;
	unsigned long flags = 0;
	unsigned int nr_frags = 0;
	unsigned int mss = 0;
	int count = 0;
 	int tso;
#ifdef MAX_SKB_FRAGS
	unsigned int f;
	len -= skb->data_len;
#endif

#ifdef CONFIG_E1000_MQ
	tx_ring = *per_cpu_ptr(adapter->cpu_tx_ring, smp_processor_id());
#else
	tx_ring = adapter->tx_ring;
#endif

	if (unlikely(skb->len <= 0)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

#ifdef NETIF_F_TSO
	mss = skb_shinfo(skb)->gso_size;
	/* The controller does a simple calculation to
	 * make sure there is enough room in the FIFO before
	 * initiating the DMA for each buffer.  The calc is:
	 * 4 = ceil(buffer len/mss).  To make sure we don't
	 * overrun the FIFO, adjust the max buffer len if mss
	 * drops. */
	if(mss) {
        max_per_txd = min(mss << 0x2, max_per_txd);
        max_txd_pwr = fls(max_per_txd) - 0x1;
	}

    if((mss) || (skb->ip_summed == CHECKSUM_PARTIAL)) {
		count++;
    }
	count++;
#else
    if(skb->ip_summed == CHECKSUM_PARTIAL) {
		count++;
    {
#endif
	count += TXD_USE_COUNT(len, max_txd_pwr);

    if(adapter->pcix_82544) {
		count++;
    }
	/* work-around for errata 10 and it applies to all controllers
	 * in PCI-X mode, so add one more descriptor to the count
	 */
	if(unlikely((adapter->hw.bus_type == iegbe_bus_type_pcix) &&
            (len > 0x7df))) {
		count++;
    }
#ifdef MAX_SKB_FRAGS
	nr_frags = skb_shinfo(skb)->nr_frags;
	for(f = 0; f < nr_frags; f++)
		count += TXD_USE_COUNT(skb_shinfo(skb)->frags[f].size,
				       max_txd_pwr);
    if(adapter->pcix_82544) {
		count += nr_frags;
    }
#ifdef NETIF_F_TSO
	/* TSO Workaround for 82571/2 Controllers -- if skb->data
	 * points to just header, pull a few bytes of payload from
	 * frags into skb->data */
	if (skb_is_gso(skb)) {
		uint8_t hdr_len;
        hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
		if (skb->data_len && (hdr_len < (skb->len - skb->data_len)) &&
			(adapter->hw.mac_type == iegbe_82571 ||
			adapter->hw.mac_type == iegbe_82572)) {
			unsigned int pull_size;
            pull_size = min((unsigned int)0x4, skb->data_len);
			if (!__pskb_pull_tail(skb, pull_size)) {
				printk(KERN_ERR "__pskb_pull_tail failed.\n");
				dev_kfree_skb_any(skb);
				return -EFAULT;
			}
		}
	}
#endif
#endif

    if(adapter->hw.tx_pkt_filtering && (adapter->hw.mac_type == iegbe_82573) ) {
		iegbe_transfer_dhcp_info(adapter, skb);
    }
#ifdef NETIF_F_LLTX
	local_irq_save(flags);
	if (!spin_trylock(&tx_ring->tx_lock)) {
		/* Collision - tell upper layer to requeue */
		local_irq_restore(flags);
		return NETDEV_TX_LOCKED;
	}
#else
	spin_lock_irqsave(&tx_ring->tx_lock, flags);
#endif

	/* need: count + 2 desc gap to keep tail from touching
	 * head, otherwise try next time */
    if (unlikely(E1000_DESC_UNUSED(tx_ring) < count + 0x2)) {
		netif_stop_queue(netdev);
		spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
		return NETDEV_TX_BUSY;
	}

#ifndef NETIF_F_LLTX
	spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
#endif

#ifdef NETIF_F_HW_VLAN_TX
	if(vlan_tx_tag_present(skb)) {
		tx_flags |= E1000_TX_FLAGS_VLAN;
		tx_flags |= (vlan_tx_tag_get(skb) << E1000_TX_FLAGS_VLAN_SHIFT);
	}
#endif

	first = tx_ring->next_to_use;

	tso = iegbe_tso(adapter, tx_ring, skb);
	if (tso < 0) {
		dev_kfree_skb_any(skb);
#ifdef NETIF_F_LLTX
		spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
#endif
		return NETDEV_TX_OK;
	}

    if (likely(tso)) {
		tx_flags |= E1000_TX_FLAGS_TSO;
    } else if (likely(iegbe_tx_csum(adapter, tx_ring, skb))) {
		tx_flags |= E1000_TX_FLAGS_CSUM;
    }
	/* Old method was to assume IPv4 packet by default if TSO was enabled.
	 * 82571 hardware supports TSO capabilities for IPv6 as well...
	 * no longer assume, we must. */
    if (likely(skb->protocol == ntohs(ETH_P_IP))) {
		tx_flags |= E1000_TX_FLAGS_IPV4;
    }
	iegbe_tx_queue(adapter, tx_ring, tx_flags,
	               iegbe_tx_map(adapter, tx_ring, skb, first,
	                            max_per_txd, nr_frags, mss));

	netdev->trans_start = jiffies;

#ifdef NETIF_F_LLTX
	/* Make sure there is space in the ring for the next send. */
    if (unlikely(E1000_DESC_UNUSED(tx_ring) < MAX_SKB_FRAGS + 0x2)) {
		netif_stop_queue(netdev);
    }
	spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
#endif

	return NETDEV_TX_OK;
}

/**
 * iegbe_tx_timeout - Respond to a Tx Hang
 * @netdev: network interface device structure
 **/

static void
iegbe_tx_timeout(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);

	/* Do the reset outside of interrupt context */
	schedule_work(&adapter->tx_timeout_task);
}

#ifdef INITWORK21
static void
iegbe_tx_timeout_task(struct work_struct *work)
{
	struct iegbe_adapter *adapter =
		container_of(work, struct iegbe_adapter, tx_timeout_task);
	iegbe_down(adapter);
	iegbe_up(adapter);
}
#else
static void
iegbe_tx_timeout_task(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	iegbe_down(adapter);
	iegbe_up(adapter);
}
#endif

/**
 * iegbe_get_stats - Get System Network Statistics
 * @netdev: network interface device structure
 *
 * Returns the address of the device statistics structure.
 * The statistics are actually updated from the timer callback.
 **/

static struct net_device_stats *
iegbe_get_stats(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);

	iegbe_update_stats(adapter);
	return &adapter->net_stats;
}

/**
 * iegbe_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 **/

static int
iegbe_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	int max_frame = new_mtu + ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;

	if((max_frame < MINIMUM_ETHERNET_FRAME_SIZE) ||
	   (max_frame > MAX_JUMBO_FRAME_SIZE)) {
		DPRINTK(PROBE, ERR, "Invalid MTU setting\n");
		return -EINVAL;
	}

#define MAX_STD_JUMBO_FRAME_SIZE 9234
	/* might want this to be bigger enum check... */
	/* 82571 controllers limit jumbo frame size to 10500 bytes */
	if ((adapter->hw.mac_type == iegbe_82571 ||
	     adapter->hw.mac_type == iegbe_82572) &&
	    max_frame > MAX_STD_JUMBO_FRAME_SIZE) {
		DPRINTK(PROBE, ERR, "MTU > 9216 bytes not supported "
				    "on 82571 and 82572 controllers.\n");
		return -EINVAL;
	}

	if(adapter->hw.mac_type == iegbe_82573 &&
	    max_frame > MAXIMUM_ETHERNET_FRAME_SIZE) {
		DPRINTK(PROBE, ERR, "Jumbo Frames not supported "
				    "on 82573\n");
		return -EINVAL;
	}

	if(adapter->hw.mac_type > iegbe_82547_rev_2) {
		adapter->rx_buffer_len = max_frame;
        E1000_ROUNDUP(adapter->rx_buffer_len, 0x1024);
	} else {
		if(unlikely((adapter->hw.mac_type < iegbe_82543) &&
		   (max_frame > MAXIMUM_ETHERNET_FRAME_SIZE))) {
			DPRINTK(PROBE, ERR, "Jumbo Frames not supported "
					    "on 82542\n");
			return -EINVAL;

		} else {
			if(max_frame <= E1000_RXBUFFER_2048) {
				adapter->rx_buffer_len = E1000_RXBUFFER_2048;
			} else if(max_frame <= E1000_RXBUFFER_4096) {
				adapter->rx_buffer_len = E1000_RXBUFFER_4096;
			} else if(max_frame <= E1000_RXBUFFER_8192) {
				adapter->rx_buffer_len = E1000_RXBUFFER_8192;
			} else if(max_frame <= E1000_RXBUFFER_16384) {
				adapter->rx_buffer_len = E1000_RXBUFFER_16384;
			}
		}
	}

	netdev->mtu = new_mtu;

	if(netif_running(netdev)) {
		iegbe_down(adapter);
		iegbe_up(adapter);
	}

	adapter->hw.max_frame_size = max_frame;

	return 0;
}

/**
 * iegbe_update_stats - Update the board statistics counters
 * @adapter: board private structure
 **/

void
iegbe_update_stats(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	unsigned long flags = 0;

#define PHY_IDLE_ERROR_COUNT_MASK 0x00FF

	spin_lock_irqsave(&adapter->stats_lock, flags);

	/* these counters are modified from iegbe_adjust_tbi_stats,
	 * called from the interrupt context, so they must only
	 * be written while holding adapter->stats_lock
	 */

	adapter->stats.crcerrs += E1000_READ_REG(hw, CRCERRS);
	adapter->stats.gprc += E1000_READ_REG(hw, GPRC);
	adapter->stats.gorcl += E1000_READ_REG(hw, GORCL);
	adapter->stats.gorch += E1000_READ_REG(hw, GORCH);
	adapter->stats.bprc += E1000_READ_REG(hw, BPRC);
	adapter->stats.mprc += E1000_READ_REG(hw, MPRC);
	adapter->stats.roc += E1000_READ_REG(hw, ROC);
	adapter->stats.prc64 += E1000_READ_REG(hw, PRC64);
	adapter->stats.prc127 += E1000_READ_REG(hw, PRC127);
	adapter->stats.prc255 += E1000_READ_REG(hw, PRC255);
	adapter->stats.prc511 += E1000_READ_REG(hw, PRC511);
	adapter->stats.prc1023 += E1000_READ_REG(hw, PRC1023);
	adapter->stats.prc1522 += E1000_READ_REG(hw, PRC1522);

	adapter->stats.symerrs += E1000_READ_REG(hw, SYMERRS);
	adapter->stats.mpc += E1000_READ_REG(hw, MPC);
	adapter->stats.scc += E1000_READ_REG(hw, SCC);
	adapter->stats.ecol += E1000_READ_REG(hw, ECOL);
	adapter->stats.mcc += E1000_READ_REG(hw, MCC);
	adapter->stats.latecol += E1000_READ_REG(hw, LATECOL);
	adapter->stats.dc += E1000_READ_REG(hw, DC);
	adapter->stats.sec += E1000_READ_REG(hw, SEC);
	adapter->stats.rlec += E1000_READ_REG(hw, RLEC);
	adapter->stats.xonrxc += E1000_READ_REG(hw, XONRXC);
	adapter->stats.xontxc += E1000_READ_REG(hw, XONTXC);
	adapter->stats.xoffrxc += E1000_READ_REG(hw, XOFFRXC);
	adapter->stats.xofftxc += E1000_READ_REG(hw, XOFFTXC);
	adapter->stats.fcruc += E1000_READ_REG(hw, FCRUC);
	adapter->stats.gptc += E1000_READ_REG(hw, GPTC);
	adapter->stats.gotcl += E1000_READ_REG(hw, GOTCL);
	adapter->stats.gotch += E1000_READ_REG(hw, GOTCH);
	adapter->stats.rnbc += E1000_READ_REG(hw, RNBC);
	adapter->stats.ruc += E1000_READ_REG(hw, RUC);
	adapter->stats.rfc += E1000_READ_REG(hw, RFC);
	adapter->stats.rjc += E1000_READ_REG(hw, RJC);
	adapter->stats.torl += E1000_READ_REG(hw, TORL);
	adapter->stats.torh += E1000_READ_REG(hw, TORH);
	adapter->stats.totl += E1000_READ_REG(hw, TOTL);
	adapter->stats.toth += E1000_READ_REG(hw, TOTH);
	adapter->stats.tpr += E1000_READ_REG(hw, TPR);
	adapter->stats.ptc64 += E1000_READ_REG(hw, PTC64);
	adapter->stats.ptc127 += E1000_READ_REG(hw, PTC127);
	adapter->stats.ptc255 += E1000_READ_REG(hw, PTC255);
	adapter->stats.ptc511 += E1000_READ_REG(hw, PTC511);
	adapter->stats.ptc1023 += E1000_READ_REG(hw, PTC1023);
	adapter->stats.ptc1522 += E1000_READ_REG(hw, PTC1522);
	adapter->stats.mptc += E1000_READ_REG(hw, MPTC);
	adapter->stats.bptc += E1000_READ_REG(hw, BPTC);

	/* used for adaptive IFS */

	hw->tx_packet_delta = E1000_READ_REG(hw, TPT);
	adapter->stats.tpt += hw->tx_packet_delta;
	hw->collision_delta = E1000_READ_REG(hw, COLC);
	adapter->stats.colc += hw->collision_delta;

	if(hw->mac_type >= iegbe_82543) {
		adapter->stats.algnerrc += E1000_READ_REG(hw, ALGNERRC);
		adapter->stats.rxerrc += E1000_READ_REG(hw, RXERRC);
		adapter->stats.tncrs += E1000_READ_REG(hw, TNCRS);
		adapter->stats.cexterr += E1000_READ_REG(hw, CEXTERR);
		adapter->stats.tsctc += E1000_READ_REG(hw, TSCTC);
		adapter->stats.tsctfc += E1000_READ_REG(hw, TSCTFC);
	}
	if(hw->mac_type > iegbe_82547_rev_2) {
		adapter->stats.iac += E1000_READ_REG(hw, IAC);
		adapter->stats.icrxoc += E1000_READ_REG(hw, ICRXOC);
		adapter->stats.icrxptc += E1000_READ_REG(hw, ICRXPTC);
		adapter->stats.icrxatc += E1000_READ_REG(hw, ICRXATC);
		adapter->stats.ictxptc += E1000_READ_REG(hw, ICTXPTC);
		adapter->stats.ictxatc += E1000_READ_REG(hw, ICTXATC);
		adapter->stats.ictxqec += E1000_READ_REG(hw, ICTXQEC);
		adapter->stats.ictxqmtc += E1000_READ_REG(hw, ICTXQMTC);
		adapter->stats.icrxdmtc += E1000_READ_REG(hw, ICRXDMTC);
	}

	/* Fill out the OS statistics structure */

	adapter->net_stats.rx_packets = adapter->stats.gprc;
	adapter->net_stats.tx_packets = adapter->stats.gptc;
	adapter->net_stats.rx_bytes = adapter->stats.gorcl;
	adapter->net_stats.tx_bytes = adapter->stats.gotcl;
	adapter->net_stats.multicast = adapter->stats.mprc;
	adapter->net_stats.collisions = adapter->stats.colc;

	/* Rx Errors */

	adapter->net_stats.rx_errors = adapter->stats.rxerrc +
		adapter->stats.crcerrs + adapter->stats.algnerrc +
		adapter->stats.rlec + adapter->stats.mpc +
		adapter->stats.cexterr;
	adapter->net_stats.rx_dropped = adapter->stats.mpc;
	adapter->net_stats.rx_length_errors = adapter->stats.rlec;
	adapter->net_stats.rx_crc_errors = adapter->stats.crcerrs;
	adapter->net_stats.rx_frame_errors = adapter->stats.algnerrc;
	adapter->net_stats.rx_fifo_errors = adapter->stats.mpc;
	adapter->net_stats.rx_missed_errors = adapter->stats.mpc;

	/* Tx Errors */

	adapter->net_stats.tx_errors = adapter->stats.ecol +
	                               adapter->stats.latecol;
	adapter->net_stats.tx_aborted_errors = adapter->stats.ecol;
	adapter->net_stats.tx_window_errors = adapter->stats.latecol;
	adapter->net_stats.tx_carrier_errors = adapter->stats.tncrs;

	/* Tx Dropped needs to be maintained elsewhere */

	/* Phy Stats */

#if 0
	if(hw->media_type == iegbe_media_type_copper
       || (hw->media_type == iegbe_media_type_oem
           && iegbe_oem_phy_is_copper(&adapter->hw))) {
		uint16_t phy_tmp;
		if((adapter->link_speed == SPEED_1000) &&
		   (!iegbe_read_phy_reg(hw, PHY_1000T_STATUS, &phy_tmp))) {
			phy_tmp &= PHY_IDLE_ERROR_COUNT_MASK;
			adapter->phy_stats.idle_errors += phy_tmp;
		}

		if((hw->mac_type <= iegbe_82546) &&
		   (hw->phy_type == iegbe_phy_m88) &&
           !iegbe_read_phy_reg(hw, M88E1000_RX_ERR_CNTR, &phy_tmp)) {
			adapter->phy_stats.receive_errors += phy_tmp;
	}
    }
#endif

	spin_unlock_irqrestore(&adapter->stats_lock, flags);
}

#ifdef CONFIG_E1000_MQ
void
iegbe_rx_schedule(void *data)
{
	struct net_device *poll_dev, *netdev = data;
	struct iegbe_adapter *adapter = netdev->priv;
	int this_cpu = get_cpu();

	poll_dev = *per_cpu_ptr(adapter->cpu_netdev, this_cpu);
	if (poll_dev == NULL) {
		put_cpu();
		return;
	}

    if (likely(netif_rx_schedule_prep(poll_dev))) {
		__netif_rx_schedule(poll_dev);
    } else {
		iegbe_irq_enable(adapter);
    }
	put_cpu();
}
#endif

/**
 * iegbe_intr - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 * @pt_regs: CPU registers structure
 **/

#ifdef CONFIG_IRQ_PASS_REGS
static irqreturn_t
iegbe_intr(int irq, void *data, struct pt_regs *regs)
#else
static irqreturn_t
iegbe_intr(int irq, void *data)
#endif
{
	struct net_device *netdev = data;
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
	 uint32_t rctl, tctl;
	uint32_t icr = E1000_READ_REG(hw, ICR);
#ifndef CONFIG_E1000_NAPI
    uint32_t i;
#endif

    if(unlikely(!icr)) {
		return IRQ_NONE;  /* Not our interrupt */
    }
	if(unlikely(icr & (E1000_ICR_RX_DESC_FIFO_PAR
                       | E1000_ICR_TX_DESC_FIFO_PAR
							  | E1000_ICR_PB
							  | E1000_ICR_CPP_TARGET
							  | E1000_ICR_CPP_MASTER ))) {

	    iegbe_irq_disable(adapter);
	    tctl = E1000_READ_REG(&adapter->hw, TCTL);
	    rctl = E1000_READ_REG(&adapter->hw, RCTL);
	    E1000_WRITE_REG(&adapter->hw, RCTL, rctl & ~E1000_TCTL_EN);
	    E1000_WRITE_REG(&adapter->hw, RCTL, rctl & ~E1000_RCTL_EN);

		 tasklet_data = (unsigned long) (icr + adapter->bd_number);
		 tasklet_schedule(&iegbe_reset_tasklet);

		return IRQ_HANDLED;
	}

#ifdef CONFIG_E1000_NAPI
	atomic_inc(&adapter->irq_sem);
    E1000_WRITE_REG(hw, IMC, ~0);
	E1000_WRITE_FLUSH(hw);
#ifdef CONFIG_E1000_MQ
	if (atomic_read(&adapter->rx_sched_call_data.count) == 0) {
		cpu_set(adapter->cpu_for_queue[0],
			adapter->rx_sched_call_data.cpumask);
		for (i = 1; i < adapter->num_queues; i++) {
			cpu_set(adapter->cpu_for_queue[i],
				adapter->rx_sched_call_data.cpumask);
			atomic_inc(&adapter->irq_sem);
		}
		atomic_set(&adapter->rx_sched_call_data.count, i);
		smp_call_async_mask(&adapter->rx_sched_call_data);
	} else {
        DEBUGOUT("call_data.count == %u\n",
              atomic_read(&adapter->rx_sched_call_data.count));
	}
#else
    if (likely(netif_rx_schedule_prep(&adapter->polling_netdev[0]))) {
		__netif_rx_schedule(&adapter->polling_netdev[0]);
    } else {
		iegbe_irq_enable(adapter);
    }
#endif

#else
	/* Writing IMC and IMS is needed for 82547.
	 * Due to Hub Link bus being occupied, an interrupt
	 * de-assertion message is not able to be sent.
	 * When an interrupt assertion message is generated later,
	 * two messages are re-ordered and sent out.
	 * That causes APIC to think 82547 is in de-assertion
	 * state, while 82547 is in assertion state, resulting
	 * in dead lock. Writing IMC forces 82547 into
	 * de-assertion state.
	 */
	if (hw->mac_type == iegbe_82547 || hw->mac_type == iegbe_82547_rev_2) {
		atomic_inc(&adapter->irq_sem);
		E1000_WRITE_REG(hw, IMC, ~0);
	}

	for (i = 0; i < E1000_MAX_INTR; i++)
		if(unlikely(!adapter->clean_rx(adapter, adapter->rx_ring) &
           !iegbe_clean_tx_irq(adapter, adapter->tx_ring))) {
			break;
        }

    if (hw->mac_type == iegbe_82547 || hw->mac_type == iegbe_82547_rev_2) {
		iegbe_irq_enable(adapter);
    }
#endif
#ifdef E1000_COUNT_ICR
	adapter->icr_txdw += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_txqe += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_lsc += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_rxseq += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_rxdmt += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_rxo += icr & 0x01UL;
    icr >>= 0x1;
	adapter->icr_rxt += icr & 0x01UL;
	if(hw->mac_type != iegbe_icp_xxxx) {
        icr >>= 0x2;
		adapter->icr_mdac += icr & 0x01UL;
        icr >>= 0x1;
		adapter->icr_rxcfg += icr & 0x01UL;
        icr >>= 0x1;
		adapter->icr_gpi += icr & 0x01UL;
	} else {
        icr >>= 0x4;
	}
	if(hw->mac_type == iegbe_icp_xxxx) {
        icr >>= 0xc;
		adapter->icr_pb += icr & 0x01UL;
        icr >>= 0x3;
		adapter->icr_intmem_icp_xxxx += icr & 0x01UL;
            icr >>= 0x1;
    		adapter->icr_cpp_target += icr & 0x01UL;
            icr >>= 0x1;
    		adapter->icr_cpp_master += icr & 0x01UL;
           icr >>= 0x1;
   		adapter->icr_stat += icr & 0x01UL;
	}
#endif

	return IRQ_HANDLED;
}

#ifdef CONFIG_E1000_NAPI
/**
 * iegbe_clean - NAPI Rx polling callback
 * @adapter: board private structure
 **/

static int
iegbe_clean(struct net_device *poll_dev, int *budget)
{
	struct iegbe_adapter *adapter;
	int work_to_do = min(*budget, poll_dev->quota);
	int tx_cleaned, i = 0, work_done = 0;

	/* Must NOT use netdev_priv macro here. */
	adapter = poll_dev->priv;

	/* Keep link state information with original netdev */
    if (!netif_carrier_ok(adapter->netdev)) {
		goto quit_polling;
    }
	while (poll_dev != &adapter->polling_netdev[i]) {
		i++;
        if (unlikely(i == adapter->num_queues)) {
			BUG();
	}
    }

	tx_cleaned = iegbe_clean_tx_irq(adapter, &adapter->tx_ring[i]);
	adapter->clean_rx(adapter, &adapter->rx_ring[i],
	                  &work_done, work_to_do);

	*budget -= work_done;
	poll_dev->quota -= work_done;

	/* If no Tx and not enough Rx work done, exit the polling mode */
	if((!tx_cleaned && (work_done == 0)) ||
	   !netif_running(adapter->netdev)) {
quit_polling:
		netif_rx_complete(poll_dev);
		iegbe_irq_enable(adapter);
		return 0;
	}

	return 1;
}

#endif


/**
 * iegbe_clean_tx_irq - Reclaim resources after transmit completes
 * @adapter: board private structure
 **/

static boolean_t
iegbe_clean_tx_irq(struct iegbe_adapter *adapter,
                   struct iegbe_tx_ring *tx_ring)
{
	struct net_device *netdev = adapter->netdev;
	struct iegbe_tx_desc *tx_desc, *eop_desc;
	struct iegbe_buffer *buffer_info;
	unsigned int i, eop;
	boolean_t cleaned = FALSE;

	i = tx_ring->next_to_clean;
	eop = tx_ring->buffer_info[i].next_to_watch;
	eop_desc = E1000_TX_DESC(*tx_ring, eop);

	while (eop_desc->upper.data & cpu_to_le32(E1000_TXD_STAT_DD)) {
		/* Premature writeback of Tx descriptors clear (free buffers
		 * and unmap pci_mapping) previous_buffer_info */
		if (likely(tx_ring->previous_buffer_info.skb != NULL)) {
			iegbe_unmap_and_free_tx_resource(adapter,
					&tx_ring->previous_buffer_info);
		}

		for (cleaned = FALSE; !cleaned; ) {
			tx_desc = E1000_TX_DESC(*tx_ring, i);
			buffer_info = &tx_ring->buffer_info[i];
			cleaned = (i == eop);

#ifdef NETIF_F_TSO
			if (!(netdev->features & NETIF_F_TSO)) {
#endif
				iegbe_unmap_and_free_tx_resource(adapter,
				                                 buffer_info);
#ifdef NETIF_F_TSO
			} else {
				if (cleaned) {
					memcpy(&tx_ring->previous_buffer_info,
					       buffer_info,
					       sizeof(struct iegbe_buffer));
					memset(buffer_info, 0,
					       sizeof(struct iegbe_buffer));
				} else {
					iegbe_unmap_and_free_tx_resource(
					    adapter, buffer_info);
				}
			}
#endif

			tx_desc->buffer_addr = 0;
			tx_desc->lower.data = 0;
			tx_desc->upper.data = 0;

            if (unlikely(++i == tx_ring->count)) { i = 0; }
		}

		tx_ring->pkt++;

		eop = tx_ring->buffer_info[i].next_to_watch;
		eop_desc = E1000_TX_DESC(*tx_ring, eop);
	}

	tx_ring->next_to_clean = i;

	spin_lock(&tx_ring->tx_lock);

	if (unlikely(cleaned && netif_queue_stopped(netdev) &&
            netif_carrier_ok(netdev))) {
		netif_wake_queue(netdev);
    }
	spin_unlock(&tx_ring->tx_lock);

	if (adapter->detect_tx_hung) {
		/* Detect a transmit hang in hardware, this serializes the
		 * check with the clearing of time_stamp and movement of i */
		adapter->detect_tx_hung = FALSE;

		if (tx_ring->buffer_info[i].dma &&
		    time_after(jiffies, tx_ring->buffer_info[i].time_stamp + HZ)
		    && !(E1000_READ_REG(&adapter->hw, STATUS) &
			E1000_STATUS_TXOFF)) {

			/* detected Tx unit hang */
			i = tx_ring->next_to_clean;
			eop = tx_ring->buffer_info[i].next_to_watch;
			eop_desc = E1000_TX_DESC(*tx_ring, eop);
			DPRINTK(DRV, ERR, "Detected Tx Unit Hang\n"
					"  TDH                  <%x>\n"
					"  TDT                  <%x>\n"
					"  next_to_use          <%x>\n"
					"  next_to_clean        <%x>\n"
					"buffer_info[next_to_clean]\n"
					"  dma                  <%zx>\n"
					"  time_stamp           <%lx>\n"
					"  next_to_watch        <%x>\n"
					"  jiffies              <%lx>\n"
					"  next_to_watch.status <%x>\n",
				readl(adapter->hw.hw_addr + tx_ring->tdh),
				readl(adapter->hw.hw_addr + tx_ring->tdt),
				tx_ring->next_to_use,
				i,
				(size_t)tx_ring->buffer_info[i].dma,
				tx_ring->buffer_info[i].time_stamp,
				eop,
				jiffies,
				eop_desc->upper.fields.status);
			netif_stop_queue(netdev);
		}
	}
#ifdef NETIF_F_TSO
	if (unlikely(!(eop_desc->upper.data & cpu_to_le32(E1000_TXD_STAT_DD)) &&
        time_after(jiffies, tx_ring->previous_buffer_info.time_stamp + HZ))) {
		iegbe_unmap_and_free_tx_resource(
		    adapter, &tx_ring->previous_buffer_info);
    }
#endif
	return cleaned;
}

/**
 * iegbe_rx_checksum - Receive Checksum Offload for 82543
 * @adapter:     board private structure
 * @status_err:  receive descriptor status and error fields
 * @csum:        receive descriptor csum field
 * @sk_buff:     socket buffer with received data
 **/

static inline void
iegbe_rx_checksum(struct iegbe_adapter *adapter,
		  uint32_t status_err, uint32_t csum,
		  struct sk_buff *skb)
{
	uint16_t status = (uint16_t)status_err;
    uint8_t errors = (uint8_t)(status_err >> 0x18);
	skb->ip_summed = CHECKSUM_NONE;

	/* 82543 or newer only */
    if(unlikely(adapter->hw.mac_type < iegbe_82543))  { return; }
	/* Ignore Checksum bit is set */
    if(unlikely(status & E1000_RXD_STAT_IXSM)) { return; }
	/* TCP/UDP checksum error bit is set */
	if(unlikely(errors & E1000_RXD_ERR_TCPE)) {
		/* let the stack verify checksum errors */
		adapter->hw_csum_err++;
		return;
	}
	/* TCP/UDP Checksum has not been calculated */
	if(adapter->hw.mac_type <= iegbe_82547_rev_2) {
        if(!(status & E1000_RXD_STAT_TCPCS)) {
			return;
        }
	} else {
        if(!(status & (E1000_RXD_STAT_TCPCS | E1000_RXD_STAT_UDPCS))) {
			return;
	}
    }
	/* It must be a TCP or UDP packet with a valid checksum */
	if(likely(status & E1000_RXD_STAT_TCPCS)) {
		/* TCP checksum is good */
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else if(adapter->hw.mac_type > iegbe_82547_rev_2) {
		/* IP fragment with UDP payload */
		/* Hardware complements the payload checksum, so we undo it
		 * and then put the value in host order for further stack use.
		 */
		csum = ntohl(csum ^ 0xFFFF);
		skb->csum = csum;
		skb->ip_summed = CHECKSUM_PARTIAL;
	}
	adapter->hw_csum_good++;
}

/**
 * iegbe_clean_rx_irq - Send received data up the network stack; legacy
 * @adapter: board private structure
 **/

static boolean_t
#ifdef CONFIG_E1000_NAPI
iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                   struct iegbe_rx_ring *rx_ring,
                   int *work_done, int work_to_do)
#else
iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                   struct iegbe_rx_ring *rx_ring)
#endif
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	struct iegbe_rx_desc *rx_desc;
	struct iegbe_buffer *buffer_info;
	struct sk_buff *skb;
	unsigned long flags = 0;
	uint32_t length;
	uint8_t last_byte;
	unsigned int i;
	boolean_t cleaned = FALSE;

	i = rx_ring->next_to_clean;
	rx_desc = E1000_RX_DESC(*rx_ring, i);

	while(rx_desc->status & E1000_RXD_STAT_DD) {
		buffer_info = &rx_ring->buffer_info[i];
#ifdef CONFIG_E1000_NAPI
        if(*work_done >= work_to_do) {
			break;
        }
		(*work_done)++;
#endif
		cleaned = TRUE;

		pci_unmap_single(pdev,
		                 buffer_info->dma,
		                 buffer_info->length,
		                 PCI_DMA_FROMDEVICE);

		skb = buffer_info->skb;
		length = le16_to_cpu(rx_desc->length);

		if(unlikely(!(rx_desc->status & E1000_RXD_STAT_EOP))) {
			/* All receives must fit into a single buffer */
			E1000_DBG("%s: Receive packet consumed multiple"
				  " buffers\n", netdev->name);
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		if(unlikely(rx_desc->errors & E1000_RXD_ERR_FRAME_ERR_MASK)) {
            last_byte = *(skb->data + length - 0x1);
			if(TBI_ACCEPT(&adapter->hw, rx_desc->status,
			              rx_desc->errors, length, last_byte)) {
				spin_lock_irqsave(&adapter->stats_lock, flags);
				iegbe_tbi_adjust_stats(&adapter->hw,
				                       &adapter->stats,
				                       length, skb->data);
				spin_unlock_irqrestore(&adapter->stats_lock,
				                       flags);
				length--;
			} else {
				dev_kfree_skb_irq(skb);
				goto next_desc;
			}
		}

		/* Good Receive */
		skb_put(skb, length - ETHERNET_FCS_SIZE);

		/* Receive Checksum Offload */
		iegbe_rx_checksum(adapter,
				  (uint32_t)(rx_desc->status) |
                  ((uint32_t)(rx_desc->errors) << 0x18),
				  rx_desc->csum, skb);
		skb->protocol = eth_type_trans(skb, netdev);
#ifdef CONFIG_E1000_NAPI
#ifdef NETIF_F_HW_VLAN_TX
		if(rx_desc->status & E1000_RXD_STAT_VP) {
			u16 vid;
			vid = le16_to_cpu(rx_desc->special) &
				E1000_RXD_SPC_VLAN_MASK
			__vlan_hwaccel_put_tag(skb, vid);
		}
#endif
		netif_receive_skb(skb);
#else /* CONFIG_E1000_NAPI */
#ifdef NETIF_F_HW_VLAN_TX
		if(rx_desc->status & E1000_RXD_STAT_VP) {
			__vlan_hwaccel_put_tag(skb,
					le16_to_cpu(rx_desc->special) &
					E1000_RXD_SPC_VLAN_MASK);
		}
#endif
		netif_rx(skb);
#endif /* CONFIG_E1000_NAPI */
		netdev->last_rx = jiffies;
		rx_ring->pkt++;

next_desc:
		rx_desc->status = 0;
		buffer_info->skb = NULL;
        if(unlikely(++i == rx_ring->count)) { i = 0; }

		rx_desc = E1000_RX_DESC(*rx_ring, i);
	}
	rx_ring->next_to_clean = i;

	adapter->alloc_rx_buf(adapter, rx_ring);

	return cleaned;
}

/**
 * iegbe_clean_rx_irq_ps - Send received data up the network stack; packet split
 * @adapter: board private structure
 **/

static boolean_t
#ifdef CONFIG_E1000_NAPI
iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                      struct iegbe_rx_ring *rx_ring,
                      int *work_done, int work_to_do)
#else
iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                      struct iegbe_rx_ring *rx_ring)
#endif
{
	union iegbe_rx_desc_packet_split *rx_desc;
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	struct iegbe_buffer *buffer_info;
	struct iegbe_ps_page *ps_page;
	struct iegbe_ps_page_dma *ps_page_dma;
	struct sk_buff *skb;
	unsigned int i, j;
	uint32_t length, staterr;
	boolean_t cleaned = FALSE;

	i = rx_ring->next_to_clean;
	rx_desc = E1000_RX_DESC_PS(*rx_ring, i);
	staterr = le32_to_cpu(rx_desc->wb.middle.status_error);

	while(staterr & E1000_RXD_STAT_DD) {
		buffer_info = &rx_ring->buffer_info[i];
		ps_page = &rx_ring->ps_page[i];
		ps_page_dma = &rx_ring->ps_page_dma[i];
#ifdef CONFIG_E1000_NAPI
        if(unlikely(*work_done >= work_to_do)) {
			break;
        }
		(*work_done)++;
#endif
		cleaned = TRUE;

		pci_unmap_single(pdev, buffer_info->dma,
				 buffer_info->length,
				 PCI_DMA_FROMDEVICE);

		skb = buffer_info->skb;

		if(unlikely(!(staterr & E1000_RXD_STAT_EOP))) {
			E1000_DBG("%s: Packet Split buffers didn't pick up"
				  " the full packet\n", netdev->name);
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		if(unlikely(staterr & E1000_RXDEXT_ERR_FRAME_ERR_MASK)) {
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		length = le16_to_cpu(rx_desc->wb.middle.length0);

		if(unlikely(!length)) {
			E1000_DBG("%s: Last part of the packet spanning"
				  " multiple descriptors\n", netdev->name);
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		/* Good Receive */
		skb_put(skb, length);

		for(j = 0; j < adapter->rx_ps_pages; j++) {
            if(!(length = le16_to_cpu(rx_desc->wb.upper.length[j]))) {
				break;
            }
			pci_unmap_page(pdev, ps_page_dma->ps_page_dma[j],
					PAGE_SIZE, PCI_DMA_FROMDEVICE);
			ps_page_dma->ps_page_dma[j] = 0;
			skb_shinfo(skb)->frags[j].page.p =
				ps_page->ps_page[j];
			ps_page->ps_page[j] = NULL;
			skb_shinfo(skb)->frags[j].page_offset = 0;
			skb_shinfo(skb)->frags[j].size = length;
			skb_shinfo(skb)->nr_frags++;
			skb->len += length;
			skb->data_len += length;
		}

		iegbe_rx_checksum(adapter, staterr,
				  rx_desc->wb.lower.hi_dword.csum_ip.csum, skb);
		skb->protocol = eth_type_trans(skb, netdev);

		if(likely(rx_desc->wb.upper.header_status &
			  E1000_RXDPS_HDRSTAT_HDRSP)) {
			adapter->rx_hdr_split++;
#ifdef HAVE_RX_ZERO_COPY
			skb_shinfo(skb)->zero_copy = TRUE;
#endif
	        }
#ifdef CONFIG_E1000_NAPI
#ifdef NETIF_F_HW_VLAN_TX
		if(staterr & E1000_RXD_STAT_VP) {
			u16 vid;
			vid = le16_to_cpu(rx_desc->wb.middle.vlan) &
				E1000_RXD_SPC_VLAN_MASK;
			__vlan_hwaccel_put_tag(skb, vid);
		}
#endif
		netif_receive_skb(skb);
#else /* CONFIG_E1000_NAPI */
#ifdef NETIF_F_HW_VLAN_TX
		if(staterr & E1000_RXD_STAT_VP) {
			__vlan_hwaccel_put_tag(skb, 
				le16_to_cpu(rx_desc->wb.middle.vlan) &
				E1000_RXD_SPC_VLAN_MASK);
		}
#endif
		netif_rx(skb);
#endif /* CONFIG_E1000_NAPI */
		netdev->last_rx = jiffies;
		rx_ring->pkt++;

next_desc:
		rx_desc->wb.middle.status_error &= ~0xFF;
		buffer_info->skb = NULL;
        if(unlikely(++i == rx_ring->count)) { i = 0; }

		rx_desc = E1000_RX_DESC_PS(*rx_ring, i);
		staterr = le32_to_cpu(rx_desc->wb.middle.status_error);
	}
	rx_ring->next_to_clean = i;

	adapter->alloc_rx_buf(adapter, rx_ring);

	return cleaned;
}

/**
 * iegbe_alloc_rx_buffers - Replace used receive buffers; legacy & extended
 * @adapter: address of board private structure
 **/
static void
iegbe_alloc_rx_buffers(struct iegbe_adapter *adapter,
                       struct iegbe_rx_ring *rx_ring)
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	struct iegbe_rx_desc *rx_desc;
	struct iegbe_buffer *buffer_info;
	struct sk_buff *skb;
	unsigned int i;
	unsigned int bufsz = adapter->rx_buffer_len + NET_IP_ALIGN + IEGBE_ADBUF;

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];

	while(!buffer_info->skb) {
		skb = dev_alloc_skb(bufsz);

		if(unlikely(!skb)) {
			/* Better luck next round */
			break;
		}

		/* Fix for errata 23, can't cross 64kB boundary */
		if(!iegbe_check_64k_bound(adapter, skb->data, bufsz)) {
			struct sk_buff *oldskb = skb;
			DPRINTK(RX_ERR, ERR, "skb align check failed: %u bytes "
					     "at %p\n", bufsz, skb->data);
			/* Try again, without freeing the previous */
			skb = dev_alloc_skb(bufsz);
			/* Failed allocation, critical failure */
			if(!skb) {
				dev_kfree_skb(oldskb);
				break;
			}

			if(!iegbe_check_64k_bound(adapter, skb->data, bufsz)) {
				/* give up */
				dev_kfree_skb(skb);
				dev_kfree_skb(oldskb);
				break; /* while !buffer_info->skb */
			} else {
				/* Use new allocation */
				dev_kfree_skb(oldskb);
			}
		}
		/* Make buffer alignment 2 beyond a 16 byte boundary
		 * this will result in a 16 byte aligned IP header after
		 * the 14 byte MAC header is removed
		 */
		skb_reserve(skb, NET_IP_ALIGN + IEGBE_ADBUF);

		skb->dev = netdev;

		buffer_info->skb = skb;
		buffer_info->length = adapter->rx_buffer_len;
		buffer_info->dma = pci_map_single(pdev,
						  skb->data,
						  adapter->rx_buffer_len,
						  PCI_DMA_FROMDEVICE);

		/* Fix for errata 23, can't cross 64kB boundary */
		if(!iegbe_check_64k_bound(adapter,
					(void *)(unsigned long)buffer_info->dma,
					adapter->rx_buffer_len)) {
			DPRINTK(RX_ERR, ERR,
				"dma align check failed: %u bytes at %p\n",
				adapter->rx_buffer_len,
				(void *)(unsigned long)buffer_info->dma);
			dev_kfree_skb(skb);
			buffer_info->skb = NULL;

			pci_unmap_single(pdev, buffer_info->dma,
					 adapter->rx_buffer_len,
					 PCI_DMA_FROMDEVICE);

			break; /* while !buffer_info->skb */
		}
		rx_desc = E1000_RX_DESC(*rx_ring, i);
		rx_desc->buffer_addr = cpu_to_le64(buffer_info->dma);

        if(unlikely((i & ~(E1000_RX_BUFFER_WRITE - 0x1)) == i)) {
			/* Force memory writes to complete before letting h/w
			 * know there are new descriptors to fetch.  (Only
			 * applicable for weak-ordered memory model archs,
			 * such as IA-64). */
			wmb();
			writel(i, adapter->hw.hw_addr + rx_ring->rdt);
		}
        if(unlikely(++i == rx_ring->count)) { i = 0; }
		buffer_info = &rx_ring->buffer_info[i];
	}

	rx_ring->next_to_use = i;
}

/**
 * iegbe_alloc_rx_buffers_ps - Replace used receive buffers; packet split
 * @adapter: address of board private structure
 **/

static void
iegbe_alloc_rx_buffers_ps(struct iegbe_adapter *adapter,
                          struct iegbe_rx_ring *rx_ring)
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	union iegbe_rx_desc_packet_split *rx_desc;
	struct iegbe_buffer *buffer_info;
	struct iegbe_ps_page *ps_page;
	struct iegbe_ps_page_dma *ps_page_dma;
	struct sk_buff *skb;
	unsigned int i, j;

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];
	ps_page = &rx_ring->ps_page[i];
	ps_page_dma = &rx_ring->ps_page_dma[i];

	while(!buffer_info->skb) {
		rx_desc = E1000_RX_DESC_PS(*rx_ring, i);

		for (j = 0; j < PS_PAGE_BUFFERS; j++) {
			if (j < adapter->rx_ps_pages) {
				if (likely(!ps_page->ps_page[j])) {
					ps_page->ps_page[j] =
						alloc_page(GFP_ATOMIC);
                    if (unlikely(!ps_page->ps_page[j])) {
						goto no_buffers;
                    }
					ps_page_dma->ps_page_dma[j] =
						pci_map_page(pdev,
							    ps_page->ps_page[j],
							    0, PAGE_SIZE,
							    PCI_DMA_FROMDEVICE);
				}
				/* Refresh the desc even if buffer_addrs didn't
				 * change because each write-back erases
				 * this info.
				 */
                rx_desc->read.buffer_addr[j+0x1] =
				     cpu_to_le64(ps_page_dma->ps_page_dma[j]);
            } else {
                rx_desc->read.buffer_addr[j+0x1] = ~0;
            }
		}

		skb = dev_alloc_skb(adapter->rx_ps_bsize0 + NET_IP_ALIGN + IEGBE_ADBUF);

        if (unlikely(!skb)) {
			break;
        }
		/* Make buffer alignment 2 beyond a 16 byte boundary
		 * this will result in a 16 byte aligned IP header after
		 * the 14 byte MAC header is removed
		 */
		skb_reserve(skb, NET_IP_ALIGN + IEGBE_ADBUF);

		skb->dev = netdev;

		buffer_info->skb = skb;
		buffer_info->length = adapter->rx_ps_bsize0;
		buffer_info->dma = pci_map_single(pdev, skb->data,
						  adapter->rx_ps_bsize0,
						  PCI_DMA_FROMDEVICE);

		rx_desc->read.buffer_addr[0] = cpu_to_le64(buffer_info->dma);

        if (unlikely((i & ~(E1000_RX_BUFFER_WRITE - 0x1)) == i)) {
			/* Force memory writes to complete before letting h/w
			 * know there are new descriptors to fetch.  (Only
			 * applicable for weak-ordered memory model archs,
			 * such as IA-64). */
			wmb();
			/* Hardware increments by 16 bytes, but packet split
			 * descriptors are 32 bytes...so we increment tail
			 * twice as much.
			 */
			writel(i<<1, adapter->hw.hw_addr + rx_ring->rdt);
		}

        if (unlikely(++i == rx_ring->count)) { i = 0; }
		buffer_info = &rx_ring->buffer_info[i];
		ps_page = &rx_ring->ps_page[i];
		ps_page_dma = &rx_ring->ps_page_dma[i];
	}

no_buffers:
	rx_ring->next_to_use = i;
}

/**
 * iegbe_smartspeed - Workaround for SmartSpeed on 82541 and 82547 controllers.
 * @adapter:
 **/

static void
iegbe_smartspeed(struct iegbe_adapter *adapter)
{
	uint16_t phy_status;
	uint16_t phy_ctrl;

	if((adapter->hw.phy_type != iegbe_phy_igp) || !adapter->hw.autoneg ||
       !(adapter->hw.autoneg_advertised & ADVERTISE_1000_FULL)) {
		return;
    }
	if(adapter->smartspeed == 0) {
		/* If Master/Slave config fault is asserted twice,
		 * we assume back-to-back */
		iegbe_read_phy_reg(&adapter->hw, PHY_1000T_STATUS, &phy_status);
        if(!(phy_status & SR_1000T_MS_CONFIG_FAULT)) { return; }
		iegbe_read_phy_reg(&adapter->hw, PHY_1000T_STATUS, &phy_status);
        if(!(phy_status & SR_1000T_MS_CONFIG_FAULT)) { return; }
		iegbe_read_phy_reg(&adapter->hw, PHY_1000T_CTRL, &phy_ctrl);
		if(phy_ctrl & CR_1000T_MS_ENABLE) {
			phy_ctrl &= ~CR_1000T_MS_ENABLE;
			iegbe_write_phy_reg(&adapter->hw, PHY_1000T_CTRL,
					    phy_ctrl);
			adapter->smartspeed++;
			if(!iegbe_phy_setup_autoneg(&adapter->hw) &&
			   !iegbe_read_phy_reg(&adapter->hw, PHY_CTRL,
				   	       &phy_ctrl)) {
				phy_ctrl |= (MII_CR_AUTO_NEG_EN |
					     MII_CR_RESTART_AUTO_NEG);
				iegbe_write_phy_reg(&adapter->hw, PHY_CTRL,
						    phy_ctrl);
			}
		}
		return;
	} else if(adapter->smartspeed == E1000_SMARTSPEED_DOWNSHIFT) {
		/* If still no link, perhaps using 2/3 pair cable */
		iegbe_read_phy_reg(&adapter->hw, PHY_1000T_CTRL, &phy_ctrl);
		phy_ctrl |= CR_1000T_MS_ENABLE;
		iegbe_write_phy_reg(&adapter->hw, PHY_1000T_CTRL, phy_ctrl);
		if(!iegbe_phy_setup_autoneg(&adapter->hw) &&
		   !iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_ctrl)) {
			phy_ctrl |= (MII_CR_AUTO_NEG_EN |
				     MII_CR_RESTART_AUTO_NEG);
			iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, phy_ctrl);
		}
	}
	/* Restart process after E1000_SMARTSPEED_MAX iterations */
    if(adapter->smartspeed++ == E1000_SMARTSPEED_MAX) {
		adapter->smartspeed = 0;
}
}

/**
 * iegbe_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int
iegbe_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
#ifdef SIOCGMIIPHY
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		return iegbe_mii_ioctl(netdev, ifr, cmd);
#endif
#ifdef ETHTOOL_OPS_COMPAT
	case SIOCETHTOOL:
		return ethtool_ioctl(ifr);
#endif
	default:
		return -EOPNOTSUPP;
	}
}

#ifdef SIOCGMIIPHY
/**
 * iegbe_mii_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int
iegbe_mii_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct mii_ioctl_data *data = if_mii(ifr);
	int retval;
	uint16_t mii_reg;
	uint16_t spddplx;
	unsigned long flags;

	if((adapter->hw.media_type == iegbe_media_type_oem &&
           !iegbe_oem_phy_is_copper(&adapter->hw)) ||
        adapter->hw.media_type == iegbe_media_type_fiber ||
        adapter->hw.media_type == iegbe_media_type_internal_serdes ) {
		return -EOPNOTSUPP;
    }
	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = adapter->hw.phy_addr;
		break;
	case SIOCGMIIREG:
        if(!capable(CAP_NET_ADMIN)) {
			return -EPERM;
        }
		spin_lock_irqsave(&adapter->stats_lock, flags);
		if(iegbe_read_phy_phy_reg(&adapter->hw, data->phy_id,
					  data->reg_num & 0x1F,
					  &data->val_out)) {
			spin_unlock_irqrestore(&adapter->stats_lock, flags);
			return -EIO;
		}
		spin_unlock_irqrestore(&adapter->stats_lock, flags);
		break;
	case SIOCSMIIREG:
        if(!capable(CAP_NET_ADMIN)){
			return -EPERM;
        }
        if(data->reg_num & ~(0x1F)) {
			return -EFAULT;
        }
		mii_reg = data->val_in;
		spin_lock_irqsave(&adapter->stats_lock, flags);
		if(iegbe_write_phy_phy_reg(&adapter->hw, data->phy_id,
					   data->reg_num, mii_reg)) {
			spin_unlock_irqrestore(&adapter->stats_lock, flags);
			return -EIO;
		}
		switch(adapter->hw.phy_type) {
		case iegbe_phy_m88:
			switch (data->reg_num) {
			case PHY_CTRL:
                if(mii_reg & MII_CR_POWER_DOWN) {
					break;
                }
				if(mii_reg & MII_CR_AUTO_NEG_EN) {
					adapter->hw.autoneg = 1;
					adapter->hw.autoneg_advertised = 0x2F;
				} else {
                    if(mii_reg & 0x40){
						spddplx = SPEED_1000;
                    } else if(mii_reg & 0x2000) {
						spddplx = SPEED_100;
                    } else {
						spddplx = SPEED_10;
                          }
					spddplx += (mii_reg & 0x100)
						   ? FULL_DUPLEX :
						   HALF_DUPLEX;
					retval = iegbe_set_spd_dplx(adapter,
								    spddplx);
					if(retval) {
						spin_unlock_irqrestore(
							&adapter->stats_lock,
							flags);
						return retval;
					}
				}
				if(netif_running(adapter->netdev)) {
					iegbe_down(adapter);
					iegbe_up(adapter);
                } else {
					iegbe_reset(adapter);
                }
				break;
			case M88E1000_PHY_SPEC_CTRL:
			case M88E1000_EXT_PHY_SPEC_CTRL:
				if(iegbe_phy_reset(&adapter->hw)) {
					spin_unlock_irqrestore(
						&adapter->stats_lock, flags);
					return -EIO;
				}
				break;
			}
			break;

		case iegbe_phy_oem:
			retval = iegbe_oem_mii_ioctl(adapter, flags, ifr, cmd);
			if(retval) {
				spin_unlock_irqrestore(
					&adapter->stats_lock, flags);
				return retval;
			}
			break;

		default:
			switch (data->reg_num) {
			case PHY_CTRL:
                if(mii_reg & MII_CR_POWER_DOWN) {
					break;
                }
				if(netif_running(adapter->netdev)) {
					iegbe_down(adapter);
					iegbe_up(adapter);
                } else {
					iegbe_reset(adapter);
                }
				break;
			}
		}
		spin_unlock_irqrestore(&adapter->stats_lock, flags);
		break;
	default:
		return -EOPNOTSUPP;
	}
	return E1000_SUCCESS;
}
#endif

void
iegbe_pci_set_mwi(struct iegbe_hw *hw)
{
	struct iegbe_adapter *adapter = hw->back;
#ifdef HAVE_PCI_SET_MWI
	int ret_val = pci_set_mwi(adapter->pdev);

    if(ret_val) {
		DPRINTK(PROBE, ERR, "Error in setting MWI\n");
    }
#else
	pci_write_config_word(adapter->pdev, PCI_COMMAND,
			      adapter->hw.pci_cmd_word |
			      PCI_COMMAND_INVALIDATE);
#endif
}

void
iegbe_pci_clear_mwi(struct iegbe_hw *hw)
{
	struct iegbe_adapter *adapter = hw->back;

#ifdef HAVE_PCI_SET_MWI
	pci_clear_mwi(adapter->pdev);
#else
	pci_write_config_word(adapter->pdev, PCI_COMMAND,
			      adapter->hw.pci_cmd_word &
			      ~PCI_COMMAND_INVALIDATE);
#endif
}

void
iegbe_read_pci_cfg(struct iegbe_hw *hw, uint32_t reg, uint16_t *value)
{
	struct iegbe_adapter *adapter = hw->back;

	pci_read_config_word(adapter->pdev, reg, value);
}

void
iegbe_write_pci_cfg(struct iegbe_hw *hw, uint32_t reg, uint16_t *value)
{
	struct iegbe_adapter *adapter = hw->back;

	pci_write_config_word(adapter->pdev, reg, *value);
}

uint32_t
iegbe_io_read(struct iegbe_hw *hw, unsigned long port)
{
	return inl(port);
}

void
iegbe_io_write(struct iegbe_hw *hw, unsigned long port, uint32_t value)
{
	outl(value, port);
}

#ifdef NETIF_F_HW_VLAN_TX

static bool
iegbe_vlan_used(struct iegbe_adapter *adapter)
{
	u16 vid;

	for_each_set_bit(vid, adapter->active_vlans, VLAN_N_VID)
		return true;
	return false;
}

static void
iegbe_vlan_filter_on_off(struct iegbe_adapter *adapter, bool filter_on)
{
	uint32_t ctrl, rctl;

	iegbe_irq_disable(adapter);

	if(filter_on) {
		/* enable VLAN tag insert/strip */
		ctrl = E1000_READ_REG(&adapter->hw, CTRL);
		ctrl |= E1000_CTRL_VME;
		E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);

		/* enable VLAN receive filtering */
		rctl = E1000_READ_REG(&adapter->hw, RCTL);
		rctl |= E1000_RCTL_VFE;
		rctl &= ~E1000_RCTL_CFIEN;
		E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
		iegbe_update_mng_vlan(adapter);
	} else {
		/* disable VLAN tag insert/strip */
		ctrl = E1000_READ_REG(&adapter->hw, CTRL);
		ctrl &= ~E1000_CTRL_VME;
		E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);

		/* disable VLAN filtering */
		rctl = E1000_READ_REG(&adapter->hw, RCTL);
		rctl &= ~E1000_RCTL_VFE;
		E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
	}

	iegbe_irq_enable(adapter);
}

static void
iegbe_vlan_rx_add_vid(struct net_device *netdev, uint16_t vid)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	uint32_t vfta, index;
	if((adapter->hw.mng_cookie.status &
		E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) &&
        (vid == adapter->mng_vlan_id)) {
		return;
    }
	/* add VID to filter table */
    index = (vid >> 0x5) & 0x7F;
	vfta = E1000_READ_REG_ARRAY(&adapter->hw, VFTA, index);
    vfta |= (0x1 << (vid & 0x1F));
	iegbe_write_vfta(&adapter->hw, index, vfta);

    set_bit(vid, adapter->active_vlans);
}

static void
iegbe_vlan_rx_kill_vid(struct net_device *netdev, uint16_t vid)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	uint32_t vfta, index;

	iegbe_irq_disable(adapter);
	iegbe_irq_enable(adapter);

	if((adapter->hw.mng_cookie.status &
		E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) &&
        (vid == adapter->mng_vlan_id)) {
		return;
    }
	/* remove VID from filter table */
    index = (vid >> 0x5) & 0x7F;
	vfta = E1000_READ_REG_ARRAY(&adapter->hw, VFTA, index);
    vfta &= ~(0x1 << (vid & 0x1F));
	iegbe_write_vfta(&adapter->hw, index, vfta);

    clear_bit(vid, adapter->active_vlans);

	if (!iegbe_vlan_used(adapter))
		iegbe_vlan_filter_on_off(adapter, false);
}

static void
iegbe_restore_vlan(struct iegbe_adapter *adapter)
{
	u16 vid;

	if (!iegbe_vlan_used(adapter))
		return;

	iegbe_vlan_filter_on_off(adapter, true);
	for_each_set_bit(vid, adapter->active_vlans, VLAN_N_VID)
		iegbe_vlan_rx_add_vid(adapter->netdev, vid);
}
#endif

int
iegbe_set_spd_dplx(struct iegbe_adapter *adapter, uint16_t spddplx)
{
	adapter->hw.autoneg = 0;

	/* Fiber NICs only allow 1000 gbps Full duplex */
	if((adapter->hw.media_type == iegbe_media_type_fiber
        || (adapter->hw.media_type == iegbe_media_type_oem
            && !iegbe_oem_phy_is_copper(&adapter->hw)))
       && spddplx != (SPEED_1000 + DUPLEX_FULL)) {
		DPRINTK(PROBE, ERR, "Unsupported Speed/Duplex configuration\n");
		return -EINVAL;
	}

	switch(spddplx) {
	case SPEED_10 + DUPLEX_HALF:
		adapter->hw.forced_speed_duplex = iegbe_10_half;
		break;
	case SPEED_10 + DUPLEX_FULL:
		adapter->hw.forced_speed_duplex = iegbe_10_full;
		break;
	case SPEED_100 + DUPLEX_HALF:
		adapter->hw.forced_speed_duplex = iegbe_100_half;
		break;
	case SPEED_100 + DUPLEX_FULL:
		adapter->hw.forced_speed_duplex = iegbe_100_full;
		break;
	case SPEED_1000 + DUPLEX_FULL:
        adapter->hw.autoneg = 0x1;
		adapter->hw.autoneg_advertised = ADVERTISE_1000_FULL;
		break;
	case SPEED_1000 + DUPLEX_HALF: /* not supported */
	default:
		DPRINTK(PROBE, ERR, "Unsupported Speed/Duplex configuration\n");
		return -EINVAL;
	}
	return 0;
}

static int
iegbe_notify_reboot(struct notifier_block *nb, unsigned long event, void *p)
{
	struct pci_dev *pdev = NULL;
	pm_message_t state = {0x3};

	switch(event) {
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		while((pdev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
			if(pci_dev_driver(pdev) == &iegbe_driver) {
				iegbe_suspend(pdev, state);
			}
			pci_dev_put(pdev);
		}
	}
	return NOTIFY_DONE;
}

static int
iegbe_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	uint32_t ctrl, ctrl_ext, rctl, manc, status, swsm;
	uint32_t wufc = adapter->wol;
	uint16_t cmd_word;

	netif_device_detach(netdev);

    if(netif_running(netdev)) {
		iegbe_down(adapter);
    }
	/*
	 * ICP_XXXX style MACs do not have a link up bit in
	 * the STATUS register, query the PHY directly
	 */
	if(adapter->hw.mac_type != iegbe_icp_xxxx) {
		status = E1000_READ_REG(&adapter->hw, STATUS);
        if(status & E1000_STATUS_LU) {
			wufc &= ~E1000_WUFC_LNKC;
        }
	} else {
		int isUp = 0;
        if(iegbe_oem_phy_is_link_up(&adapter->hw, &isUp) != E1000_SUCCESS) {
			isUp = 0;
        }
        if(isUp) {
			wufc &= ~E1000_WUFC_LNKC;
	}
    }

	if(wufc) {
		iegbe_setup_rctl(adapter);
		iegbe_set_multi(netdev);

		/* turn on all-multi mode if wake on multicast is enabled */
		if(adapter->wol & E1000_WUFC_MC) {
			rctl = E1000_READ_REG(&adapter->hw, RCTL);
			rctl |= E1000_RCTL_MPE;
			E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
		}

		if(adapter->hw.mac_type >= iegbe_82540) {
			ctrl = E1000_READ_REG(&adapter->hw, CTRL);
			/* advertise wake from D3Cold */
			#define E1000_CTRL_ADVD3WUC 0x00100000
			/* phy power management enable */
			ctrl |= E1000_CTRL_ADVD3WUC |
			        (adapter->hw.mac_type != iegbe_icp_xxxx
			                     ? E1000_CTRL_EN_PHY_PWR_MGMT : 0);

			E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);
		}

		if(adapter->hw.media_type == iegbe_media_type_fiber ||
		   adapter->hw.media_type == iegbe_media_type_internal_serdes) {
			/* keep the laser running in D3 */
			ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
			ctrl_ext |= E1000_CTRL_EXT_SDP7_DATA;
			E1000_WRITE_REG(&adapter->hw, CTRL_EXT, ctrl_ext);
		}

        /* Allow OEM PHYs (if any exist) to keep the laser
         *running in D3 */
        iegbe_oem_fiber_live_in_suspend(&adapter->hw);

		/* Allow time for pending master requests to run */
		iegbe_disable_pciex_master(&adapter->hw);

		E1000_WRITE_REG(&adapter->hw, WUC, E1000_WUC_PME_EN);
		E1000_WRITE_REG(&adapter->hw, WUFC, wufc);
        pci_enable_wake(pdev, 0x3, 0x1);
        pci_enable_wake(pdev, 0x4, 0x1); /* 4 == D3 cold */
	} else {
		E1000_WRITE_REG(&adapter->hw, WUC, 0);
		E1000_WRITE_REG(&adapter->hw, WUFC, 0);
        pci_enable_wake(pdev, 0x3, 0);
        pci_enable_wake(pdev, 0x4, 0); /* 4 == D3 cold */
	}

	pci_save_state(pdev);

	if(adapter->hw.mac_type >= iegbe_82540
	   && adapter->hw.mac_type != iegbe_icp_xxxx
	   && adapter->hw.media_type == iegbe_media_type_copper) {
		manc = E1000_READ_REG(&adapter->hw, MANC);
		if(manc & E1000_MANC_SMBUS_EN) {
			manc |= E1000_MANC_ARP_EN;
			E1000_WRITE_REG(&adapter->hw, MANC, manc);
            pci_enable_wake(pdev, 0x3, 0x1);
            pci_enable_wake(pdev, 0x4, 0x1); /* 4 == D3 cold */
		}
	}

	switch(adapter->hw.mac_type) {
	case iegbe_82571:
	case iegbe_82572:
		ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
		E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
				ctrl_ext & ~E1000_CTRL_EXT_DRV_LOAD);
		break;
	case iegbe_82573:
		swsm = E1000_READ_REG(&adapter->hw, SWSM);
		E1000_WRITE_REG(&adapter->hw, SWSM,
				swsm & ~E1000_SWSM_DRV_LOAD);
		break;
	default:
		break;
	}

	pci_disable_device(pdev);
	if(adapter->hw.mac_type == iegbe_icp_xxxx) {
		/*
		 * ICP xxxx devices are not true PCI devices, in the context
		 * of power management, disabling the bus mastership is not
		 * sufficient to disable the device, it is also necessary to
		 * disable IO, Memory, and Interrupts if they are enabled.
		 */
		pci_read_config_word(pdev, PCI_COMMAND, &cmd_word);
        if(cmd_word & PCI_COMMAND_IO) {
			cmd_word &= ~PCI_COMMAND_IO;
        }
        if(cmd_word & PCI_COMMAND_MEMORY) {
			cmd_word &= ~PCI_COMMAND_MEMORY;
        }
        if(cmd_word & PCI_COMMAND_INTX_DISABLE) {
			cmd_word &= ~PCI_COMMAND_INTX_DISABLE;
        }
		pci_write_config_word(pdev, PCI_COMMAND, cmd_word);
	}

    state.event = (state.event > 0) ? 0x3 : 0;
	pci_set_power_state(pdev, state.event);
	 if(gcu_suspend == 0)
	 {
	 	if(gcu == NULL) {
			gcu = pci_get_device(PCI_VENDOR_ID_INTEL, GCU_DEVID, NULL);
		}
	 	gcu_iegbe_suspend(gcu, 0x3);
	 	gcu_suspend = 1;
	 	gcu_resume = 0;
	 }
	return 0;
}

#ifdef CONFIG_PM
static int
iegbe_resume(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	uint32_t manc, ret_val, swsm;
	uint32_t ctrl_ext;
	 int offset;
    uint32_t vdid;

	 if(gcu_resume == 0)
	 {
	 	if(gcu == NULL) {
    		gcu = pci_get_device(PCI_VENDOR_ID_INTEL, GCU_DEVID, NULL);
		   pci_read_config_dword(gcu, 0x00, &vdid);
		}

	 	if(gcu) {
			gcu_iegbe_resume(gcu);
	 		gcu_resume = 1;
	 		gcu_suspend = 0;
		} else {
			printk("Unable to resume GCU!\n");
		}
	 }
    pci_set_power_state(pdev, 0x0);
	pci_restore_state(pdev);
	ret_val = pci_enable_device(pdev);
	pci_set_master(pdev);

    pci_enable_wake(pdev, 0x3, 0x0);
    pci_enable_wake(pdev, 0x4, 0x0); /* 4 == D3 cold */

	iegbe_reset(adapter);
	E1000_WRITE_REG(&adapter->hw, WUS, ~0);
    offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_ST)
                 + PCI_ST_SMIA_OFFSET;
    pci_write_config_dword(adapter->pdev, offset, 0x00000006);
    E1000_WRITE_REG(&adapter->hw, IMC1, ~0UL);
    E1000_WRITE_REG(&adapter->hw, IMC2, ~0UL);

    if(netif_running(netdev)) {
		iegbe_up(adapter);
    }
	netif_device_attach(netdev);

	if(adapter->hw.mac_type >= iegbe_82540
	   && adapter->hw.mac_type != iegbe_icp_xxxx
	   && adapter->hw.media_type == iegbe_media_type_copper) {
		manc = E1000_READ_REG(&adapter->hw, MANC);
		manc &= ~(E1000_MANC_ARP_EN);
		E1000_WRITE_REG(&adapter->hw, MANC, manc);
	}

	switch(adapter->hw.mac_type) {
	case iegbe_82571:
	case iegbe_82572:
		ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
		E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
				ctrl_ext | E1000_CTRL_EXT_DRV_LOAD);
		break;
	case iegbe_82573:
		swsm = E1000_READ_REG(&adapter->hw, SWSM);
		E1000_WRITE_REG(&adapter->hw, SWSM,
				swsm | E1000_SWSM_DRV_LOAD);
		break;
	default:
		break;
	}

	return 0;
}
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void
iegbe_netpoll(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	disable_irq(adapter->pdev->irq);
#ifdef CONFIG_IRQ_PASS_REGS
	iegbe_intr(adapter->pdev->irq, netdev, NULL);
#else
	iegbe_intr(adapter->pdev->irq, netdev);
#endif
	enable_irq(adapter->pdev->irq);
}
#endif

/* iegbe_main.c */
