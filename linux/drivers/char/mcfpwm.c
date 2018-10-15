/*
 * mcfpwm.c, 1.0
 *
 * Copyright (c) 2006 Arcturus Networks Inc.
 *                    by MaTed <www.ArcturusNetworks.com>
 *
 * Description:
 * 	- this device is just to allow register access to the PWM
 * 	  registers of the MCF532x
 * 	- device mapping of PWM registers to User Mode is broken
 * NOTES:
 * 	- code written for nonMMU
 * 	- would be "nic" to add audio, but for now, only IOCTL for access 
 * 	  to PWM registers
 *      - this is only tested on M532x
 *
 * changes:
 * 	- May 23/08		Initial version
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/autoconf.h>
#include <linux/ioctl.h>

#if defined(CONFIG_UC53281EVM)	/* Only valid on uC532x */

/** IOCTL **/
#define MCFPWM_IOC_MAGIC  'Z'
#define MCFPWM_IOC_MAXNR  28

/* Under the 53281 eratta, Userland access is not valid */
#define MCFPWM_IOCSUSEACCESS 	_IOW(MCFPWM_IOC_MAGIC, 0, int)	/* set PACR36 bits */
#define MCFPWM_IOCGSYSACCESS 	_IOR(MCFPWM_IOC_MAGIC, 1, int)	/* get PACR36 bits */

/* The following take/return the lower 8 bits as the data to the register */
#define MCFPWM_IOCS_GPIO	_IOW(MCFPWM_IOC_MAGIC, 2, int)	/* set PAR_PWM reg */
#define MCFPWM_IOCG_GPIO	_IOR(MCFPWM_IOC_MAGIC, 3, int)	/* get PAR_PWM reg */
#define MCFPWM_IOCS_ENABLE	_IOW(MCFPWM_IOC_MAGIC, 4, int)	/* set PWME reg */
#define MCFPWM_IOCG_ENABLE	_IOR(MCFPWM_IOC_MAGIC, 5, int)	/* get PWME reg */
#define MCFPWM_IOCS_POL		_IOW(MCFPWM_IOC_MAGIC, 6, int)	/* set PWMPOL reg */
#define MCFPWM_IOCG_POL		_IOR(MCFPWM_IOC_MAGIC, 7, int)	/* get PWMPOL reg */
#define MCFPWM_IOCS_CLK		_IOW(MCFPWM_IOC_MAGIC, 8, int)	/* set PWMCLK reg */
#define MCFPWM_IOCG_CLK		_IOR(MCFPWM_IOC_MAGIC, 9, int)	/* get PWMCLK reg */
#define MCFPWM_IOCS_PRCLK	_IOW(MCFPWM_IOC_MAGIC, 10, int)	/* set PWMPRCLK reg */
#define MCFPWM_IOCG_PRCLK	_IOR(MCFPWM_IOC_MAGIC, 11, int)	/* get PWMPRCLK reg */
#define MCFPWM_IOCS_CAE		_IOW(MCFPWM_IOC_MAGIC, 12, int)	/* set PWMCAE reg */
#define MCFPWM_IOCG_CAE		_IOR(MCFPWM_IOC_MAGIC, 13, int)	/* get PWMCAE reg */
#define MCFPWM_IOCS_CTL		_IOW(MCFPWM_IOC_MAGIC, 14, int)	/* set PWMCTL reg */
#define MCFPWM_IOCG_CTL		_IOR(MCFPWM_IOC_MAGIC, 15, int)	/* get PWMCTL reg */
#define MCFPWM_IOCS_SCLA	_IOW(MCFPWM_IOC_MAGIC, 16, int)	/* set PWMSCLA reg */
#define MCFPWM_IOCG_SCLA 	_IOR(MCFPWM_IOC_MAGIC, 17, int)	/* get PWMSCLA reg */
#define MCFPWM_IOCS_SCLB 	_IOW(MCFPWM_IOC_MAGIC, 18, int)	/* set PWMSCLB reg */
#define MCFPWM_IOCG_SCLB 	_IOR(MCFPWM_IOC_MAGIC, 19, int)	/* get PWMSCLB reg */
#define MCFPWM_IOCS_SHUT 	_IOW(MCFPWM_IOC_MAGIC, 20, int)	/* set PWMSDN reg */
#define MCFPWM_IOCG_SHUT 	_IOR(MCFPWM_IOC_MAGIC, 21, int)	/* get PWMSDN reg */

