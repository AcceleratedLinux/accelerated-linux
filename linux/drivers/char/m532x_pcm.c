/***************************************************************************
 *   Copyright (c) 2008 Arcturus Networks Inc.
 *                 by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 *   This code is based on drivers/char/m5329_pcm.c
 *   Copyright (c) 2006 Arcturus Networks Inc.
 *                 by MaTed <www.ArcturusNetworks.com>
 *
 ***************************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/fcntl.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/time.h>

#include <asm/uaccess.h>
#include <asm/m532xsim.h>

#define M532X_PCM_MAJOR		120
#define M532X_PCM_NAME		"VoIP"
#define M532X_PCM_VERSION	"v.2.2 (c) 2008 Arcturus Networks Inc."

#define MAX_DEV_NUM	 1
#define MAX_TCD_NUM	 8
#define MAX_PTIME	 3
#define MAX_FRNUM	 3
#define MIN_FRSIZE	160	/* 80 is MIN if 8KHz, 160 if 16KHz */

#define FLAG_DEV_OPENED	0x8000

#define	M532X_PCM_RX_IRQ_LEVEL	4	/* Rx DMA Interrupt Priority */
#define	M532X_PCM_TX_IRQ_LEVEL	4	/* Tx DMA Interrupt Priority */

/******************************** Rx/Read  ***/
#define DMA11_IRQ_SOURCE 19
#define DMA11_IRQ_NUMBER (19+64)
#define DMA11_TX_CHAN	11

/******************************** Tx/Write ***/
#define DMA9_IRQ_SOURCE 17
#define DMA9_IRQ_NUMBER (17+64)
#define DMA9_RX_CHAN	9

/******************************* IOCTL ***/
#define M532X_PCM_IOC_MAGIC  	'Z'
#define M532X_PCM_IOC_MAXNR  	26
#define M532X_PCM_IOC_TXBUFF	1

#define M532X_PCM_IOCEXIT	_IO (M532X_PCM_IOC_MAGIC, 0)	/* shutdown pcm device */
#define M532X_PCM_IOCRESET	_IO (M532X_PCM_IOC_MAGIC, 1)	/* reset pcm */

#define M532X_PCM_IOCGCODEC	_IOR(M532X_PCM_IOC_MAGIC, 2, int)	/* get curent codec */
#define M532X_PCM_IOCSCODEC	_IOW(M532X_PCM_IOC_MAGIC, 3, int)	/* set curent codec */
#define M532X_PCM_IOCGCODEC_EF	_IOR(M532X_PCM_IOC_MAGIC, 4, int)	/* get codec encode entry */
#define M532X_PCM_IOCGCODEC_DF	_IOR(M532X_PCM_IOC_MAGIC, 5, int)	/* set codec decode entry */
#define M532X_PCM_IOCSCODEC_EF	_IOR(M532X_PCM_IOC_MAGIC, 6, int)	/* get codec encode entry */
#define M532X_PCM_IOCSCODEC_DF	_IOR(M532X_PCM_IOC_MAGIC, 7, int)	/* set codec decode entry */

#define M532X_PCM_IOCGAEC	_IOR(M532X_PCM_IOC_MAGIC, 8, int)	/* get curent AEC state */
#define M532X_PCM_IOCSAEC	_IOW(M532X_PCM_IOC_MAGIC, 9, int)	/* set curent AEC state */
#define M532X_PCM_IOCGAECF	_IOR(M532X_PCM_IOC_MAGIC, 10, int)	/* get AEC function entry */
#define M532X_PCM_IOCSAECF	_IOW(M532X_PCM_IOC_MAGIC, 11, int)	/* set AEC function entry */

#define M532X_PCM_IOCGNUMTCD	_IOR(M532X_PCM_IOC_MAGIC, 12, int)	/* get num_tcd */
#define M532X_PCM_IOCSNUMTCD	_IOW(M532X_PCM_IOC_MAGIC, 13, int)	/* set num_tcd */

#define M532X_PCM_IOCSPTIME	_IOW(M532X_PCM_IOC_MAGIC, 14, int)	/* set ptime */
#define M532X_PCM_IOCGPTIME	_IOR(M532X_PCM_IOC_MAGIC, 15, int)	/* get ptime */

#define M532X_PCM_IOCSTONE	_IOW(M532X_PCM_IOC_MAGIC, 16, int)	/* set local TONE_G state */
#define M532X_PCM_IOCGTONE	_IOR(M532X_PCM_IOC_MAGIC, 17, int)	/* get local TONE_G state */
#define M532X_PCM_IOCSTONEF	_IOW(M532X_PCM_IOC_MAGIC, 18, int)	/* set TONE_G function entry */
#define M532X_PCM_IOCSCALL	_IOW(M532X_PCM_IOC_MAGIC, 19, int)	/* magic call */
#define M532X_PCM_IOCGTONEF	_IOR(M532X_PCM_IOC_MAGIC, 20, int)	/* get TONE_G function entry */
#define M532X_PCM_IOCSTONEREMOTE _IOW(M532X_PCM_IOC_MAGIC, 21, int)	/* set remote TONE_G state */
#define M532X_PCM_IOCGTONEREMOTE _IOR(M532X_PCM_IOC_MAGIC, 22, int)	/* get remote TONE_G state */

