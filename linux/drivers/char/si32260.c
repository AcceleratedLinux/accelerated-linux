/*
 *  Si32260.c - Silicon Labs SLIC
 *
 *  Copyright (c) 2013-2015, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/fiq.h>

#include "../../arch/arm/mach-imx/common.h"
#include "../../arch/arm/mach-imx/hardware.h"
#include "../../arch/arm/mach-imx/iomux-v3.h"

struct snd_pcm;
struct snd_pcm_substream;
struct snd_soc_pcm_runtime;
#include "../../sound/soc/fsl/imx-ssi.h"
#include "../../sound/soc/fsl/imx-audmux.h"
#include <linux/platform_data/asoc-imx-ssi.h>

/*
 * The PCM/audio interface of the SI32260 SLIC on the Accelerated Concepts
 * Dial Capture board is connected to an internal SSI port of the iMX253 via
 * its internal AUDIO mux module. The control interface is via an SPI port
 * of the iMX253.
 *
 * This driver controls both parts of that si32260 connection. We don't really
 * need to tap off the audio stream to user space - we only need to deliver it
 * from one slic interface to the other. So this driver ends up being a quirky
 * mix of hardware driving components. It is very specific to the connection
 * of the si32260 we use on the iMX253.
 *
 * It probably doesn't make sense to use the kernels audio subsystem for the
 * PCM side of the driver. For one thing its SSI driving code is setup for
 * I2S/AC97, and not the TDM method we use. And it is all very heavy weight
 * for what we want.
 *
 * To minimize latency from the servicing of the SSI/PCM interface we use an
 * ARM fast interrupt. This means we should never overrun/underrun on the PCM
 * data interface.
 */
#undef DRV_NAME
#define DRV_NAME	"si32260"

static void __iomem *ssip;
static void __iomem *audmuxp;
static int ssi_irq;

/*
 * As a debug aid I have left some conditional code here which allows reading
 * and writing the PCM audio stream to/from user space. It is pretty simple,
 * but very effective. Note that with this enabled you do not get the audio
 * loop from one port to the other - only the audio you play/capture.
 */
/*#define	DEBUGPCM*/

#ifdef DEBUGPCM
/*
 * Fixed size buffers to store RX FIFO, TX FIFO and status PCM data.
 * Organize them to be buffer size aligned to simplify FIQ code.
 */
#define	BUFSIZE		0x10000

static u8 bigbuf[4*BUFSIZE];
static u8 *rxbuf, *txbuf, *tracebuf;

static void si32260_fiq_init(void)
{
	struct pt_regs regs;

	rxbuf = (u8 *) (((unsigned int) &bigbuf[BUFSIZE]) & 0xffff0000);
	txbuf = rxbuf + BUFSIZE;
	tracebuf = txbuf + BUFSIZE;
	memset(&regs, 0, sizeof(regs));
	regs.ARM_r8 = (unsigned int) ssip;
	regs.ARM_r9 = (unsigned int) rxbuf;
	set_fiq_regs(&regs);
}

#else /* DEBUGPCM */

static void si32260_fiq_init(void)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));
	regs.ARM_r8 = (unsigned int) ssip;
	set_fiq_regs(&regs);
}

#endif /* !DEBUGPCM */

/*
 * Start the FIQ. First we want to adjust the FIFO levels to allow some
 * TX and RX slop - that is so we don't underrun the TX or overrun the RX.
 */
static void si32260_fiq_prime_and_start(void)
{
	unsigned long flags;
	unsigned int sfcsr, rxcnt, i;

	local_irq_save(flags);
	sfcsr = readl(ssip + SSI_SFCSR);
	rxcnt = (sfcsr >> SSI_RX_FIFO_0_COUNT_SHIFT) & 0xf;
	for (; rxcnt > 12; rxcnt--)
		readl(ssip + SSI_SRX0);
	for (i = 0; i < 4; i++)
		writel(0xffff0000, ssip + SSI_STX0);
	enable_fiq(ssi_irq);
	local_irq_restore(flags);
}

extern unsigned char si32260_fiq_start;
extern unsigned char si32260_fiq_end;

#ifdef DEBUGPCM
/*
 * FIQ registers:
 * r8  - SSI base pointer
 * r9  - RX buf pointer (deduce TX and status buffer from this)
 * r10 - buf offset counter
 */
