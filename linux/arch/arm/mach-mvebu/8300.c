/*
 * 8300.c -- special support for the Accelerated 8300
 *
 * (C) Copyright 2013, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define GPIO_WDISABLE0	2
#define GPIO_WDISABLE1	3
#define GPIO_USB1_PWR	4
#define GPIO_USB2_PWR	31
#define GPIO_USB3_PWR	47
#define GPIO_PCIe_RESET	48

static int ac8300_pcie_init(void)
{
	int rc;

	printk("AC8300: initializing PCIe device control lines\n");

	/* Configure W_DISABLE0 (WWAN port) */
	rc = gpio_request(GPIO_WDISABLE0, "W_Disable0");
	gpio_direction_output(GPIO_WDISABLE0, 1);
	gpio_set_value(GPIO_WDISABLE0, 1);
	gpio_export(GPIO_WDISABLE0, 0);

	/* Configure W_DISABLE1 (WIFI port) */
	gpio_request(GPIO_WDISABLE1, "W_Disable1");
	gpio_direction_output(GPIO_WDISABLE1, 1);
	gpio_set_value(GPIO_WDISABLE1, 1);
	gpio_export(GPIO_WDISABLE1, 0);

	/* Configure PCIe RESET (WIFI port) */
	gpio_request(GPIO_PCIe_RESET, "PCIe RESET");
	gpio_direction_output(GPIO_PCIe_RESET, 1);
	gpio_set_value(GPIO_PCIe_RESET, 1);
	gpio_export(GPIO_PCIe_RESET, 0);

	return 0;
}
late_initcall(ac8300_pcie_init);

static int ac8300_usb_powerup(void)
{
	printk("AC8300: powering up external USB ports\n");

	/* Enable power to external USB connectors */
	gpio_request(GPIO_USB1_PWR, "USB1 Power");
	gpio_direction_output(GPIO_USB1_PWR, 1);
	gpio_set_value(GPIO_USB1_PWR, 0);
	gpio_export(GPIO_USB1_PWR, 0);

	/* Stager the USB power ups */
	mdelay(50);

	gpio_request(GPIO_USB2_PWR, "USB2 Power");
	gpio_direction_output(GPIO_USB2_PWR, 1);
	gpio_set_value(GPIO_USB2_PWR, 0);
	gpio_export(GPIO_USB2_PWR, 0);

	mdelay(50);

	gpio_request(GPIO_USB3_PWR, "USB3 Power");
	gpio_direction_output(GPIO_USB3_PWR, 1);
	gpio_set_value(GPIO_USB3_PWR, 0);
	gpio_export(GPIO_USB3_PWR, 0);

	return 0;
}
late_initcall_sync(ac8300_usb_powerup);

