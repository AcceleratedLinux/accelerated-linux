/*
 * Support for AcceleratedConcepts 5400-RM
 * David McCullough <david.mccullough@accelerated.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mv643xx_eth.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include "common.h"
#include "kirkwood.h"
#include "board.h"

#if defined(CONFIG_MACH_6330MX)
#define	GPIO_USB_PWR	50 /* gpio 50 is power enable */
#define	GPIO_USB_OVC	31 /* gpio 31 is over current */
#define	USB_PWR_ON	GPIO_ON
#define	USB_PWR_OFF	GPIO_OFF
#define	USB_OVC_TRUE	GPIO_ON
#elif defined(CONFIG_MACH_6350SR)
#define	GPIO_USB_PWR	4  /* gpio 4 is power enable */
#define	GPIO_USB_OVC	31 /* gpio 31 is over current */
#define	USB_PWR_ON	GPIO_ON
#define	USB_PWR_OFF	GPIO_OFF
#define	USB_OVC_TRUE	GPIO_ON
#else
#define	GPIO_USB_PWR	15 /* gpio 15 is power enable */
#define	GPIO_USB_OVC	17 /* gpio 17 is over current */
#define	USB_PWR_ON	GPIO_OFF
#define	USB_PWR_OFF	GPIO_ON
#define	USB_OVC_TRUE	GPIO_ON
#endif

#define	GPIO_ON		0
#define	GPIO_OFF	1

/*
 * Check the status of the USB over current every 20 millseconds.
 * If it's high we turn it off for 5 seconds and try again.
 */

static void accelerated_usboc_handler(struct timer_list *t)
{
	static int count_down = 0;
	static DEFINE_TIMER(accelerated_usboc_timer, accelerated_usboc_handler);

	if (count_down > 0) {
		count_down--;
		if (count_down == 0) {
			/* Turn on USB power */
			gpio_set_value(GPIO_USB_PWR, USB_PWR_ON);
			printk(KERN_ERR "accelerated: Re-enabling USB power after timeout\n");
		}
	} else if (gpio_get_value(GPIO_USB_OVC) == USB_OVC_TRUE) {
		/* Turn of USB power */
		gpio_set_value(GPIO_USB_PWR, USB_PWR_OFF);
		count_down = 50;
		printk(KERN_ERR "accelerated: USB disabled due to OC\n");
	}

	mod_timer(&accelerated_usboc_timer, jiffies + msecs_to_jiffies(20));
}

static __init int accelerated_usbpwr_init(void)
{
	int rc;

	/* USB power and OC checking */

	rc = gpio_request(GPIO_USB_PWR, "USB Power");
	if (!rc)
		rc = gpio_direction_output(GPIO_USB_PWR, USB_PWR_ON);
	if (rc) {
		printk(KERN_ERR "accelerated: can't set up USB Power GPIO: %d\n", rc);
		return rc;
	}
	rc = gpio_request(GPIO_USB_OVC, "USB OVC");
	if (rc)
		rc = gpio_direction_input(GPIO_USB_OVC);
	if (rc) {
		printk(KERN_ERR "accelerated: can't set up USB Over Current: %d\n", rc);
		return rc;
	}
	gpiod_export(gpio_to_desc(GPIO_USB_PWR), 0);
	gpiod_export(gpio_to_desc(GPIO_USB_OVC), 0);
	mdelay(20);
	/* start the timer, do first check */
	accelerated_usboc_handler(0);
	printk(KERN_INFO "accelerated: USB Power Control configured.\n");
	return 0;
}
late_initcall(accelerated_usbpwr_init);


#if defined(CONFIG_MACH_5400_RM_DT) || defined(CONFIG_MACH_6300_EX_DT)

#define	GPIO_CELL_DISABLE   37 /* gpio 37 cell disable */
#define	GPIO_CELL_RESET     38 /* gpio 38 is cell reset */

static __init int accelerated_cell_init(void)
{
	int rc;

	rc = gpio_request(GPIO_CELL_DISABLE, "Cell Disable");
	if (!rc)
		rc = gpio_direction_output(GPIO_CELL_DISABLE, 1);
	if (rc) {
		printk(KERN_ERR "accelerated: can't set up Cell Disable: %d\n", rc);
		return rc;
	}
	gpiod_export(gpio_to_desc(GPIO_CELL_DISABLE), 0);

	rc = gpio_request(GPIO_CELL_RESET, "Cell Reset");
	if (!rc)
		rc = gpio_direction_output(GPIO_CELL_RESET, 1);
	if (rc) {
		printk(KERN_ERR "accelerated: can't set up Cell Reset: %d\n", rc);
		return rc;
	}
	gpiod_export(gpio_to_desc(GPIO_CELL_RESET), 0);

	printk(KERN_INFO "accelerated: CELL Control configured.\n");
	return 0;
}
late_initcall(accelerated_cell_init);

#endif /* CONFIG_MACH_5400_RM_DT || CONFIG_MACH_6300_EX_DT */


#if defined(CONFIG_MACH_5400_RM_DT)

#define	GPIO_BUZZER     44 /* gpio 44 is buzzer on/off (inverted) */
static __init int accelerated_buzzer_init(void)
{
	int rc;

	if (!of_machine_is_compatible("accelerated,5400-rm"))
		return 0;

	rc = gpio_request(GPIO_BUZZER, "Cell Disable");
	if (!rc)
		rc = gpio_direction_output(GPIO_BUZZER, 0);
	if (rc) {
		printk(KERN_ERR "accelerated: can't set up Buzzer: %d\n", rc);
		return rc;
	}
	gpiod_export(gpio_to_desc(GPIO_BUZZER), 0);
	return 0;
}
late_initcall(accelerated_buzzer_init);

#endif /* ! CONFIG_MACH_5400_RM_DT */

#if defined(CONFIG_MACH_5400_RM_DT) || defined(CONFIG_MACH_6300_EX_DT)

void __init accelerated_init(void)
{
	void __iomem *rp;

	rp = ioremap(KIRKWOOD_REGS_PHYS_BASE, 0x100000);
	/* set CMOS PAD I/O volatge to 1.8V */
	writel(readl(rp + 0x100e0) | 0x8080, rp + 0x100e0);
	iounmap(rp);
}

#endif /* CONFIG_MACH_5400_RM_DT || CONFIG_MACH_6300_EX_DT */