void si32260_cfiq(void)
{
	asm("						\n\
                .globl si32260_fiq_start		\n\
                .globl si32260_fiq_end			\n\
							\n\
	si32260_fiq_start:				\n\
		ldr	r12, [r8, #0x2c]		\n\
		add	r9, r9, #0x20000		\n\
		str	r12, [r9, r10]			\n\
		sub	r9, r9, #0x20000		\n\
							\n\
		mov	r11, #8				\n\
							\n\
	1:	ldr	r12, [r8, #0x8]			\n\
		str	r12, [r9, r10]			\n\
		add	r9, r9, #0x10000		\n\
		ldr	r12, [r9, r10]			\n\
		sub	r9, r9, #0x10000		\n\
		str	r12, [r8, #0x0]			\n\
							\n\
		add	r10, r10, #4			\n\
		and	r10, r10, #0xff00ffff		\n\
							\n\
		subs	r11, #1				\n\
		bne	1b				\n\
							\n\
		subs	pc, lr, #4			\n\
	si32260_fiq_end:				\n\
	");
}
#else /* DEBUGPCM */
/*
 * FIQ registers:
 * r8  - SSI base pointer (set before we get here)
 * r9  - local loop counter
 * r10 - PCM data
 */
void si32260_cfiq(void)
{
	asm("						\n\
                .globl si32260_fiq_start		\n\
                .globl si32260_fiq_end			\n\
							\n\
	si32260_fiq_start:				\n\
		mov	r9, #8				\n\
	1:	ldr	r10, [r8, #0x8]			\n\
		str	r10, [r8, #0x0]			\n\
		subs	r9, #1				\n\
		bne	1b				\n\
		subs	pc, lr, #4			\n\
	si32260_fiq_end:				\n\
	");
}
#endif /* !DEBUGPCM */

static void si32260_ssi_intenable(void)
{
	writel(SSI_SIER_TIE | SSI_SIER_TFE0_EN, ssip + SSI_SIER);
}

static void si32260_ssi_intdisable(void)
{
	writel(0, ssip + SSI_SIER);
}

static struct fiq_handler si32260_fh = {
	.name	= "si32260",
};

/*
 * Missing definition of the PHCONFIG register.
 */
#define	SSI_PHCONFIG	0x5c

static int si32260_ssi_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct clk *clkp;
	int rc;

	clkp = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clkp)) {
		printk(KERN_ERR DRV_NAME ": failed to get imx-ssi clk\n");
		clkp = NULL;
	} else {
		clk_prepare_enable(clkp);
	}

	clkp = devm_clk_get(&pdev->dev, "per");
	if (IS_ERR(clkp)) {
		printk(KERN_ERR DRV_NAME ": failed to get imx-ssi per clk\n");
		clkp = NULL;
	} else {
		clk_prepare_enable(clkp);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ssip = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ssip)) {
		printk(KERN_ERR DRV_NAME ": failed to ioremap(SSI)\n");
                return PTR_ERR(ssip);
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	ssi_irq = res->start;

	writel(0x00000300, ssip + SSI_PHCONFIG);

	/* Use first data slot for our PCM data (assume 16bit word) */
	writel(0xfffffffe, ssip + SSI_STMSK);
	writel(0xfffffffe, ssip + SSI_SRMSK);

	/* Set TX FIFO interrupt threashold to at least 12 empty words */
	writel(SSI_SFCSR_TFWM0(12), ssip + SSI_SFCSR);

	/* TX side is clock generator, RX is syncrhonized to that */
	writel(SSI_STCCR_PM(0x40) | SSI_STCCR_DC(0x3) | SSI_STCCR_WL(16),
		ssip + SSI_STCCR);
	writel(SSI_SRCCR_PM(0x40) | SSI_SRCCR_DC(0x3) | SSI_SRCCR_WL(16),
		ssip + SSI_SRCCR);

	writel(SSI_STCR_TFSL | SSI_STCR_TFEN0 | SSI_STCR_TFDIR | SSI_STCR_TXDIR,
		ssip + SSI_STCR);
	writel(SSI_SRCR_RFSL | SSI_SRCR_RFEN0, ssip + SSI_SRCR);

	/* Enable it and start the engine */
	writel(0, ssip + SSI_SOR);
	writel(SSI_SCR_SSIEN | SSI_SCR_TE | SSI_SCR_RE | SSI_SCR_NET |
		SSI_SCR_SYN, ssip + SSI_SCR);

	rc = claim_fiq(&si32260_fh);
	if (rc) {
		printk(KERN_ERR DRV_NAME ": failed to claim FIQ, rc=%d\n", rc);
		return -ENODEV;
	}

	si32260_fiq_init();
	mxc_set_irq_fiq(ssi_irq, 1);
	set_fiq_handler(&si32260_fiq_start, &si32260_fiq_end - &si32260_fiq_start);
	si32260_ssi_intenable();
	si32260_fiq_prime_and_start();

	return 0;
}

