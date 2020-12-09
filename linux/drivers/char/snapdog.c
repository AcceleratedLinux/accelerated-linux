/****************************************************************************/
/*
 *	SnapGear Hardware Watchdog driver (this WD cannot be stopped)
 *
 *	Copyright 2004 David McCullough <davidm@snapgear.com>, All Rights Reserved.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *	
 *	based on softdog.c by Alan Cox <alan@redhat.com>
 */
/****************************************************************************/
 
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/reboot.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/uaccess.h>
#include <asm/irq_regs.h>

/****************************************************************************/
/*
 * here we put the platform specific bits (headers/poke function)
 */

#ifdef CONFIG_SH_SECUREEDGE5410
	#include <asm/io.h>

	static inline void enable_dog(void) {}

	static inline void poke_the_dog(void)
	{
		volatile char dummy;
		dummy = * (volatile char *) 0xb8000000;
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#ifdef CONFIG_MACH_IPD
	#include <asm/io.h>

	static volatile char *dog_addr = NULL;

	static inline void enable_dog(void)
	{
		dog_addr = (char *) ioremap(0x20000000, 32);
	}

	static inline void poke_the_dog(void)
	{
		if (dog_addr) {
			volatile char dummy = *dog_addr;
		}
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_ESS710) || defined(CONFIG_MACH_IVPN) || \
    defined(CONFIG_MACH_SG560) || defined(CONFIG_MACH_SG580) || \
    defined(CONFIG_MACH_SG640) || defined(CONFIG_MACH_SG720) || \
    defined(CONFIG_MACH_SG590) || defined(CONFIG_MACH_SE5100)
	#include <asm/io.h>
	#include <mach/ixp4xx-gpio.h>

	static inline void enable_dog(void)
	{
		*IXP4XX_GPIO_GPCLKR &= 0xffff0000;
	}

	static inline void poke_the_dog(void)
	{
		*IXP4XX_GPIO_GPOUTR ^= 0x4000;
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_SG8100)
	#include <asm/io.h>
	#include <mach/ixp4xx-gpio.h>

	static inline void enable_dog(void)
	{
	}

	static inline void poke_the_dog(void)
	{
		*IXP4XX_GPIO_GPOUTR ^= 0x2000;
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_SG560USB) || defined(CONFIG_MACH_SG560ADSL) || \
    defined(CONFIG_MACH_SG565) || defined(CONFIG_MACH_SHIVA1100)
	#include <asm/io.h>
	#include <mach/sg.h>

	static volatile unsigned char *wdtcs2;

	static inline void enable_dog(void)
	{
		/* CS7 is watchdog alive. Set it to 8bit and writable */
		*SG565_WATCHDOG_EXP_CS = 0xbfff0003;
		wdtcs2 = (volatile unsigned char *) ioremap(SG565_WATCHDOG_BASE_PHYS, 512);
	}

	static inline void poke_the_dog(void)
	{
		if (wdtcs2)
			*wdtcs2 = 0;
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#ifdef CONFIG_GEODEWATCHDOG
	#include <asm/io.h>

	static inline void enable_dog(void) {}

	static inline void poke_the_dog(void)
	{
		unsigned int v;
		v = inl(0x6410);
		outl((v | 0x200), 0x6410);
		outl((v & ~0x200), 0x6410);
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_SG590) || defined(CONFIG_SG8200)
	#include <asm/mach-cavium-octeon/gpio.h>

	static int wdt_state;

	static inline void enable_dog(void)
	{
		octeon_gpio_raw_config(9, OCTEON_GPIO_OUTPUT);
	}

	static inline void poke_the_dog(void)
	{
		if (wdt_state++ & 1)
			octeon_gpio_raw_clear(0x1 << 9);
		else
			octeon_gpio_raw_set(0x1 << 9);
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_8300) || defined(CONFIG_MACH_6300CX) || \
    defined(CONFIG_MACH_6300LX) || defined(CONFIG_MACH_6330MX) || \
    defined(CONFIG_MACH_6350SR) || defined(CONFIG_MACH_CM71xx) || \
    defined(CONFIG_MACH_ACM700x)
	#include <asm/gpio.h>

#ifdef CONFIG_MACH_CM71xx
	#define GPIO_WATCHDOG   46
#elif CONFIG_MACH_ACM700x
	#define GPIO_WATCHDOG 61
	#define GPIO_WATCHDOG_EN 60
#else
	#define GPIO_WATCHDOG	54
#endif

	static int wdt_state;
	static int dog_initted;

	static inline void enable_dog(void)
	{
#ifdef GPIO_WATCHDOG_EN
		gpio_request(GPIO_WATCHDOG_EN, "Watchdog Enable");
		gpio_direction_output(GPIO_WATCHDOG_EN, 1);
#endif
		gpio_request(GPIO_WATCHDOG, "Watchdog");
		gpio_direction_output(GPIO_WATCHDOG, 1);
		dog_initted = 1;
	}

	static inline void poke_the_dog(void)
	{
		if (dog_initted)
			gpio_set_value(GPIO_WATCHDOG, (wdt_state++ & 0x1));
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_ARMADA_38X)
	#include <linux/io.h>

	/*
	 * We need to be able to start kicking the dog early in kernel
	 * boot, so we setup the GPIO as soon as possble. We avoid using
	 * the kernels own GPIO services - they are setup up too late
	 * in the boot process.
	 */
#ifdef CONFIG_MACH_U115
	#define GPIO_ADDR	0xf1018100
	#define GPIO_WDT	20	/* GPIO 20 (low bank) */
	#define GPIO_WDT_EN	19	/* GPIO 19 (low bank) */
#else
	#define GPIO_ADDR	0xf1018140
	#define GPIO_WDT	21	/* GPIO 53 (high bank) */
#endif

	static int wdt_state;
	static void __iomem *snapdog_regp;

	static inline void poke_the_dog(void)
	{
		u32 v;

		if (snapdog_regp) {
			/* Toggle WDI value */
			v = readl(snapdog_regp + 0x0);
			if (wdt_state++ & 0x1)
				v &= ~(0x1 << GPIO_WDT);
			else
				v |= (0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x0);
		}
	}

	static int enable_dog(void)
	{
		u32 v;

		if (snapdog_regp == NULL) {
			snapdog_regp = ioremap(GPIO_ADDR, 0x40);

			/* Set WDT GPIO as output */
			v = readl(snapdog_regp + 0x4);
			v &= ~(0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x4);

#ifdef CONFIG_MACH_U115
			/* Enable WDT GPIO as output */
			v = readl(snapdog_regp + 0x4);
			v &= ~(0x1 << GPIO_WDT_EN);
			writel(v, snapdog_regp + 0x4);

			/* now enable it */
			v = readl(snapdog_regp + 0x0);
			v |= (0x1 << GPIO_WDT_EN);
			writel(1, snapdog_regp + 0x0);
#endif
		}

		return 0;
	}

	early_initcall(enable_dog);

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_SOC_IMX6UL)
	#include <linux/io.h>

	/*
	 * We need to be able to start kicking the dog early in kernel
	 * boot, so we setup the GPIO as soon as possble. We avoid using
	 * the kernels own GPIO services - they are setup up too late
	 * in the boot process.
	 */
	static const u32 bankaddr[] = {
		0,
		0x0209c000,
		0x020a0000,
		0x020a4000,
		0x020a8000,
		0x020ac000,
	};

	static int wdt_state;
	static void __iomem *snapdog_regp;

	static inline void poke_the_dog(void)
	{
		u32 v;

		if (snapdog_regp) {
			/* Toggle WDI value */
			v = readl(snapdog_regp + 0x0);
			if (wdt_state++ & 0x1)
				v &= ~(0x1 << CONFIG_SNAPDOG_GPIO_BIT);
			else
				v |= (0x1 << CONFIG_SNAPDOG_GPIO_BIT);
			writel(v, snapdog_regp + 0x0);
		}
	}

	static int enable_dog(void)
	{
		u32 v;

		if (snapdog_regp == NULL) {
			const u32 addr = bankaddr[CONFIG_SNAPDOG_GPIO_BANK];

			snapdog_regp = ioremap(addr, 0x40);

			/* Set WDT GPIO as output */
			v = readl(snapdog_regp + 0x4);
			v |= (0x1 << CONFIG_SNAPDOG_GPIO_BIT);
			writel(v, snapdog_regp + 0x4);
		}

		return 0;
	}

	early_initcall(enable_dog);

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_UTM400)
	#include <asm/gpio.h>

	static int wdt_state;

	static int dog_initted = 0;

	static inline void enable_dog(void) { dog_initted = 1; }

	static inline void poke_the_dog(void)
	{
		if (dog_initted)
			gpio_set_value(44, wdt_state++ & 0x1);
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#ifdef CONFIG_UTM2000
	#include <asm/io.h>
	#include "../watchdog/iwdt.h"

	#define	WDT_INDEX	0x4e
	#define	WDT_DATA	0x4f

	static u16 wdt_iobase;

	static void wdt_get_iobase(void)
	{
		outb(0x80, WDT_INDEX);
		outb(0x86, WDT_INDEX);

		outb(0x07, WDT_INDEX);
		outb(0x06, WDT_DATA);

		outb(0x60, WDT_INDEX);
		wdt_iobase = inb(WDT_DATA) << 8;
		outb(0x61, WDT_INDEX);
		wdt_iobase |= inb(WDT_DATA);

		outb(0x68, WDT_INDEX);
		outb(0x08, WDT_INDEX);
	}

	static inline void poke_the_dog(void)
	{
		if (wdt_iobase) {
			unsigned long flags;
			local_irq_save(flags);
			outb(0x80, wdt_iobase + WDT_RLD_REG0);
			outb(0x86, wdt_iobase + WDT_RLD_REG0);
			outb(0x01, wdt_iobase + WDT_RLD_REG1);
			local_irq_restore(flags);
		}
	}

	#define	T1	2000		/* Primary counter is 2s */
	#define	T2	100		/* Secondary count is 100ms */

	static inline void enable_dog(void)
	{
		wdt_get_iobase();
		printk("snapdog: internal WDT watchdog I/0 address 0x%x\n",
			wdt_iobase);

		if (wdt_iobase == 0) {
			printk("snapdog: WDT hardware not found!\n");
			return;
		}

		/* Clear timeout condition */
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb(0x02, wdt_iobase + WDT_RLD_REG1);

		/* Set timer pre-load counters */
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb((T1 & 0xff), wdt_iobase + WDT_PRELD1_REG0);
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb(((T1 >> 8) & 0xff), wdt_iobase + WDT_PRELD1_REG1);
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb(((T1 >> 16) & 0xff), wdt_iobase + WDT_PRELD1_REG2);

		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb((T2 & 0xff), wdt_iobase + WDT_PRELD2_REG0);
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb(((T2 >> 8) & 0xff), wdt_iobase + WDT_PRELD2_REG1);
		outb(0x80, wdt_iobase + WDT_RLD_REG0);
		outb(0x86, wdt_iobase + WDT_RLD_REG0);
		outb(((T2 >> 16) & 0xff), wdt_iobase + WDT_PRELD2_REG2);

		/* Set WDT config */
		outb(0, wdt_iobase + WDT_CONFIG_REG);

		poke_the_dog();

		/* Enable the watchdog, and then lock it */
		outb(WDT_ENABLE, wdt_iobase + WDT_LOCK_REG);
		outb(WDT_ENABLE | WDT_LOCK, wdt_iobase + WDT_LOCK_REG);

		poke_the_dog();
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_ATH79_MACH_6300_DX)
	#include <linux/gpio.h>

	#define NB_GPIO_WDT_WDI		26

	static int dog_inited;
	static int wdt_state;

	static inline void enable_dog(void)
	{
		/* Setup GPIOs that run the external power watchdog */
		gpio_request(NB_GPIO_WDT_WDI, "WDI");
		gpio_direction_output(NB_GPIO_WDT_WDI, 1);
		gpio_set_value_cansleep(NB_GPIO_WDT_WDI, 1);
		dog_inited = 1;
	}

	static inline void poke_the_dog(void)
	{
		if (dog_inited) {
			gpio_set_value(NB_GPIO_WDT_WDI, wdt_state & 1);
			wdt_state++;
		}
	}

	static inline void the_dog_is_dead(void) {}

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_5400_RM_DT) || defined(CONFIG_MACH_6300_EX_DT)
	#include <linux/io.h>
	/*
	 * We need to be able to start kicking the dog early in kernel
	 * boot, so we setup the GPIO as soon as possble. We avoid using
	 * the kernels own GPIO services - they are setup up too late
	 * in the boot process.
	 */
	#define GPIO_WDT		4 /* High bank GPIO36 */

	static int wdt_state = 0;
	static void *snapdog_regp;

	static void poke_the_dog(void)
	{
		u32 v;

		if (snapdog_regp) {
			/* Toggle WDI value */
			v = readl(snapdog_regp + 0x10140);
			if (wdt_state++ & 0x1)
				v &= ~(0x1 << GPIO_WDT);
			else
				v |= (0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x10140);
		}
	}

	static int enable_dog(void)
	{
		u32 v;

		if (snapdog_regp == NULL) {
			snapdog_regp = ioremap(0xf1000000, 128*1024);

			/* Set WDI GPIO as output */
			v = readl(snapdog_regp + 0x10144);
			v &= ~(0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x10144);

			poke_the_dog();
			poke_the_dog();
		}
		return 0;
	}
	static inline void the_dog_is_dead(void) {}

	core_initcall(enable_dog);

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_MACH_IM72xx)
	#include <linux/io.h>
	/*
	 * We need to be able to start kicking the dog early in kernel
	 * boot, so we setup the GPIO as soon as possble. We avoid using
	 * the kernels own GPIO services - they are setup up too late
	 * in the boot process.
	 */
	#define GPIO_WDT		7 /* Low bank GPIO7 */

	static int wdt_state = 0;
	static void *snapdog_regp;

	static void poke_the_dog(void)
	{
		u32 v;

		if (snapdog_regp) {
			/* Toggle WDI value */
			v = readl(snapdog_regp + 0x10100);
			if (wdt_state++ & 0x1)
				v &= ~(0x1 << GPIO_WDT);
			else
				v |= (0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x10100);
		}
	}

	static int enable_dog(void)
	{
		u32 v;

		if (snapdog_regp == NULL) {
			snapdog_regp = ioremap(0xf1000000, 128*1024);

			/* Set WDI GPIO as output */
			v = readl(snapdog_regp + 0x10104);
			v &= ~(0x1 << GPIO_WDT);
			writel(v, snapdog_regp + 0x10104);

			poke_the_dog();
			poke_the_dog();
		}
		return 0;
	}
	static inline void the_dog_is_dead(void) {}

	core_initcall(enable_dog);

	#define HAS_HW_SERVICE 1
