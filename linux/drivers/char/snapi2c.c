/*****************************************************************************/

/*
 *	snapi2c.c -- driver for SnapGear bit-banged I2C interface.
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
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/semaphore.h>

/*****************************************************************************/

#define	SNAPI2C_MSIZE	0x100

#define	SNAPI2C_ADDR	0xd4		/* IIC address of IDT5V9886C device */
#define	SNAPI2C_RD	1		/* Read bit command */
#define	SNAPI2C_WR	0		/* Write bit command */

DECLARE_MUTEX(snapi2c_sem);

/*****************************************************************************/
#ifdef CONFIG_MACH_IPD
/*****************************************************************************/

#include <asm/io.h>

/*
 * On the EP9312/IPD,  the clock in on EGIO[1] and the data is on EGPIO[3]
 */
#define	SDA	0x8
#define	SCL	0x2
#define IN	0
#define OUT	1

static void gpio_line_config(int line, int dir)
{
	unsigned long flags;

	save_flags(flags); cli();
	if (dir == OUT)
		outl(inl(GPIO_PADDR) | line, GPIO_PADDR); /* data is output */
	else
		outl(inl(GPIO_PADDR) & ~line, GPIO_PADDR); /* data is input */
	restore_flags(flags);
}

static void gpio_line_set(int line, int val)
{
	unsigned long flags;

	save_flags(flags); cli();
	if (val)
		outl(inl(GPIO_PADR) | line, GPIO_PADR);
	else
		outl(inl(GPIO_PADR) & ~line, GPIO_PADR);
	restore_flags(flags);
}

static inline void gpio_line_get(int line, int *val)
{
	*val = (inl(GPIO_PADR) & line) ? 1 : 0;
}

/*****************************************************************************/
#elif defined(CONFIG_MACH_CM41xx) || defined(CONFIG_MACH_CM4008)
/*****************************************************************************/

#include <asm/io.h>

/*
 *	GPIO lines 6, 7 and 8 are used for the RTC.
 */
#define	SDAT	6		/* SDA transmit */
#define	SDAR	7		/* SDA receiver */
#define	SCL	8		/* SCL - clock */
#define	SDA	SDAR

#define IN	0
#define OUT	1

#define	SDAT_B	(1 << SDAT)
#define	SDAR_B	(1 << SDAR)
#define	SCL_B	(1 << SCL)

static volatile unsigned int *gpdatap = (volatile unsigned int *) (IO_ADDRESS(KS8695_IO_BASE) + KS8695_GPIO_DATA);
static volatile unsigned int *gpmodep = (volatile unsigned int *) (IO_ADDRESS(KS8695_IO_BASE) + KS8695_GPIO_MODE);

static inline void gpio_line_config(int line, int dir)
{
	if (line == SDA) {
		if (dir == IN)
			*gpdatap |= SDAT_B;
	}
	if (line == SCL) {
		/* We do normal initialization for all GPIO bits here */
		*gpmodep |= SCL_B | SDAT_B;
		*gpmodep &= ~SDAR_B;
	}
}

static inline void gpio_line_set(int line, int val)
{
	if (line == SCL) {
		if (val)
			*gpdatap |= SCL_B;
		else
			*gpdatap &= ~SCL_B;
	} else {
		if (val)
			*gpdatap |= SDAT_B;
		else
			*gpdatap &= ~SDAT_B;
	}
}

static inline void gpio_line_get(int line, int *val)
{
	*val = (*gpdatap & SDAR_B) ? 1 : 0;
}

/*****************************************************************************/
#else
/*****************************************************************************/

/*
 *	The IIC lines to the IDT5V9885C are GPIO lines from the IXP4xx.
 *	The clock line is on GPIO12, and the data line on GPIO11.
 */
#define	SDA	11
#define	SCL	12
#define	IN	IXP4XX_GPIO_IN
#define	OUT	IXP4XX_GPIO_OUT