static int si32260_ssi_remove(struct platform_device *pdev)
{
	disable_fiq(ssi_irq);
	si32260_ssi_intdisable();
	mxc_set_irq_fiq(ssi_irq, 0) ;
	release_fiq(&si32260_fh);
	return 0;
}

static const struct of_device_id si32260_ssi_dt_ids[] = {
	{ .compatible = "si32260-ssi", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, si32260_ssi_dt_ids);

static struct platform_driver si32260_ssi_driver = {
	.driver = {
			.name = "si32260-ssi",
			.of_match_table = si32260_ssi_dt_ids,
		},
	.probe = si32260_ssi_probe,
	.remove = si32260_ssi_remove,
};
module_platform_driver(si32260_ssi_driver);

/*
 * We rely on the iMX AUDMUX hardware module to route the signals from the
 * internal SSI module to the external SoC pins.
 */
#define IMX_AUDMUX_PTCR(x)	((x) * 8)
#define IMX_AUDMUX_PDCR(x)	((x) * 8 + 4)

static int si32260_audmux_probe(struct platform_device *pdev)
{
	struct resource *res;

#if 0
	/* No longer needed with devicetree? */
	struct clk *clkp;
	clkp = devm_clk_get(&pdev->dev, "audmux");
	if (IS_ERR(clkp)) {
		printk(KERN_ERR DRV_NAME ": failed to get imx-audmux clk\n");
		clkp = NULL;
	} else {
		clk_prepare_enable(clkp);
	}
#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	audmuxp = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(audmuxp)) {
		printk(KERN_ERR DRV_NAME ": failed to ioremap(AUDMUX)\n");
                return PTR_ERR(ssip);
	}

	/* PORT1 is connected to SSI0 */
	writel(IMX_AUDMUX_V2_PTCR_SYN, audmuxp + IMX_AUDMUX_PTCR(0));
	writel(IMX_AUDMUX_V2_PDCR_RXDSEL(2), audmuxp + IMX_AUDMUX_PDCR(0));

	/* PORT3 is connected to AUD3 pins */
	writel(IMX_AUDMUX_V2_PTCR_SYN | IMX_AUDMUX_V2_PTCR_TFSDIR |
		IMX_AUDMUX_V2_PTCR_TCLKDIR, audmuxp + IMX_AUDMUX_PTCR(2));
	writel(0, audmuxp + IMX_AUDMUX_PDCR(2));

	return 0;
}

static int si32260_audmux_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id si32260_audmux_dt_ids[] = {
	{ .compatible = "si32260-audmux", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, si32260_audmux_dt_ids);

static struct platform_driver si32260_audmux_driver = {
	.driver = {
			.name = "si32260-audmux",
			.of_match_table = si32260_audmux_dt_ids,
		},
	.probe = si32260_audmux_probe,
	.remove = si32260_audmux_remove,
};
module_platform_driver(si32260_audmux_driver);

/*
 * The SPI control interface to the si32260 SLIC.
 */
static struct spi_device *si32260_spi;

static int si32260_spi_probe(struct spi_device *spi)
{
	si32260_spi = spi;
	return 0;
}

static int si32260_spi_remove(struct spi_device *spi)
{
	si32260_spi = NULL;
	return 0;
}

static const struct spi_device_id si32260_spi_ids[] = {
	{ "si32260", 0 },
	{ /* sentinel */ },
};

static struct spi_driver si32260_spi_driver = {
	.driver = {
		.name = "si32260",
		.owner = THIS_MODULE,
	},
	.id_table = si32260_spi_ids,
	.probe = si32260_spi_probe,
	.remove = si32260_spi_remove,
};
module_spi_driver(si32260_spi_driver);

/*
 * SI32260 control read and write support functions.
 */
#define	SI_BRDCST	0x80
#define	SI_READ		0x40
#define	SI_WRITE	0x00
#define	SI_REG		0x20
#define	SI_RAM		0x00

static u8 si32260_readbyte(u8 slicnr, u8 addr)
{
	u8 sbuf[2];
	u8 rbuf[1];

	sbuf[0] = SI_READ | SI_REG | (slicnr << 4);
	sbuf[1] = addr;
	spi_write_then_read(si32260_spi, &sbuf[0], 2, &rbuf[0], 1);

	return rbuf[0];
}

static void si32260_writebyte(u8 slicnr, u8 addr, u8 val)
{
	u8 sbuf[3];

	sbuf[0] = SI_WRITE | SI_REG | (slicnr << 4);
	sbuf[1] = addr;
	sbuf[2] = val;
	spi_write_then_read(si32260_spi, &sbuf[0], 3, NULL, 0);
}

/*
 * The standard character device interface...
 */
#define MAXBYTES	0x100

static loff_t si32260_llseek( struct file *fp, loff_t offset, int whence)
{
	switch (whence) {
	case 0:
		fp->f_pos = offset;
		break;
	case 1:
		fp->f_pos += offset;
		break;
	case 2:
		fp->f_pos = MAXBYTES - 1;
		break;
	default:
		return -EINVAL;
	}

	if (fp->f_pos >= MAXBYTES)
		fp->f_pos = MAXBYTES - 1;
	return fp->f_pos;
}

#ifdef DEBUGPCM
#define	DBUFSIZE	8192
static u8 dbuf[DBUFSIZE];

static unsigned char *si32260_getrxbuf(void)
{
	struct pt_regs regs;
	unsigned int starthalf, myhalf;

	get_fiq_regs(&regs);
	starthalf = (regs.ARM_r10 < BUFSIZE/2) ? 0 : 1;

	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(5);

		get_fiq_regs(&regs);
		myhalf = (regs.ARM_r10 < BUFSIZE/2) ? 0 : 1;
		if (starthalf != myhalf)
			break;
	}

	if (myhalf)
		return rxbuf;
	return rxbuf + BUFSIZE/2;
}