#endif

#if defined(CONFIG_DTB_MT7621_EX15)
	#include <linux/io.h>
	/*
	 * We need to be able to start kicking the dog early in kernel
	 * boot, so we setup the GPIO as soon as possble. We avoid using
	 * the kernels own GPIO services - they are setup up too late
	 * in the boot process.
	 */
	#define WDT_GPIO_CTRL		0x600
	#define WDT_GPIO_DATA		0x620
	#define WDT_GPIO_PIN		29 /* GPIO29 */

	static int wdt_state = 0;
	static void *snapdog_regp;

	static void poke_the_dog(void)
	{
		u32 v;

		if (snapdog_regp) {
			/* Toggle WDT value */
			v = readl(snapdog_regp + WDT_GPIO_DATA);
			if (wdt_state++ & 0x1)
				v &= ~(0x1 << WDT_GPIO_PIN);
			else
				v |= (0x1 << WDT_GPIO_PIN);
			writel(v, snapdog_regp + WDT_GPIO_DATA);
		}
	}

	static int enable_dog(void)
	{
		u32 v;

		if (snapdog_regp == NULL) {
			snapdog_regp = ioremap(0x1e000000, 2*1024*1024);

			/* Set WDT GPIO as output */
			v = readl(snapdog_regp + WDT_GPIO_CTRL);
			v |= 0x1 << WDT_GPIO_PIN;
			writel(v, snapdog_regp + WDT_GPIO_CTRL);

			poke_the_dog();
			poke_the_dog();
		}
		return 0;
	}
	static inline void the_dog_is_dead(void) {}

	core_initcall(enable_dog);

	#define HAS_HW_SERVICE 1
