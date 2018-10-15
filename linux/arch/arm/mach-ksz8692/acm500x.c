/****************************************************************************/

/*
 *	acm500x.c -- support code for the OpenGear/ACM500X boards
 *
 * 	(C) Copyright 2009,  Greg Ungerer <greg.ungerer@opengear.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/****************************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <asm/io.h>
#include <mach/platform.h>

static struct spi_board_info acm500x_spi_board_info[] __initdata = {
	{
		.modalias	= "lm70",
		.mode		= SPI_MODE_0 | SPI_3WIRE,
		.max_speed_hz	= 1000000,
		.bus_num	= 0,
		.chip_select	= 1,
	},	
};

static int __init acm500x_init(void)
{
	spi_register_board_info(acm500x_spi_board_info, ARRAY_SIZE(acm500x_spi_board_info));
	KS8692_WRITE_REG(KS8692_DDR_MEM_CFG, 0x00010198);
	return 0;
}

arch_initcall(acm500x_init);

