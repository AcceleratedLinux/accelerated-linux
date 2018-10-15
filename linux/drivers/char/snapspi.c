/*****************************************************************************/

/*
 *	snapspi.c -- driver for SnapGear bit-banged SPI interface.
 *
 * 	(C) Copyright 2004-2009, Greg Ungerer <gerg@snapgear.com>
 */

/*****************************************************************************/

#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <mach/hardware.h>

/*****************************************************************************/

#define	SNAPSPI_MAXSIZE	0x01000000	/* 24bit address size */

#define	CMD_WRSR	0x01		/* SPI write status register command */
#define	CMD_WRITE	0x02		/* SPI write command */
#define	CMD_READ	0x03		/* SPI read command */
#define	CMD_WRDI	0x04		/* SPI write disable command */
#define	CMD_RDSR	0x05		/* SPI read status register command */
#define	CMD_WREN	0x06		/* SPI write enable command */
#define	CMD_EWSR	0x50		/* SPI enable write status register */
#define	CMD_CHIPERASE	0x60		/* SPI chip erase command */
#define	CMD_READID	0x90		/* SPI read devcie ID command */
#define	CMD_AAIWR	0xad		/* SPI auto increment writing */

/*****************************************************************************/

/*
 *	The SPI lines to the SST25VF032B flash are GPIO lines from the
 *	IXP4xx. (They were borrowed from LED lines on the SG560!)
 */
#define	SPI_CS	2
#define	SPI_SI	3
#define	SPI_SO	4
#define	SPI_CLK	5

#define	IN	IXP4XX_GPIO_IN
#define	OUT	IXP4XX_GPIO_OUT

/*****************************************************************************/

static void gpio_line_set_slow(u8 line, int value)
{
	gpio_line_set(line, value);
	udelay(1);
}

static void gpio_line_get_slow(u8 line, int *value)
{
	gpio_line_get(line, value);
	udelay(1);
}

/*****************************************************************************/

void snapspi_chipenable(void)
{
	gpio_line_set(SPI_SI, 0);
	gpio_line_set(SPI_CLK, 0);
	gpio_line_set(SPI_CS, 0);
}


void snapspi_chipdisable(void)
{
	gpio_line_set(SPI_CLK, 0);
	gpio_line_set(SPI_CS, 1);
	gpio_line_set(SPI_SI, 0);
	udelay(1);
}

/*****************************************************************************/

void snapspi_sendbyte(unsigned int val)
{
	int i;

	for (i = 7; (i >= 0); i--) {
		gpio_line_set_slow(SPI_CLK, 0);
		gpio_line_set_slow(SPI_SI, ((val >> i) & 0x1));
		gpio_line_set_slow(SPI_CLK, 1);
	}
}

unsigned int snapspi_recvbyte(void)
{
	unsigned int val, bit;
	int i;

	for (i = 0, val = 0; (i < 8); i++) {
		gpio_line_set_slow(SPI_CLK, 0);
		gpio_line_get_slow(SPI_SO, &bit);
		val = (val << 1) | bit;
		gpio_line_set_slow(SPI_CLK, 1);
	}

	return val;
}

void snapspi_sendaddr(unsigned int addr)
{
	snapspi_sendbyte((addr >> 16) & 0xff);
	snapspi_sendbyte((addr >> 8) & 0xff);
	snapspi_sendbyte(addr & 0xff);
}
/*****************************************************************************/

u8 snapspi_rdsr(void)
{
	u8 status;
	snapspi_chipenable();
	snapspi_sendbyte(CMD_RDSR);
	status = snapspi_recvbyte();
	snapspi_chipdisable();
	return status;
}

/*****************************************************************************/

void snapspi_unlock(void)
{
	snapspi_chipenable();
	snapspi_sendbyte(CMD_EWSR);
	snapspi_chipdisable();
	snapspi_chipenable();
	snapspi_sendbyte(CMD_WRSR);
	snapspi_sendbyte(0);
	snapspi_chipdisable();
}

/*****************************************************************************/

void snapspi_erase(void)
{
	snapspi_chipenable();
	snapspi_sendbyte(CMD_WREN);
	snapspi_chipdisable();
	snapspi_chipenable();
	snapspi_sendbyte(CMD_CHIPERASE);
	snapspi_chipdisable();
	mdelay(50);
}

/*****************************************************************************/