#endif /* CONFIG_DTB_MT7621_EX15 */


#ifndef HAS_HW_SERVICE
	static inline void enable_dog(void) {}
	static inline void poke_the_dog(void) {}
	static inline void the_dog_is_dead(void)
	{
		machine_restart(NULL);
		printk(KERN_CRIT "snapdog: reboot failed!.\n");
	}
#endif

/****************************************************************************/

static unsigned long snapdog_last = 0;
static unsigned long snapdog_next = 0;
static int           snapdog_service_required = 0;
static unsigned long snapdog_busy = 0;
static int           snapdog_kernel = 0;
static int           snapdog_timeout = 60;
static int           snapdog_ltimeout = 300;
static int           snapdog_use_long_timeout = 0;
static int           snapdog_quiet = 0;
static int           snapdog_warned = 0;
static int           snapdog_stackdump = 64;
static int           snapdog_reboot_requested = 0;


module_param(snapdog_kernel, int, 0);
MODULE_PARM_DESC(snapdog_kernel,
		"Watchdog is kernel only (userland servicing not required)");

module_param(snapdog_timeout, int, 0);
MODULE_PARM_DESC(snapdog_timeout,
		"Watchdog timeout for user service in seconds");

module_param(snapdog_ltimeout, int, 0);
MODULE_PARM_DESC(snapdog_ltimeout,
		"Watchdog 'long' timeout for user service in seconds");

