/*
 *  linux/include/asm-arm/arch-ks8692/system.h      
 *
 *  Copyright (C) 2006-2008 Micrel, Inc.
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
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/io.h>
#include <mach/platform.h>


#if 0
#ifndef CONFIG_LEDS
static int led_count = 0;
#endif
#endif

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
#if 0
#ifndef CONFIG_LEDS
	unsigned int val;

	led_count++;
	val = __raw_readl( VIO( KS8692_GPIO_DATA ));
	val &= ~0x0F00;
	val |= ~( led_count << 2 ) & 0x0F00;
	__raw_writel( val, VIO( KS8692_GPIO_DATA ));
#endif
#endif
	cpu_do_idle();
}

static void arch_reset(char mode, const char *cmd)
{
	unsigned int val;

	if (mode == 's')
		cpu_reset(0);

	/*
	 * To reset, use the watchdog timer
	 */
#if 0
	val = __raw_readl( VIO( KS8692_TIMER_CTRL )) & TIMER_TIME1_ENABLE;
#else
	/* Need to turn off timer 1 also lest old U-Boot code cannot setup the
	   clock properly.
	*/
	val = 0;
#endif
	__raw_writel( val, VIO( KS8692_TIMER_CTRL ));
	val = (10 << 8) | 0xFF;
	__raw_writel( val, VIO( KS8692_TIMER0 ));
	__raw_writel( 2, VIO( KS8692_TIMER0_PCOUNT ));
	val = __raw_readl( VIO( KS8692_TIMER_CTRL )) | TIMER_TIME0_ENABLE;
	__raw_writel( val, VIO( KS8692_TIMER_CTRL ));
}

#endif