#define M532X_PCM_IOCSAECOFF 	_IOW(M532X_PCM_IOC_MAGIC, 23, int)	/* set AEC tx buffer offset */

#define M532X_PCM_IOCSFREQ 	_IOW(M532X_PCM_IOC_MAGIC, 24, int)	/* set PCM Freq in KHz (8 or 16) */

#define M532X_PCM_IOCSCOMPLEX 	_IOW(M532X_PCM_IOC_MAGIC, 25, unsigned int)	/* set CODEC&FREQ&PTIME */

#define TONE_LOCAL_BIT_MAP	0x80000000
#define TONE_REMOTE_BIT_MAP	0x40000000


typedef struct set_codec_struct {
	unsigned int codec;
	unsigned int pcm2codec;
	unsigned int codec2pcm;
	unsigned short freq;
	unsigned short ptime;
} set_codec_t;

typedef u16 frame_t[MAX_FRNUM * MIN_FRSIZE];

/** DMA TCD Structure **/
typedef struct mcf_tcd {
	volatile u32 saddr;	/* source address */
	volatile u16 attr;	/* transfer attributes */
	volatile u16 soff;	/* signed source address offset */
	volatile u32 nbytes;	/* minor byte count */
	volatile u32 slast;	/* last source address adjustment */
	volatile u32 daddr;	/* destination address */
	volatile u16 citer;	/* current minor loop link, major loop count */
	volatile u16 doff;	/* signed destination address offset */
	volatile u32 dlast_sga;	/* last destination address adjustment */
	volatile u16 biter;	/* beginning minor loop link, major loop count */
	volatile u16 csr;	/* control and status */
} mcf_tcd_t;

/** Driver Private Structure **/
typedef struct m532x_pcm_dev {
	u32 flags;
	u32 frsize;
	u32 ptime;		/* 1 - 10ms | 2 - 20ms | 3 - 30ms */
	u32 num_tcd;
	u32 timeout;
	u32 tx_event;
	u32 rx_event;
	u32 tx_dma_indx;	/* current index for tx tcd for dma HW */
	u32 tx_dma_indx_prev;	/* prev index for tx tcd for dma HW */
	u32 tx_indx;		/* current index for write function */
	u32 rx_dma_indx;	/* current index for rx tcd for dma HW */
	u32 rx_dma_indx_prev;	/* prev index for tx tcd for dma HW */
	u32 rx_indx;		/* current index for read function     */

	u32 codec;		/* 0 - PCM, X - codec number */
	int (*codec2pcm) (u16 * pcm, u8 * codec, int ptime);
	int (*pcm2codec) (u8 * codec, u16 * pcm, int ptime);

	u32 aec;		/* 0 | 1 : AEC off | on  */
	u32 aec_indx;		/* 0, 1, 2 */
	int (*aec_run) (u16 * rin, u16 * sin, u16 * sout, int pfrsz);

	u32 conference;		/* 0 | 1 : 3-way conference off | on  */
	void (*mixer) (u16 * out, u16 * first, u16 * second, int p_frsz);

	u32 tone;		/* 0 - no, X - local tone number for play */
	u32 tone_remote;	/* 0 - no, X - remote tone number for transfer */
	int (*tone2pcm) (u16 * pcm, int tone, int ptime);

	wait_queue_head_t write_wait;
	wait_queue_head_t read_wait;

	spinlock_t tx_lock;
	spinlock_t rx_lock;

	mcf_tcd_t *mcf_tx_tcd;	/* bases of tx tcd (uncached)    */
	mcf_tcd_t *mcf_rx_tcd;	/* bases of rx tcd (uncached)    */

	frame_t *tx_buf;	/* bases of tx buffers (uncached)    */
	frame_t *rx_buf;	/* bases of rx buffers (uncached)    */
	frame_t *silence_buf;	/* base  of silence buffer (uncached)  */
	frame_t *aec_buf;	/* base  of AEC output buffer */
} m532x_pcm_dev_t;

static m532x_pcm_dev_t m532x_pcm_dev[MAX_DEV_NUM] __attribute__ ((aligned(32)));

static mcf_tcd_t _mcf_tx_tcd[MAX_TCD_NUM] __attribute__ ((aligned(32)));
static mcf_tcd_t _mcf_rx_tcd[MAX_TCD_NUM] __attribute__ ((aligned(32)));