module_param(snapdog_stackdump, int, 0);
MODULE_PARM_DESC(snapdog_stackdump,
		"Number of long words to dump from the stack");

/****************************************************************************/
/*
 * a really dumb stack dump,  we may need better on some platforms
 * at least this one is implemented,  unlike dump_stack which is largely
 * just a stub :-(
 */

static void snapdog_show_stack(struct pt_regs *regs)
{
	unsigned long i;
	unsigned long *addr = &i;

	printk("Kernel stack:");
	for (i = 0; i < snapdog_stackdump; i++) {
		if (i % 4 == 0)
			printk("\n%08lx:", (unsigned long) addr);
		printk(" 0x%08lx", *addr++);
	}
	printk("\n");
}

/****************************************************************************/
/*
 *	Because we need to service this guy from deep in other more critical
 *	code,  we export a function to do this that we can call where
 *	appropriate.
 *
 *	Also the watchdog never stops,  it is always running.  We must service
 *	it until we are opened,  then we stop servicing it if we are not looked
 *	after appropriately.
 *
 *	I know there are much more clever ways to code the following,  but then
 *	who would understand it,  let alone know it did the right thing when
 *	jiffies wraps ;-)
 */

void
snapdog_service(void)
{
	struct pt_regs *regs;
	int the_dog_is_alive = 0;

	if (snapdog_kernel) {
		the_dog_is_alive = 1;
	} else if (!snapdog_service_required) {
		the_dog_is_alive = 1;
	} else if (snapdog_next < snapdog_last) {
		if (jiffies < snapdog_next || jiffies >= snapdog_last)
			the_dog_is_alive = 1;
	} else if (jiffies >= snapdog_last && jiffies < snapdog_next) {
		the_dog_is_alive = 1;
	}

	if (the_dog_is_alive)
		poke_the_dog();
	else if (!snapdog_warned) {
		snapdog_warned = 1;
		printk(KERN_CRIT "snapdog: expired, allowing system reboot.\n");
		regs = get_irq_regs();
		if (regs) {
			show_regs(regs);
			snapdog_show_stack(regs);
		}
		the_dog_is_dead();
	}
}