/* the following take the upper 16(3) bits as the Channel, lower (8) bits as data */
/* Counter register access */
#define MCFPWM_IOCS_CNT_N 	_IOW(MCFPWM_IOC_MAGIC, 22, int)	/* set PWMCNTn reg */
#define MCFPWM_IOCG_CNT_N 	_IOR(MCFPWM_IOC_MAGIC, 23, int)	/* get PWMCNTn reg */
/* Period register access */
#define MCFPWM_IOCS_PER_N 	_IOW(MCFPWM_IOC_MAGIC, 24, int)	/* set PWMPERn reg */
#define MCFPWM_IOCG_PER_N 	_IOR(MCFPWM_IOC_MAGIC, 25, int)	/* get PWMPERn reg */
/* Period register access */
#define MCFPWM_IOCS_DTY_N 	_IOW(MCFPWM_IOC_MAGIC, 26, int)	/* set PWMDTYn reg */
#define MCFPWM_IOCG_DTY_N 	_IOR(MCFPWM_IOC_MAGIC, 27, int)	/* get PWMDTYn reg */

#if !defined(inl)
#define readb(addr) \
 ({ unsigned char __v = (*(volatile unsigned char *) (addr)); __v; })
#define readw(addr) \
 ({ unsigned short __v = (*(volatile unsigned short *) (addr)); __v; })
#define readl(addr) \
 ({ unsigned int __v = (*(volatile unsigned int *) (addr)); __v; })

#define writeb(b,addr) (void)((*(volatile unsigned char *) (addr)) = (b))
#define writew(b,addr) (void)((*(volatile unsigned short *) (addr)) = (b))
#define writel(b,addr) (void)((*(volatile unsigned int *) (addr)) = (b))

#define inb(addr) readb(addr)
#define inw(addr) readw(addr)
#define inl(addr) readl(addr)

#define outb(x,addr) ((void) writeb(x,addr))
#define outw(x,addr) ((void) writew(x,addr))
#define outl(x,addr) ((void) writel(x,addr))

#endif

/** PWM register definitions */
#define PWM_PACRE	0xFC000040	/* default 0x4 */
#define PWM_PODR_PWM	0xFC0A4006	/* default 0x3c */
#define PWM_PDDR_PWM	0xFC0A401A	/* default 0x00 */
#define PWM_PPDSDR_PWM	0xFC0A402E	/* default 0b00xxxx00 */
#define PWM_PCLRR_PWM	0xFC0A4042	/* default 0x00 (write only) */
#define PWM_PAR_PWM	0xFC0A4051	/* default 0x00 */
#define PWM_DSCR_PWM	0xFC0A4069	/* default 0bx1 */
#define PWM_PWME	0xFC090020	/* default 0x00 */
#define PWM_PWMPOL	0xFC090021	/* default 0x00 */
#define PWM_PWMCLK	0xFC090022	/* default 0x00 */
#define PWM_PWMPRCLK	0xFC090023	/* default 0x00 */
#define PWM_PWMCAE	0xFC090024	/* default 0x00 */
#define PWM_PWMCTL	0xFC090025	/* default 0x00 */
#define PWM_PWMSCLA	0xFC090028	/* default 0x00 */
#define PWM_PWMSCLB	0xFC090029	/* default 0x00 */
#define PWM_PWMCNT0	0xFC09002C	/* default 0x00 */
#define PWM_PWMPER0	0xFC090034	/* default 0xFF */
#define PWM_PWMMDTY0	0xFC09003C	/* default 0xFF */
#define PWM_PWMSDN	0xFC090044	/* default 0x00 */

#define Major	122
#define Minor	0

static int Device_Open = 0;

static int mcf_pwm_ioctl(struct inode *inode, struct file *filp,
			 unsigned int cmd, unsigned long arg);

static int mcf_pwm_open(struct inode *inode, struct file *filp)
{
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	/* Enable access to pwm registers from Userland
	 * ** NOTE: Cannot access from Userland
	 */
	outl(inl(PWM_PACRE) & 0xFFFF0FFF, PWM_PACRE);
	return 0;
}

static int mcf_pwm_release(struct inode *inode, struct file *filp)
{
	Device_Open--;
	return 0;
}