static frame_t _pcm_tx_buf[MAX_TCD_NUM] __attribute__ ((aligned(32)));
static frame_t _pcm_rx_buf[MAX_TCD_NUM] __attribute__ ((aligned(32)));

static const frame_t _pcm_silence __attribute__ ((aligned(32)));
static const frame_t _aec_buf __attribute__ ((aligned(32)));

static u32 first_run = 1;

/******************************************************************************/
/*******************************************  DEV/HW settings   ***************/
/******************************************************************************/
static void m532x_pcm_dev_init(void)
{
	int i;
	printk(M532X_PCM_NAME ": " M532X_PCM_VERSION "\n");

	for (i = 0; i < MAX_DEV_NUM; i++) {
		m532x_pcm_dev[i].flags = 0;
		m532x_pcm_dev[i].frsize = 80;	/* 80 16bit words is default (8KHz) */
		m532x_pcm_dev[i].ptime = 2;	/* 20ms is default */
		m532x_pcm_dev[i].num_tcd = MAX_TCD_NUM;
		m532x_pcm_dev[i].timeout =
		    (m532x_pcm_dev[i].ptime * (m532x_pcm_dev[i].num_tcd - 2));
		m532x_pcm_dev[i].tx_event = 0;
		m532x_pcm_dev[i].rx_event = 0;
		m532x_pcm_dev[i].tx_dma_indx = 0;
		m532x_pcm_dev[i].tx_indx = 1;
		m532x_pcm_dev[i].rx_dma_indx = 0;
		m532x_pcm_dev[i].rx_indx = m532x_pcm_dev[i].num_tcd - 1;

		m532x_pcm_dev[i].codec = 0;
		m532x_pcm_dev[i].codec2pcm = NULL;
		m532x_pcm_dev[i].pcm2codec = NULL;
		m532x_pcm_dev[i].aec = 0;
		m532x_pcm_dev[i].aec_indx = 0;
		m532x_pcm_dev[i].aec_run = NULL;
		m532x_pcm_dev[i].conference = 0;
		m532x_pcm_dev[i].mixer = NULL;
		m532x_pcm_dev[i].tone = 0;
		m532x_pcm_dev[i].tone_remote = 0;
		m532x_pcm_dev[i].tone2pcm = NULL;

		m532x_pcm_dev[i].mcf_tx_tcd =
		    (mcf_tcd_t *) ((u32) _mcf_tx_tcd |
				   CONFIG_UNCACHED_REGION_MASK);
		m532x_pcm_dev[i].mcf_rx_tcd =
		    (mcf_tcd_t *) ((u32) _mcf_rx_tcd |
				   CONFIG_UNCACHED_REGION_MASK);

		m532x_pcm_dev[i].tx_buf =
		    (frame_t *) ((u32) _pcm_tx_buf |
				 CONFIG_UNCACHED_REGION_MASK);
		m532x_pcm_dev[i].rx_buf =
		    (frame_t *) ((u32) _pcm_rx_buf |
				 CONFIG_UNCACHED_REGION_MASK);
		m532x_pcm_dev[i].silence_buf = (frame_t *) (u32) _pcm_silence;
		m532x_pcm_dev[i].aec_buf = (frame_t *) ((u32) _aec_buf |
							CONFIG_UNCACHED_REGION_MASK);

		init_waitqueue_head(&m532x_pcm_dev[i].read_wait);
		init_waitqueue_head(&m532x_pcm_dev[i].write_wait);
	}
	return;
}