static unsigned char *si32260_gettxbuf(void)
{
	struct pt_regs regs;

	get_fiq_regs(&regs);
	if (regs.ARM_r10 < BUFSIZE/2)
		return txbuf + BUFSIZE/2;
	return txbuf;
}

static void si32260_waittxbuf(unsigned char *bp)
{
	struct pt_regs regs;
	unsigned int starthalf, myhalf;

	starthalf = (bp == txbuf) ? 0 : 1;

	for (;;) {
		get_fiq_regs(&regs);
		myhalf = (regs.ARM_r10 < BUFSIZE/2) ? 0 : 1;
		if (starthalf == myhalf)
			break;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(5);
	}
}
#endif /* DEBUGPCM */

static ssize_t si32260_read(struct file *fp, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned int minor = iminor(fp->f_path.dentry->d_inode);
	ssize_t t;
	u8 val;

#ifdef DEBUGPCM
	if ((minor == 2) || (minor == 3)) {
		unsigned int chan = minor - 2;
		u8 *startbuf, *bp;
		int i;

		if (count != DBUFSIZE)
			return -EINVAL;
		startbuf = si32260_getrxbuf();
		bp = startbuf + (chan + 2);
		for (i = 0; i < DBUFSIZE; i++) {
			dbuf[i] = *bp;
			bp += 4;
		}
		copy_to_user(buf, &dbuf[0], DBUFSIZE);
		return count;
	}
#endif /* DEBUGPCM */

	if (si32260_spi == NULL)
		return -ENODEV;
	if (minor > 1)
		return -ENODEV;
	if (*ppos >= MAXBYTES)
		return 0;
	if ((*ppos + count) > MAXBYTES)
		count = MAXBYTES - *ppos;

	for (t = 0; t < count; t++, *ppos += 1) {
		val = si32260_readbyte(minor, *ppos);
		put_user(val, buf++);
	}

	return t;
}

static ssize_t si32260_write(struct file *fp, const char __user *buf, size_t count, loff_t *ppos)
{
	unsigned int minor = iminor(fp->f_path.dentry->d_inode);
	ssize_t t;
	u8 val;

#ifdef DEBUGPCM
	if ((minor == 2) || (minor == 3)) {
		unsigned int chan = minor - 2;
		u8 *startbuf, *bp;
		int i;

		if (count != DBUFSIZE)
			return -EINVAL;
		copy_from_user(&dbuf[0], buf, DBUFSIZE);
		startbuf = si32260_gettxbuf();
		bp = startbuf + (chan + 2);
		for (i = 0; i < DBUFSIZE; i++) {
			*bp = dbuf[i];
			bp += 4;
		}
		si32260_waittxbuf(startbuf);
		return count;
	}
#endif /* DEBUGPCM */

	if (si32260_spi == NULL)
		return -ENODEV;
	if (minor > 1)
		return -ENODEV;
	if (*ppos >= MAXBYTES)
		return 0;
	if ((*ppos + count) > MAXBYTES)
		count = MAXBYTES - *ppos;

	for (t = 0; t < count; t++, *ppos += 1) {
		get_user(val, buf++);
		si32260_writebyte(minor, *ppos, val);
	}

	return t;
}

