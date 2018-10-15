/*
 *  Accelerated Concepts NetReach board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define NETREACH_UB_GPIO_LED_WLAN		0
#define NETREACH_UB_GPIO_LED_USB		1
#define NETREACH_UB_GPIO_LED_LAN		13
#define NETREACH_UB_GPIO_LED_WAN		17
#define NETREACH_UB_GPIO_LED_WPS		27

#define NETREACH_UB_GPIO_BTN_RESET	11
#define NETREACH_UB_GPIO_BTN_WPS		12

#define NETREACH_UB_GPIO_USB_POWER	26

#define NETREACH_UB_KEYS_POLL_INTERVAL	20	/* msecs */
#define NETREACH_UB_KEYS_DEBOUNCE_INTERVAL	(3 * NETREACH_UB_KEYS_POLL_INTERVAL)

#define NETREACH_UB_MAC0_OFFSET		0x0006
#define NETREACH_UB_MAC1_OFFSET		0x0000
#define NETREACH_UB_CALDATA_OFFSET	0x1000

/*
 * Define our flash layout.
 */
static struct mtd_partition netreach_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x040000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "u-boot-env",
		.offset		= 0x040000,
		.size		= 0x010000,
	}, {
		.name		= "config",
		.offset		= 0x050000,
		.size		= 0x020000,
	}, {
		.name		= "rootfs",
		.offset		= 0x070000,
		.size		= 0x5e0000,
	}, {
		.name		= "kernel",
		.offset		= 0x650000,
		.size		= 0x1a0000,
	}, {
		.name		= "image",
		.offset		= 0x070000,
		.size		= 0x780000,
	}, {
		.name		= "art",
		.offset		= 0x7F0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}
};

static struct flash_platform_data netreach_flash_data = {
	.parts		= netreach_partitions,
	.nr_parts	= ARRAY_SIZE(netreach_partitions),
};

static void __init netreach_ub_gpio_setup(void)
{
	u32 t;

	t = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);

	gpio_request(NETREACH_UB_GPIO_USB_POWER, "USB power");
	gpio_direction_output(NETREACH_UB_GPIO_USB_POWER, 1);
}

static void __init netreach_ub_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	netreach_ub_gpio_setup();

	ath79_register_m25p80(&netreach_flash_data);

	ath79_init_mac(ath79_eth1_data.mac_addr,
			art + NETREACH_UB_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr,
			art + NETREACH_UB_MAC1_OFFSET, 0);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(art + NETREACH_UB_CALDATA_OFFSET, NULL);
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_NETREACH, "NETREACH", "Accelerated Concepts NetReach",
	     netreach_ub_setup);