static void tcd_setup(m532x_pcm_dev_t * dev)
{
	int i;
	mcf_tcd_t *txp = dev->mcf_tx_tcd;
	mcf_tcd_t *rxp = dev->mcf_rx_tcd;

	/* stop DMA - just in case */
	MCF_EDMA_TCD9_CSR &= ~MCF_EDMA_TCD_CSR_START;
	MCF_EDMA_TCD11_CSR &= ~MCF_EDMA_TCD_CSR_START;
	mdelay(100);

	for (i = 0; i < dev->num_tcd; i++) {
		txp[i].saddr = (u32) dev->silence_buf;
		txp[i].daddr = ((u32) & MCF_SSI_TX0) + 2;
		txp[i].attr =
		    (MCF_EDMA_TCD_ATTR_SSIZE_16BIT |
		     MCF_EDMA_TCD_ATTR_DSIZE_16BIT);
		txp[i].soff = 2;
		txp[i].doff = 0;
		txp[i].nbytes = 2;
		txp[i].slast = 0;
		txp[i].biter = txp[i].citer = dev->ptime * dev->frsize;

		if (i == dev->num_tcd - 1)
			txp[i].dlast_sga = (u32) & txp[0];
		else
			txp[i].dlast_sga = (u32) & txp[i + 1];

		if (i == 0)
			txp[i].csr =
			    MCF_EDMA_TCD_CSR_E_SG | MCF_EDMA_TCD_CSR_INT_MAJOR;
		else
			txp[i].csr =
			    MCF_EDMA_TCD_CSR_E_SG | MCF_EDMA_TCD_CSR_INT_MAJOR |
			    MCF_EDMA_TCD_CSR_START;

		rxp[i].saddr = ((u32) & MCF_SSI_RX0) + 2;
		rxp[i].daddr = (u32) dev->rx_buf[i];
		rxp[i].attr =
		    (MCF_EDMA_TCD_ATTR_SSIZE_16BIT |
		     MCF_EDMA_TCD_ATTR_DSIZE_16BIT);
		rxp[i].doff = 2;
		rxp[i].soff = 0;
		rxp[i].nbytes = 2;
		rxp[i].slast = 0;
		rxp[i].biter = rxp[i].citer = dev->ptime * dev->frsize;
		if (i == dev->num_tcd - 1)
			rxp[i].dlast_sga = (u32) & rxp[0];
		else
			rxp[i].dlast_sga = (u32) & rxp[i + 1];

		if (i == 0)
			rxp[i].csr =
			    MCF_EDMA_TCD_CSR_E_SG | MCF_EDMA_TCD_CSR_INT_MAJOR;
		else
			rxp[i].csr =
			    MCF_EDMA_TCD_CSR_E_SG | MCF_EDMA_TCD_CSR_INT_MAJOR |
			    MCF_EDMA_TCD_CSR_START;
	}
	return;
}

static void start_dma(m532x_pcm_dev_t * dev)
{
	mcf_tcd_t *txp = dev->mcf_tx_tcd;
	mcf_tcd_t *rxp = dev->mcf_rx_tcd;

	memcpy((void *)MCF_EDMA_TCD11_SADDR_ADDR, &txp[0], sizeof(mcf_tcd_t));
	memcpy((void *)MCF_EDMA_TCD9_SADDR_ADDR, &rxp[0], sizeof(mcf_tcd_t));

	/* first TCD can now have start enabled */
	txp->csr |= MCF_EDMA_TCD_CSR_START;
	rxp->csr |= MCF_EDMA_TCD_CSR_START;

	/* 5. Enable any hardware service requests via the EDMA_ERQ. */
	MCF_EDMA_ERQ |= (MCF_EDMA_EEI_EEI9 | MCF_EDMA_EEI_EEI11);

	/* now we start DMA  */
	MCF_EDMA_TCD9_CSR |= MCF_EDMA_TCD_CSR_START;
	MCF_EDMA_TCD11_CSR |= MCF_EDMA_TCD_CSR_START;
}

static void m532x_hw_init(void)
{
	/* Issue a SSI reset */
	MCF_SSI_CR &= ~MCF_SSI_CR_SSI_EN;	/* disable SSI module */

	/* SSI module uses external CPU clock */
	MCF_CCM_MISCCR &= ~(MCF_CCM_MISCCR_SSI_SRC | MCF_CCM_MISCCR_TIM_DMA);

	/* Enable weak pullup (initial default) */
	MCF_CCM_MISCCR |= (MCF_CCM_MISCCR_SSI_PUS | MCF_CCM_MISCCR_SSI_PUE);

	/* Clock divider - enforce default values */
	MCF_CCM_CDR = MCF_CCM_CDR_SSIDIV(1) | MCF_CCM_CDR_LPDIV(0);

	/* Set Pin Assignment Register - set to SSI */
	/* Enable the SSI pins */
	MCF_GPIO_PAR_SSI =
	    (0 | MCF_GPIO_PAR_SSI_PAR_MCLK | MCF_GPIO_PAR_SSI_PAR_TXD(3)
	     | MCF_GPIO_PAR_SSI_PAR_RXD(3)
	     | MCF_GPIO_PAR_SSI_PAR_FS(3) | MCF_GPIO_PAR_SSI_PAR_BCLK(3));

	MCF_SSI_CR = MCF_SSI_CR_SYN	/* Enable synchronous mode */
	    | MCF_SSI_CR_SSI_EN;	/* Enable SSI mode */

	MCF_SSI_TCR = MCF_SSI_TCR_TFEN0 |	/* TX FIFO 0 enabled */
	    MCF_SSI_TCR_TFSL |	/* frame sync is 1 bit long */
	    MCF_SSI_TCR_TXBIT0 |	/* shifting w respect to bit 0 */
	    MCF_SSI_TCR_TEFS;	/* TX frame sync 1 bit before data */

	MCF_SSI_RCR = MCF_SSI_RCR_RFEN0 |	/* RX FIFO 0 enabled */
	    MCF_SSI_RCR_RFSL |	/* frame sync is 1 bit long */
	    MCF_SSI_RCR_RXBIT0 |	/* shifting w respect to bit 0 */
	    MCF_SSI_RCR_REFS;	/* RX frame sync 1 bit before data */

	MCF_SSI_CCR = MCF_SSI_CCR_WL(7);	/* 16 bit word length */
	MCF_SSI_FCSR &= ~(MCF_SSI_FCSR_TFWM0(0xf) | MCF_SSI_FCSR_RFWM0(0xf));
	MCF_SSI_FCSR |= MCF_SSI_FCSR_TFWM0(0x4) | MCF_SSI_FCSR_RFWM0(0x4);

	MCF_SSI_IER = MCF_SSI_IER_RDMAE |	/* Enable Rx DMA */
	    MCF_SSI_IER_TDMAE;	/* Enable Tx DMA */

	/* DMA interrupt */
	MCF_INTC0_ICR(DMA11_IRQ_SOURCE) = M532X_PCM_TX_IRQ_LEVEL;
	MCF_INTC0_ICR(DMA9_IRQ_SOURCE) = M532X_PCM_RX_IRQ_LEVEL;

	/* Clear interrupts */
	MCF_INTC0_CIMR = DMA11_IRQ_SOURCE;
	MCF_INTC0_CIMR = DMA9_IRQ_SOURCE;

	/* enable the SSI tx and rx */
	MCF_SSI_CR |= MCF_SSI_CR_SSI_EN |	/* enable SSI module */
	    MCF_SSI_CR_TE |	/* Enable transmitter */
	    MCF_SSI_CR_RE;	/* Enable receiver */
}