/* GPIO3 line 4 */
#define	GPIO_SLICRESET	IMX_GPIO_NR(3, 4)

static void si32260_reset_init(void)
{
	gpio_request(GPIO_SLICRESET, "SLIC RESET");
	gpio_direction_output(GPIO_SLICRESET, 1);
	mdelay(1);
        gpio_set_value(GPIO_SLICRESET, 0);
        mdelay(10);
        gpio_set_value(GPIO_SLICRESET, 1);
}

static void si32260_reset(int state)
{
	gpio_set_value(GPIO_SLICRESET, (state ? 0 : 1));
}

/*
 * The modem modem attached to the slic has some unusual reset
 * requirements. Take care of those here.
 */
#define	GPIO_MODEMIRQ	IMX_GPIO_NR(1, 22)
#define	GPIO_UART4DCD	IMX_GPIO_NR(1, 29)
#define	GPIO_UART4DTR	IMX_GPIO_NR(1, 30)
#define	GPIO_MODEMRESET	IMX_GPIO_NR(3, 15)
#define	GPIO_UART1DCD	IMX_GPIO_NR(2, 31)
#define	GPIO_SIMDETECT	IMX_GPIO_NR(3, 24)

#define MX25_PAD_KPP_ROW2__UART1_DCD    IOMUX_PAD(0x3a8, 0x1b0, 0x14, 0, 0, NO_PAD_CTRL)
#define MX25_PAD_KPP_ROW2__GPIO_2_31    IOMUX_PAD(0x3a8, 0x1b0, 0x15, 0, 0, NO_PAD_CTRL)

#define MX25_IOMUXC_BASE_ADDR	0x43fac000

static void si32260_modem_init(void)
{
	void __iomem *muxp;

	muxp = ioremap(MX25_IOMUXC_BASE_ADDR, SZ_16K);
	if (muxp == NULL) {
		printk(KERN_ERR DRV_NAME ": failed to ioremap(IOMUX)\n");
		return;
	}
	mxc_iomux_v3_init(muxp);

	gpio_request(GPIO_MODEMRESET, "MODEM RESET");
	gpio_direction_output(GPIO_MODEMRESET, 1);
	gpio_request(GPIO_MODEMIRQ, "MODEM IRQ");
	gpio_direction_output(GPIO_MODEMIRQ, 1);

	mxc_iomux_v3_setup_pad(MX25_PAD_KPP_ROW2__GPIO_2_31);
	gpio_request(GPIO_UART1DCD, "UART1 DCD");
	gpio_direction_output(GPIO_UART1DCD, 1);

	/*
	 * Reset sequence for the modem.
	 */
	mdelay(20);
	gpio_set_value(GPIO_MODEMRESET, 0);
	mdelay(500);
	gpio_set_value(GPIO_MODEMRESET, 1);
	mdelay(20);

	gpio_direction_input(GPIO_MODEMIRQ);

	gpio_direction_input(GPIO_UART1DCD);
	gpio_free(GPIO_UART1DCD);
	mxc_iomux_v3_setup_pad(MX25_PAD_KPP_ROW2__UART1_DCD);

	iounmap(muxp);
}

/*
 * Reset ioctl command.
 */
#define	SI_RESET	0x32260000

static long si32260_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case SI_RESET:
		si32260_reset(arg);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct file_operations si32260_fops = {
	.owner = THIS_MODULE,
	.llseek = si32260_llseek,
	.read = si32260_read,
	.write = si32260_write,
	.unlocked_ioctl = si32260_ioctl,
};

#define	SI32260_MAJOR	25

static int __init si32260_init(void)
{
	printk(KERN_INFO DRV_NAME ": SLIC driver for iMX253\n");
	if (register_chrdev(25, "si32260", &si32260_fops)) {
		printk(KERN_ERR DRV_NAME "failed to register chrdev major=%d\n", SI32260_MAJOR);
		return -ENODEV;
	}

	si32260_modem_init();
	si32260_reset_init();
	return 0;
}

static void si32260_exit(void)
{
}

late_initcall(si32260_init);
module_exit(si32260_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_DESCRIPTION("Driver for SI32260 SLIC attached to iMX253");