EXPORT_SYMBOL(snapdog_service);

/****************************************************************************/
/*
 * bump the userland expiry
 */

static inline void
snapdog_user_service(void)
{
	if (snapdog_reboot_requested)
		return;

	snapdog_last = jiffies;
	if (snapdog_use_long_timeout)
		snapdog_next = snapdog_last + HZ * snapdog_ltimeout;
	else
		snapdog_next = snapdog_last + HZ * snapdog_timeout;
	snapdog_warned = 0;
}

/****************************************************************************/
/*
 *	Allow only one person to hold it open
 */

static int
snapdog_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &snapdog_busy))
		return -EBUSY;

	/* Activate timer */
	snapdog_service_required = 1;
	if (snapdog_use_long_timeout) {
		/* Opening reverts to using short timeouts */
		snapdog_use_long_timeout = 0;

		if (!snapdog_quiet) {
			printk(KERN_INFO "snapdog: now using short timeouts.\n");
		}
	}
	snapdog_user_service();

	if (!snapdog_quiet) {
		printk(KERN_INFO "snapdog: user servicing enabled (short=%d,long=%d).\n",
				snapdog_timeout, snapdog_ltimeout);
	}


	/* Opening turns off quiet mode */
	snapdog_quiet = 0;

	
	return 0;
}