/******************************************************************************/
/**************************************************   DMA IRQs   **************/
/******************************************************************************/
static irqreturn_t m532x_pcm_tx_dmaisr(int irq, void *dev_id)
{
	m532x_pcm_dev_t *dev = (m532x_pcm_dev_t *) dev_id;
	mcf_tcd_t *txp = dev->mcf_tx_tcd;

	/* Clear eDMA interrupt */
	MCF_EDMA_CINT = DMA11_TX_CHAN;

	/* set to slience buffer */
	txp[dev->tx_dma_indx & (dev->num_tcd - 1)].saddr =
	    (u32) dev->silence_buf;

	/* Increase TX DMA index */
	dev->tx_dma_indx++;
	
	if ((dev->tx_dma_indx & (dev->num_tcd - 1)) == 
	   ((dev->tx_indx -1) & (dev->num_tcd - 1)))
		dev->tx_indx++;

	if (waitqueue_active(&dev->write_wait))
		wake_up_interruptible(&dev->write_wait);

	return IRQ_HANDLED;
}

static irqreturn_t m532x_pcm_rx_dmaisr(int irq, void *dev_id)
{
	m532x_pcm_dev_t *dev = (m532x_pcm_dev_t *) dev_id;

	/* Clear eDMA interrupt */
	MCF_EDMA_CINT = DMA9_RX_CHAN;

	/* Increase RX DMA index */
	dev->rx_dma_indx++;
	
	if ((dev->rx_dma_indx & (dev->num_tcd - 1)) == 
	   ((dev->rx_indx -1) & (dev->num_tcd - 1)))
		dev->rx_indx++;

	if (waitqueue_active(&dev->read_wait))
		wake_up_interruptible(&dev->read_wait);

	return IRQ_HANDLED;
}

#if defined(CONFIG_SENSORS_LM4931)

#define LM493X_REG_BASIC_CONFIG		0x00
#define LM493X_REG_CLK_MUX		0x08
extern u8 lm493x_read_direct(u8);
extern void lm493x_write_direct(u8, u16);

static void codec_reset(int freq)
{

	u16 tmp = (u16) lm493x_read_direct(LM493X_REG_BASIC_CONFIG);

	lm493x_write_direct(LM493X_REG_BASIC_CONFIG, 0x10);	/* RESET */
	if (freq == 80)
		lm493x_write_direct(LM493X_REG_CLK_MUX, 0x00);	/*  8 KHz */
	else if (arg == 160)
		lm493x_write_direct(LM493X_REG_CLK_MUX, 0x40);	/*  16 KHz */
	lm493x_write_direct(LM493X_REG_BASIC_CONFIG, tmp);
}

#else

#define codec_reset(F)

#endif

