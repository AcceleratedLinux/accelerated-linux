/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2008 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <asm/mach-cavium-octeon/gpio.h>

#define	OCTEON_GPIO_BASE	0x8001070000000800
#define	OCTEON_GPIO_READ	0x8001070000000880
#define	OCTEON_GPIO_SET		0x8001070000000888
#define	OCTEON_GPIO_CLEAR	0x8001070000000890
#define	OCTEON_GPIO_INT_ACK	0x8001070000000898

unsigned long octeon_gpio_raw_read(void)
{
	return *((volatile u64 *) OCTEON_GPIO_READ);
}
EXPORT_SYMBOL_GPL(octeon_gpio_raw_read);

void octeon_gpio_raw_clear(unsigned long bits)
{
	*((volatile u64 *) OCTEON_GPIO_CLEAR) = bits;
}
EXPORT_SYMBOL_GPL(octeon_gpio_raw_clear);

void octeon_gpio_raw_set(unsigned long bits)
{
	*((volatile u64 *) OCTEON_GPIO_SET) = bits;
}
EXPORT_SYMBOL_GPL(octeon_gpio_raw_set);

void octeon_gpio_raw_config(int line, int type)
{
	if ((line >= 0) && (line <= 15))
		*((volatile u64 *) (OCTEON_GPIO_BASE + (line*8))) = type;
}
EXPORT_SYMBOL_GPL(octeon_gpio_raw_config);

void octeon_gpio_raw_interrupt_ack(int line)
{
	if ((line >= 0) && (line <= 15))
		*((volatile u64 *) OCTEON_GPIO_INT_ACK) = 0x1 << line;
}
EXPORT_SYMBOL_GPL(octeon_gpio_raw_interrupt_ack);

