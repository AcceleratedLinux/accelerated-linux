/*
 * amdfch.c -- BIOS flash mapping support AMD FCH
 *
 * (C) Copyright 2015, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

static struct mtd_partition amdfch_partitions[] = {
	{
		.name	= "AMD FCH Flash (ALL)",
		.offset	= 0,
	},
	{
		.name	= "Environment",
		.offset	= 0x00000000,
		.size	= 0x00010000,
	},
	{
		.name	= "BIOS",
		.offset	= 0x00010000,
	},
};

static struct flash_platform_data amdfch_flash_data = {
	.parts		= amdfch_partitions,
	.nr_parts	= (sizeof(amdfch_partitions)/sizeof(amdfch_partitions[0])),
};

static struct spi_board_info amdfch_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
		.platform_data	= &amdfch_flash_data,
	},
};

static int __init amdfch_init(void)
{
	printk("AMDFCH: MTD BIOS flash support\n");

	spi_register_board_info(amdfch_spi_info, 1);
	return 0;
}

module_init(amdfch_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_DESCRIPTION("AMD FCH BIOS Flash support");