/****************************************************************************/

static int
snapdog_release(struct inode *inode, struct file *file)
{
	if (!snapdog_quiet) {
		if (!snapdog_service_required) {
			printk(KERN_INFO
					"snapdog: disabled user servicing of watchdog timer.\n");
		} else if (snapdog_use_long_timeout) {
			printk(KERN_CRIT
					"snapdog: device closed, watchdog will reboot!\n");
		}
	}
	clear_bit(0, &snapdog_busy);
	return 0;
}

/****************************************************************************/

static ssize_t
snapdog_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	/*  Can't seek (pwrite) on this device  */
	if (*ppos != file->f_pos)
		return -ESPIPE;

	/*
	 *	Refresh the timer.
	 */
	if (len) {
		size_t i;

		for (i = 0; i != len; i++) {
			char c;
			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V') {
				snapdog_service_required = 0;
			}
			else if (c == 'T') {
				if (!snapdog_quiet) {
					printk(KERN_INFO "snapdog: now using long timeouts.\n");
				}
				snapdog_use_long_timeout = 1;
			}
			else if (c == 'Q') {
				/* Go quiet */
				snapdog_quiet = 1;
			}
		}
		snapdog_user_service();
		return 1;
	}
	return 0;
}

/****************************************************************************/

static long
snapdog_ioctl(
	struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	static struct watchdog_info ident = {
		.options = WDIOF_MAGICCLOSE,
		.identity = "HW/SW Watchdog for SnapGear",
	};

	switch (cmd) {
	default:
		return(-ENOIOCTLCMD);

	case WDIOC_GETSUPPORT:
		if (copy_to_user((struct watchdog_info __user *) arg, &ident, sizeof(ident)))
			return -EFAULT;
		return(0);

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return(put_user(0, (int __user *) arg));

	case WDIOC_KEEPALIVE:
		snapdog_user_service();
		return(0);
	}
}

static int
snapdog_reboot(
	struct notifier_block *n,
	unsigned long code,
	void *x)
{
	if (code == SYS_DOWN || SYS_HALT || SYS_POWER_OFF) {
		snapdog_use_long_timeout = 0;
		snapdog_service_required = 1;
		snapdog_kernel = 0;
		snapdog_user_service();
		snapdog_reboot_requested = 1;
	}

	return NOTIFY_DONE;
}

/****************************************************************************/

static struct file_operations snapdog_fops = {
	.owner		= THIS_MODULE,
	.write		= snapdog_write,
	.unlocked_ioctl	= snapdog_ioctl,
	.open		= snapdog_open,
	.release	= snapdog_release,
};


static struct miscdevice snapdog_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &snapdog_fops,
};

static struct notifier_block reboot = {
	.notifier_call = snapdog_reboot,
	.priority = INT_MAX, /* before any real devices */
};

/****************************************************************************/

static const char banner[] __initdata =
	KERN_INFO "snapdog: HW/SW watchdog timer for SnapGear/Others\n";

static int __init
watchdog_init(void)
{
	int ret;

	enable_dog();

	ret = misc_register(&snapdog_miscdev);
	if (ret)
		return ret;

	(void) register_reboot_notifier(&reboot);

	printk(banner);

	return 0;
}

/****************************************************************************/

static void __exit
watchdog_exit(void)
{
	misc_deregister(&snapdog_miscdev);
	unregister_reboot_notifier(&reboot);
}

/****************************************************************************/

module_init(watchdog_init);
module_exit(watchdog_exit);
MODULE_AUTHOR("David McCullough <davidm@snapgear.com>");
MODULE_DESCRIPTION("Driver for SnapGear HW/SW watchdog timer(s)");
MODULE_LICENSE("GPL");

/****************************************************************************/