/******************************************************************************/
/**********************************************   Driver  FOPS   **************/
/******************************************************************************/
static void m532x_pcm_exit(void);
static int
m532x_pcm_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		unsigned long arg)
{
	m532x_pcm_dev_t *dev = (m532x_pcm_dev_t *) file->private_data;
	unsigned int ret = 0;
	unsigned int tmp = 0;
	unsigned int stat = 0;
	unsigned long flags;

	if (_IOC_TYPE(cmd) != M532X_PCM_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > M532X_PCM_IOC_MAXNR)
		return -ENOTTY;

	local_irq_save(flags);

	switch (cmd) {

	case M532X_PCM_IOCEXIT:
		m532x_pcm_exit();
		break;

	case M532X_PCM_IOCRESET:
		first_run = 1;
		break;

	case M532X_PCM_IOCGCODEC:
		ret = dev->codec;
		break;

	case M532X_PCM_IOCSCODEC:
		dev->codec = arg;
		break;

	case M532X_PCM_IOCGCODEC_EF:
		ret = (unsigned int)dev->pcm2codec;

		break;

	case M532X_PCM_IOCGCODEC_DF:
		ret = (unsigned int)dev->codec2pcm;
		break;

	case M532X_PCM_IOCSCODEC_EF:
		dev->pcm2codec = (void *)arg;
		break;

	case M532X_PCM_IOCSCODEC_DF:
		dev->codec2pcm = (void *)arg;
		break;

	case M532X_PCM_IOCGAEC:
		ret = dev->aec;
		break;

	case M532X_PCM_IOCSAEC:
		dev->aec = arg;
		break;

	case M532X_PCM_IOCSCALL:
	  {
		struct call_struct {
		  int (*call_routine)(void *);
		} * call_parm;

		call_parm = (struct call_struct *) arg;
		ret = call_parm->call_routine( (void *) arg);
		break;
	  }

	case M532X_PCM_IOCGAECF:
		ret = (unsigned int)dev->aec_run;
		break;

	case M532X_PCM_IOCSAECF:
		dev->aec_run = (void *)arg;
		break;

	case M532X_PCM_IOCGTONE:
		ret = dev->tone;
		break;

	case M532X_PCM_IOCGTONEREMOTE:
		ret = dev->tone_remote;
		break;

	case M532X_PCM_IOCSTONE:
		dev->tone = arg;
		break;

	case M532X_PCM_IOCSTONEREMOTE:
		dev->tone_remote = arg;
		break;

	case M532X_PCM_IOCGTONEF:
		ret = (unsigned int)dev->tone2pcm;
		break;

	case M532X_PCM_IOCSTONEF:
		dev->tone2pcm = (void *)arg;
		break;

	case M532X_PCM_IOCGNUMTCD:
		ret = dev->num_tcd;
		break;

	case M532X_PCM_IOCSNUMTCD:
		if (arg != dev->num_tcd) {
			if (arg == 4 || arg == 8 || arg == 16) {
				dev->num_tcd = arg;
				dev->timeout =
				    ((dev->num_tcd - 1) * dev->ptime);
				tcd_setup(dev);
				dev->tx_dma_indx = 0;
				dev->tx_indx = 3;
				dev->rx_dma_indx = 0;
				dev->rx_indx = 3;
				start_dma(dev);
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case M532X_PCM_IOCGPTIME:
		ret = dev->ptime;
		break;

	case M532X_PCM_IOCSPTIME:
		tmp = arg & 0xFF;
		if (dev->ptime != tmp) {
			if (tmp > 0 && tmp <= MAX_PTIME) {
				dev->ptime = tmp;
				dev->timeout =
				    ((dev->num_tcd - 2) * dev->ptime);
				tcd_setup(dev);
				dev->tx_dma_indx = 0;
				dev->tx_indx = 3;
				dev->rx_dma_indx = 0;
				dev->rx_indx = 3;
				start_dma(dev);
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case M532X_PCM_IOCSAECOFF:
		dev->aec_indx = arg;
		break;

	case M532X_PCM_IOCSFREQ:
		tmp = (arg & 0xFF) * 10;
		if (dev->frsize != tmp) {
			if (tmp == 80 || tmp == 160) {
				dev->frsize = tmp;
				tcd_setup(dev);
				dev->tx_dma_indx = 0;
				dev->tx_indx = 3;
				dev->rx_dma_indx = 0;
				dev->rx_indx = 3;
				codec_reset(tmp);
				start_dma(dev);
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case M532X_PCM_IOCSCOMPLEX:
		{
			set_codec_t *scodecdata;

			scodecdata = (set_codec_t *) arg;

			tmp = scodecdata->ptime & 0xFF;
			if (dev->ptime != tmp) {
				if (tmp > 0 && tmp <= MAX_PTIME) {
					stat |= 1;
					dev->ptime = tmp;
					dev->timeout =
					    ((dev->num_tcd - 2) * dev->ptime);
				} else {
					ret = -EINVAL;
					break;
				}
			}

			tmp = (scodecdata->freq & 0xFF) * 10;
			if (dev->frsize != tmp) {
				if (tmp == 80 || tmp == 160) {
					stat |= 2;
					dev->frsize = tmp;
					codec_reset(tmp);
				} else {
					ret = -EINVAL;
					break;
				}
			}

			if (scodecdata->codec & TONE_LOCAL_BIT_MAP) {
				dev->tone = scodecdata->codec & 0x3FFFFFF;
				dev->tone2pcm = (void *)scodecdata->codec2pcm;
			}

			else if (scodecdata->codec & TONE_REMOTE_BIT_MAP) {
				dev->tone_remote =
				    scodecdata->codec & 0x3FFFFFF;
				dev->tone2pcm = (void *)scodecdata->codec2pcm;

			} else if (scodecdata->codec != dev->codec) {
				dev->codec = scodecdata->codec;
				dev->pcm2codec = (void *)scodecdata->pcm2codec;
				dev->codec2pcm = (void *)scodecdata->codec2pcm;
			}

			if (stat & 3) {
				tcd_setup(dev);
				dev->tx_dma_indx = 0;
				dev->tx_indx = 3;
				dev->rx_dma_indx = 0;
				dev->rx_indx = 3;
				start_dma(dev);
			}
			break;
		}
	default:
		printk("%s error: Non exist or obsolete IOCTL call #%d=%x\n",
		       __FILE__, cmd, cmd);
		ret = -ENOSYS;
	}
	local_irq_restore(flags);

	return ret;
}

static ssize_t
m532x_pcm_read(struct file *file, char *buf, size_t count, loff_t * offset)
{
	m532x_pcm_dev_t *dev = (m532x_pcm_dev_t *) file->private_data;
	unsigned long flags;
	ssize_t ret = -EAGAIN;

	/****************************** simple jitter buffer implementation ***/
	volatile unsigned int dma_indx_tmp;
	int dma_indx_diff;
	int lrx_dma_indx;
	int lrx_indx;
	int ltx_indx;

	dma_indx_tmp = dev->rx_dma_indx;
	dma_indx_diff = dma_indx_tmp - dev->rx_dma_indx_prev;
	dev->rx_dma_indx_prev = dma_indx_tmp;

	lrx_dma_indx = (dma_indx_tmp & (dev->num_tcd - 1));
	lrx_indx = (dev->rx_indx & (dev->num_tcd - 1));
	ltx_indx = ((dev->tx_indx - dev->aec_indx) & (dev->num_tcd - 1));

	if (dma_indx_diff >= (dev->num_tcd-1)
	    || ((dma_indx_tmp - dev->rx_indx) >= (dev->num_tcd-1))) {
		dev->rx_indx = (dma_indx_tmp - 2);
		lrx_indx = (dev->rx_indx & (dev->num_tcd - 1));
	}

	if ((dma_indx_tmp - dev->rx_indx) == 1) {
		ret = wait_event_interruptible_timeout(dev->read_wait,
						       ((dev->rx_indx+1) !=
						       dev->rx_dma_indx),
						       dev->timeout);
		if (ret == -ERESTARTSYS)
			return -ERESTARTSYS;
		if (ret == 0)
			return -ETIMEDOUT;
		else
			ret = count;
	}

	/********************************* aec / tone_generation / encoding ***/
	local_irq_save(flags);

	if (dev->tone_remote)
		dev->tone2pcm(dev->rx_buf[lrx_indx], dev->tone, dev->ptime);

	if (dev->aec != 0 && dev->tone_remote == 0) {
		if (dev->codec != 0) {
			dev->aec_run(dev->tx_buf[ltx_indx],
				     dev->rx_buf[lrx_indx],
				     (u16 *) dev->aec_buf,
				     (dev->frsize == 160) ? dev->ptime * 2 :
				     dev->ptime);
			dev->pcm2codec(buf, (u16 *) dev->aec_buf, dev->ptime);
		} else {
			dev->aec_run(dev->tx_buf[ltx_indx],
				     dev->rx_buf[lrx_indx],
				     (u16 *) buf,
				     (dev->frsize == 160) ? dev->ptime * 2 :
				     dev->ptime);
		}
	} else {
		if (dev->codec != 0) {
			dev->pcm2codec(buf, dev->rx_buf[lrx_indx], dev->ptime);
		} else {
			copy_to_user(buf, dev->rx_buf[lrx_indx], count);
		}
	}

	dev->rx_indx++;
	
	local_irq_restore(flags);

	return (ret);
}

static ssize_t
m532x_pcm_write(struct file *file, const char *buf, size_t cnt, loff_t * offset)
{
	m532x_pcm_dev_t *dev = (m532x_pcm_dev_t *) file->private_data;
	mcf_tcd_t *txp = dev->mcf_tx_tcd;
	unsigned long flags;
	ssize_t ret = -EAGAIN;

	/****************************** simple jitter buffer implementation ***/
	volatile unsigned int dma_indx_tmp;
	int dma_indx_diff;
	int ltx_dma_indx;
	int ltx_indx;

	dma_indx_tmp = dev->tx_dma_indx;
	dma_indx_diff = dma_indx_tmp - dev->tx_dma_indx_prev;
	dev->tx_dma_indx_prev = dma_indx_tmp;

	ltx_dma_indx = (dma_indx_tmp & (dev->num_tcd - 1));
	ltx_indx = (dev->tx_indx & (dev->num_tcd - 1));

	if (dma_indx_diff >= (dev->num_tcd - 1 )
	    || ((dma_indx_tmp - dev->tx_indx) >= (dev->num_tcd - 1))) {
		dev->tx_indx = (dma_indx_tmp - 2);
		ltx_indx = (dev->tx_indx & (dev->num_tcd - 1));
	}

	if ((dma_indx_tmp - dev->tx_indx) == 1) {
		ret = wait_event_interruptible_timeout(dev->write_wait,
						       ((dev->tx_indx + 1) !=
							dev->tx_dma_indx),
						       dev->timeout);
		if (ret == -ERESTARTSYS)
			return -ERESTARTSYS;
		if (ret == 0)
			return -ETIMEDOUT;
		else
			ret = cnt;
	}

	/*************************************** tone_generation / decoding ***/
	local_irq_save(flags);
	
	if (dev->tone != 0) {
		dev->tone2pcm(dev->tx_buf[ltx_indx], dev->tone, dev->ptime);
	} else {
		if (dev->codec != 0) {
			dev->codec2pcm(dev->tx_buf[ltx_indx], (char *)buf,
				       dev->ptime);
		} else {
			copy_from_user(dev->tx_buf[ltx_indx], buf, cnt);
		}
	}
	txp[ltx_indx].saddr = (u32) & dev->tx_buf[ltx_indx];

	dev->tx_indx++;
	
	local_irq_restore(flags);

	return (ret);
}

static int m532x_pcm_open(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	m532x_pcm_dev_t *dev;

	if ((minor >= MAX_DEV_NUM)) {
		printk("%s: %s: Trying to open invalid PCM number [%d]\n",
		       __FILE__, __FUNCTION__, minor);
		return -ENODEV;
	}

	file->private_data = &m532x_pcm_dev[minor];

	dev = (m532x_pcm_dev_t *) file->private_data;

	if (first_run) {
#if defined(CONFIG_SENSORS_LM4931)
		extern void lm493x_setup_direct(void);
		lm493x_setup_direct();
#endif
		first_run = 0;
		tcd_setup(&m532x_pcm_dev[0]);
		m532x_hw_init();
		start_dma(&m532x_pcm_dev[0]);
	}

	return 0;
}

static struct file_operations m532x_pcm_fops = {
	.open = m532x_pcm_open,
	.ioctl = m532x_pcm_ioctl,
	.read = m532x_pcm_read,
	.write = m532x_pcm_write,
};

static int m532x_pcm_init(void)
{
	printk(KERN_DEBUG "Module m532x_pcm init\n");

	m532x_pcm_dev_init();

	if (request_irq(DMA11_IRQ_NUMBER, &m532x_pcm_tx_dmaisr,
			0, "PCM_TX", (void *)m532x_pcm_dev))
		return -1;

	if (request_irq(DMA9_IRQ_NUMBER, &m532x_pcm_rx_dmaisr,
			0, "PCM_RX", (void *)m532x_pcm_dev)) {

		free_irq(DMA11_IRQ_NUMBER, &m532x_pcm_dev);
		return -1;
	}

	if (register_chrdev(M532X_PCM_MAJOR, M532X_PCM_NAME, &m532x_pcm_fops)) {
		printk(M532X_PCM_NAME ": unable to get major %d\n",
		       M532X_PCM_MAJOR);

		free_irq(DMA9_IRQ_NUMBER, &m532x_pcm_dev);
		free_irq(DMA11_IRQ_NUMBER, &m532x_pcm_dev);
		return -EIO;
	}
	return 0;
}

static void m532x_pcm_exit(void)
{
	printk(KERN_DEBUG "Module m532x_pcm exit\n");

	/* stop DMA 
	   6. Request channel service by either software 
	   (setting the TCDn_CSR[START] bit) or by hardware
	   (slave device asserting its eDMA peripheral request signal).
	 */

	MCF_EDMA_TCD9_CSR &= ~MCF_EDMA_TCD_CSR_START;
	MCF_EDMA_TCD11_CSR &= ~MCF_EDMA_TCD_CSR_START;

	free_irq(DMA9_IRQ_NUMBER, &m532x_pcm_dev);
	free_irq(DMA11_IRQ_NUMBER, &m532x_pcm_dev);

	unregister_chrdev(M532X_PCM_MAJOR, M532X_PCM_NAME);

	printk(M532X_PCM_NAME "  " M532X_PCM_VERSION "  unloaded\n");

	return;
}

module_init(m532x_pcm_init);
module_exit(m532x_pcm_exit);

MODULE_DESCRIPTION("Voice (SSI/DMA) driver for the uC53281EVM Module");
MODULE_AUTHOR("Oleksandr Zhadan (www.ArcturusNetworks.com)");
MODULE_LICENSE("GPL");