/*****************************************************************************/
#endif
/*****************************************************************************/

static void gpio_line_config_slow(u8 line, u32 style)
{
	gpio_line_config(line, style);
	udelay(10);
}

static void gpio_line_set_slow(u8 line, int value)
{
	gpio_line_set(line, value);
	udelay(10);
}

static void gpio_line_get_slow(u8 line, int *value)
{
	gpio_line_get(line, value);
	udelay(10);
}

/*****************************************************************************/

void snapi2c_readack(void)
{
	unsigned int ack;

	gpio_line_config_slow(SDA, IN);
	gpio_line_set_slow(SCL, 1);
	gpio_line_get_slow(SDA, &ack);
	gpio_line_set_slow(SCL, 0);
	gpio_line_config_slow(SDA, OUT);
}

void snapi2c_writeack(void)
{
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SCL, 0);
}

void snapi2c_writenack(void)
{
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SCL, 0);
}

void snapi2c_sendbits(unsigned int val)
{
	int i;

	gpio_line_set_slow(SCL, 0);
	for (i = 7; (i >= 0); i--) {
		gpio_line_set_slow(SDA, ((val >> i) & 0x1));
		gpio_line_set_slow(SCL, 1);
		gpio_line_set_slow(SCL, 0);
	}
}

unsigned int snapi2c_recvbits(void)
{
	unsigned int val, bit;
	int i;

	gpio_line_set_slow(SCL, 0);
	gpio_line_config_slow(SDA, IN);
	for (i = 0, val = 0; (i < 8); i++) {
		gpio_line_set_slow(SCL, 1);
		gpio_line_get_slow(SDA, &bit);
		val = (val << 1) | bit;
		gpio_line_set_slow(SCL, 0);
	}

	gpio_line_config_slow(SDA, OUT);
	return val;
}

/*****************************************************************************/

/* 
 *	The read byte sequenece is actually a write sequence followed
 *	by the read sequenece. The first write is to set the register
 *	address, and is a complete cycle itself.
 */
unsigned int snapi2c_readbyte(unsigned int addr)
{
	unsigned int id, val;

	down(&snapi2c_sem);
#if 0
	printk("snapi2c_readbyte(addr=%x)\n", addr);
#endif

	/* Send start signal */
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SDA, 0);

	/* Send IDT5V9885C device address byte, and write command for addr */
	snapi2c_sendbits(SNAPI2C_ADDR | SNAPI2C_WR);
	snapi2c_readack();

	snapi2c_sendbits(0);
	snapi2c_readack();

	snapi2c_sendbits(addr);
	snapi2c_readack();

	/* Send stop signal */
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);


	/* Now send sequence to read bytes, starting with start signal */
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SDA, 0);

	/* Send IDT5V9885C device address byte, and read command for addr */
	snapi2c_sendbits(SNAPI2C_ADDR | SNAPI2C_RD);
	snapi2c_readack();
	id = snapi2c_recvbits();
	snapi2c_writeack();
	val = snapi2c_recvbits();
	snapi2c_writenack();

	/* Send stop signal */
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);

	up(&snapi2c_sem);

	return val;
}

/*****************************************************************************/

void snapi2c_writebyte(unsigned int addr, unsigned int val)
{
	down(&snapi2c_sem);
#if 0
	printk("snapi2c_writebyte(addr=%x)\n", addr);
#endif

	/* Send start signal */
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SDA, 0);

	/* Send IDT5V9885C device address byte, and write command */
	snapi2c_sendbits(SNAPI2C_ADDR | SNAPI2C_WR);
	snapi2c_readack();

	/* Send write command byte */
	snapi2c_sendbits(0);
	snapi2c_readack();

	/* Send word address and data to write */
	snapi2c_sendbits(addr);
	snapi2c_readack();
	snapi2c_sendbits(val);
	snapi2c_readack();

	/* Send stop signal */
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);

	up(&snapi2c_sem);
}

/*****************************************************************************/

