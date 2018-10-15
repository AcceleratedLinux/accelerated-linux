/*
 *  linux/arch/arm/mach-ks8692/leds.c
 *
 *  KSZ8692 LED control routines
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2007 Micrel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
#include <linux/smp.h>
#include <linux/spinlock.h>
#endif

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/leds.h>


static int saved_leds;

static void centaur_leds_event(led_event_t ledevt)
{
	unsigned long flags;
	unsigned int update_leds;
	unsigned int val;
	
	local_irq_save(flags);

	update_leds = 1;

	switch (ledevt) {
	case led_idle_start:
		saved_leds |= 0x0100;
		break;

	case led_idle_end:
		saved_leds &= ~0x0100;
		break;

	case led_timer:
		saved_leds ^= 0x0200;
		break;

	case led_red_on:
		saved_leds &= ~0x0400;
		break;

	case led_red_off:
		saved_leds |= 0x0400;
		break;

	default:
		update_leds = 0;
		break;
	}

	if (update_leds) {
		val = __raw_readl( VIO( KS8692_GPIO_DATA ));
		val &= ~0x0F00;
		val |= saved_leds;
		__raw_writel( val, VIO( KS8692_GPIO_DATA ));
	}
	local_irq_restore(flags);
}

static int __init leds_init(void)
{
	leds_event = centaur_leds_event;

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
core_initcall(leds_init);
#else
__initcall(leds_init);
#endif