void snapspi_setup(void)
{
	/* Set the GPIO lines as inputs or outputs as appropriate */
	gpio_line_config(SPI_CS, OUT);
	gpio_line_config(SPI_CLK, OUT);
	gpio_line_config(SPI_SI, OUT);
	gpio_line_config(SPI_SO, IN);

	/* Set SPI bus into idle mode */
	gpio_line_set(SPI_CS, 1);
	gpio_line_set(SPI_CLK, 0);
	gpio_line_set(SPI_SI, 0);
}

/*****************************************************************************/

static int snapspi_open(struct inode *inode, struct file *file)
{
	u8 man, id;

#if 0
	printk("snapspi_open(inode=%p,file=%p)\n", inode, file);
#endif

	snapspi_chipenable();
	snapspi_sendbyte(CMD_READID);
	snapspi_sendaddr(0);
	man = snapspi_recvbyte();
	id = snapspi_recvbyte();
	snapspi_chipdisable();

	printk("SNAPSPI: Device manufacturer=0x%02x ID=0x%02x\n", man, id);
	printk("SNAPSPI: Device status=0x%02x\n", snapspi_rdsr());

	return 0;
}

/*****************************************************************************/

static ssize_t snapspi_read(struct file *fp, char __user *buf, size_t count, loff_t *ptr)
{
	loff_t p = *ptr;
	int total;

#if 0
	printk("snapspi_read(buf=%p,count=%d)\n", buf, count);
#endif

	if (p >= SNAPSPI_MAXSIZE)
		return 0;

	if (count > (SNAPSPI_MAXSIZE - p))
		count = SNAPSPI_MAXSIZE - p;

	local_irq_disable();

	snapspi_chipenable();
	snapspi_sendbyte(CMD_READ);
	snapspi_sendaddr(p);
	for (total = 0; (total < count); total++)
		put_user(snapspi_recvbyte(), buf++);
	snapspi_chipdisable();

	local_irq_enable();

	*ptr += total;
	return total;
}

/*****************************************************************************/

static void snapspi_wait(void)
{
	int i;
	u8 status;

	for (i = 0; i < 1000000; i++) {
		status = snapspi_rdsr();
		if ((status & 0x1) == 0)
			return;
		udelay(1);
	}
	printk("SNAPSPI: timed out waiting?\n");
}

/*****************************************************************************/

static ssize_t snapspi_write(struct file *fp, const char __user *buf, size_t count, loff_t *ptr)
{
	unsigned int addr = (unsigned int) (*ptr);
	int total, len, i;
	char val;

#if 0
	printk("snapspi_write(buf=%p,count=%d)\n", buf, count);
#endif

	if (addr >= SNAPSPI_MAXSIZE)
		return 0;

	if (count > (SNAPSPI_MAXSIZE - addr))
		count = SNAPSPI_MAXSIZE - addr;

	snapspi_unlock();
	snapspi_erase();
	snapspi_wait();

	local_irq_disable();

	for (total = 0, len = 0; total < count; total += len) {
		snapspi_chipenable();
		snapspi_sendbyte(CMD_WREN);
		snapspi_chipdisable();

		len = 0x100 - (addr & 0xff);
		if (len > (count - total))
			len = count - total;

		snapspi_chipenable();
		snapspi_sendbyte(CMD_WRITE);
		snapspi_sendaddr(addr);
		for (i = 0; i < len; i++) {
			get_user(val, buf);
			buf++;
			snapspi_sendbyte(val);
		}
		snapspi_chipdisable();

		snapspi_wait();

		addr += len;
		printk("\b\b\b\b\b\b\b\b%dk", total >> 10);
	}
	printk("\n");

	/* Disable the write enable bit */
	snapspi_chipenable();
	snapspi_sendbyte(CMD_WRDI);
	snapspi_chipdisable();

	local_irq_enable();

	*ptr += total;
	return total;
}

/*****************************************************************************/

/*
 *	Exported file operations structure for driver...
 */
static struct file_operations snapspi_fops =
{
	.owner = THIS_MODULE, 
	.open = snapspi_open,
	.read =  snapspi_read,
	.write = snapspi_write,
};

static struct miscdevice snapspi_dev =
{
	RTC_MINOR,
	"spi",
	&snapspi_fops
};

/*****************************************************************************/

static int __init snapspi_init(void)
{
	snapspi_setup();
	misc_register(&snapspi_dev);
	printk("SNAPSPI: bit-banged SPI driver\n");
	return 0;
}

static void __exit snapspi_exit(void)
{
	misc_deregister(&snapspi_dev);
}

/*****************************************************************************/

module_init(snapspi_init);
module_exit(snapspi_exit);

MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_LICENSE("GPL");

/*****************************************************************************/