void snapi2c_progsave(void)
{
	down(&snapi2c_sem);
#if 0
	printk("snapi2c_progsave()\n");
#endif

	/* Send start signal */
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SDA, 0);

	/* Send IDT5V9885C device address byte, and write command */
	snapi2c_sendbits(SNAPI2C_ADDR | SNAPI2C_WR);
	snapi2c_readack();

	/* Send write command byte */
	snapi2c_sendbits(0x1);
	snapi2c_readack();

	/* Send stop signal */
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);

	up(&snapi2c_sem);
}

/*****************************************************************************/

void snapi2c_progrestore(void)
{
	down(&snapi2c_sem);
#if 0
	printk("snapi2c_progrestore()\n");
#endif

	/* Send start signal */
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);
	gpio_line_set_slow(SDA, 0);

	/* Send IDT5V9885C device address byte, and write command */
	snapi2c_sendbits(SNAPI2C_ADDR | SNAPI2C_WR);
	snapi2c_readack();

	/* Send write command byte */
	snapi2c_sendbits(0x2);
	snapi2c_readack();

	/* Send stop signal */
	gpio_line_set_slow(SDA, 0);
	gpio_line_set_slow(SCL, 1);
	gpio_line_set_slow(SDA, 1);

	up(&snapi2c_sem);
}

/*****************************************************************************/

void snapi2c_setup(void)
{
	down(&snapi2c_sem);

	/* Initially set the IIC lines to be outputs from the IXP4xx */
	gpio_line_config(SCL, OUT);
	gpio_line_config(SDA, OUT);

	/* Set IIC bus into idle mode */
	gpio_line_set(SCL, 1);
	gpio_line_set(SDA, 1);

	up(&snapi2c_sem);
}

/*****************************************************************************/

static ssize_t snapi2c_read(struct file *fp, char __user *buf, size_t count, loff_t *ptr)
{
	loff_t p = *ptr;
	int total;

#if 0
	printk("snapi2c_read(buf=%x,count=%d)\n", (int) buf, count);
#endif

	if (p >= SNAPI2C_MSIZE)
		return 0;

	if (count > (SNAPI2C_MSIZE - p))
		count = SNAPI2C_MSIZE - p;

	for (total = 0; (total < count); total++)
		put_user(snapi2c_readbyte(p + total), buf++);

	*ptr += total;
	return total;
}

/*****************************************************************************/

static ssize_t snapi2c_write(struct file *fp, const char __user *buf, size_t count, loff_t *ptr)
{
	loff_t p = *ptr;
	int total;
	char val;

#if 0
	printk("snapi2c_write(buf=%x,count=%d)\n", (int) buf, count);
#endif

	if (p >= SNAPI2C_MSIZE)
		return 0;

	if (count > (SNAPI2C_MSIZE - p))
		count = SNAPI2C_MSIZE - p;

	for (total = 0; (total < count); total++, buf++) {
		get_user(val,buf);
		snapi2c_writebyte((p + total), val);
	}

	snapi2c_progsave();

	*ptr += total;
	return total;
}

/*****************************************************************************/

/*
 *	Exported file operations structure for driver...
 */
static struct file_operations snapi2c_fops =
{
	.owner = THIS_MODULE, 
	.read =  snapi2c_read,
	.write = snapi2c_write,
};

static struct miscdevice snapi2c_dev =
{
	RTC_MINOR,
	"i2c",
	&snapi2c_fops
};

/*****************************************************************************/

static int __init snapi2c_init(void)
{
	snapi2c_setup();
	misc_register(&snapi2c_dev);
	printk("SNAPI2C: bit-banged I2C driver\n");
	return 0;
}

static void __exit snapi2c_exit(void)
{
	misc_deregister(&snapi2c_dev);
}

/*****************************************************************************/

module_init(snapi2c_init);
module_exit(snapi2c_exit);

MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_LICENSE("GPL");

/*****************************************************************************/