static int mcf_pwm_ioctl(struct inode *inode, struct file *filp,
			 unsigned int cmd, unsigned long arg)
{
	unsigned char chan;
	unsigned char data;
	unsigned int ret;

	union {
		struct {
			unsigned char filler1;
			unsigned char channel;	/* Channel number */
			unsigned char filler2;
			unsigned char data;	/* data to be written / returned */
		} s;
		unsigned int arg;
	} u;

	u.arg = (unsigned int)arg;
	chan = (u.s.channel) & 0x3;
	data = u.s.data;

	if (_IOC_TYPE(cmd) != MCFPWM_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > MCFPWM_IOC_MAXNR)
		return -ENOTTY;

	ret = 0;

	switch (cmd) {
	case MCFPWM_IOCSUSEACCESS:	/* set PACR36 bits - default 0x4 */
		outl((data << 12) | (inl(PWM_PACRE) & 0xFFFF0FFF), PWM_PACRE);
		break;
	case MCFPWM_IOCGSYSACCESS:	/* get PACR36 bits */
		ret = ((inl(PWM_PACRE)) >> 12) & 0xf;
		break;
	/* The following take/return the lower 8 bits as the data to the register */
	case MCFPWM_IOCS_GPIO:	/* set PAR_PWM reg */
		outb(data, PWM_PAR_PWM);
		break;
	case MCFPWM_IOCG_GPIO:	/* get PAR_PWM reg */
		ret = inb(PWM_PAR_PWM);
		break;
	case MCFPWM_IOCS_ENABLE:	/* set PWME reg */
		outb(data, PWM_PWME);
		break;
	case MCFPWM_IOCG_ENABLE:	/* get PWME reg */
		ret = inb(PWM_PWME);
		break;
	case MCFPWM_IOCS_POL:	/* set PWMPOL reg */
		outb(data, PWM_PWMPOL);
		break;
	case MCFPWM_IOCG_POL:	/* get PWMPOL reg */
		ret = inb(PWM_PWMPOL);
		break;
	case MCFPWM_IOCS_CLK:	/* set PWMCLK reg */
		outb(data, PWM_PWMCLK);
		break;
	case MCFPWM_IOCG_CLK:	/* get PWMCLK reg */
		ret = inb(PWM_PWMCLK);
		break;
	case MCFPWM_IOCS_PRCLK:	/* set PWMPRCLK reg */
		outb(data, PWM_PWMPRCLK);
		break;
	case MCFPWM_IOCG_PRCLK:	/* get PWMPRCLK reg */
		ret = inb(PWM_PWMPRCLK);
		break;
	case MCFPWM_IOCS_CAE:	/* set PWMCAE reg */
		outb(data, PWM_PWMCAE);
		break;
	case MCFPWM_IOCG_CAE:	/* get PWMCAE reg */
		ret = inb(PWM_PWMCAE);
		break;
	case MCFPWM_IOCS_CTL:	/* set PWMCTL reg */
		outb(data, PWM_PWMCTL);
		break;
	case MCFPWM_IOCG_CTL:	/* get PWMCTL reg */
		ret = inb(PWM_PWMCTL);
		break;
	case MCFPWM_IOCS_SCLA:	/* set PWMSCLA reg */
		outb(data, PWM_PWMSCLA);
		break;
	case MCFPWM_IOCG_SCLA:	/* get PWMSCLA reg */
		ret = inb(PWM_PWMSCLA);
		break;
	case MCFPWM_IOCS_SCLB:	/* set PWMSCLB reg */
		outb(data, PWM_PWMSCLB);
		break;
	case MCFPWM_IOCG_SCLB:	/* get PWMSCLB reg */
		ret = inb(PWM_PWMSCLB);
		break;
	case MCFPWM_IOCS_SHUT:	/* set PWMSDN reg */
		outb(data, PWM_PWMSDN);
		break;
	case MCFPWM_IOCG_SHUT:	/* get PWMSDN reg */
		ret = inb(PWM_PWMSDN);
		break;

	/* the following take the upper 16(3) bits as the Channel, 
	 * lower 16 (8) bits as data. Counter register access 
         */
	case MCFPWM_IOCS_CNT_N:	/* set PWMCNTn reg */
		outb(data, chan + PWM_PWMCNT0);
		break;
	case MCFPWM_IOCG_CNT_N:	/* get PWMCNTn reg */
		ret = inb(chan + PWM_PWMCNT0);
		break;

	/* Period register access */
	case MCFPWM_IOCS_PER_N:	/* set PWMPERn reg */
		outb(data, chan + PWM_PWMPER0);
		break;
	case MCFPWM_IOCG_PER_N:	/* get PWMPERn reg */
		ret = inb(chan + PWM_PWMPER0);
		break;

	/* Period register access */
	case MCFPWM_IOCS_DTY_N:	/* set PWMDTYn reg */
		outb(data, chan + PWM_PWMMDTY0);
		break;
	case MCFPWM_IOCG_DTY_N:	/* get PWMDTYn reg */
		ret = inb(chan + PWM_PWMMDTY0);
		break;

	default:
		return -EINVAL;
	}
	return ret;
}

static struct file_operations mcf_pwm_fops = {
	.ioctl = mcf_pwm_ioctl,
	.open = mcf_pwm_open,
	.release = mcf_pwm_release,
};

static int mcf_pwm_init(void)
{
	int ret;

	ret = register_chrdev(Major, "mcfpwm", &mcf_pwm_fops);
	if (ret < 0) {
		printk("MCF_PWM device failed registration with %d\n", Major);
		return Major;
	}
	printk("MCF_PWM: %d initialized\n", Major);
	return 0;
}

static void mcf_pwm_exit(void)
{
	if (unregister_chrdev(Major, "MCF_PWM"))
		printk("MCF_PWM: unregister failed\n");
}

module_init(mcf_pwm_init);
module_exit(mcf_pwm_exit);

MODULE_AUTHOR("MaTed / Ted Ma (www.ArcturusNetworks.com)");
MODULE_LICENSE("GPL");
#endif
