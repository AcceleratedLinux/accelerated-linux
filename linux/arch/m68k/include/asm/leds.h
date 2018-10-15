/*
 * linux/include/asm-m68knommu/leds.h
 *
 * Copyright (c) 2008 Arturus Networks Inc.
 *               by Oleksandr G Zhadan <www.ArcturusNetworks.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 */

#ifndef _ASM_M68KNOMMU_LEDS_H
#define _ASM_M68KNOMMU_LEDS_H

typedef enum {
	led_null,
	led_idle_start,
	led_idle_end,
	led_start,
	led_stop,
	led_led0_init,
	led_led0_exit,
	led_led0_on,
	led_led0_off,
	led_led1_init,
	led_led1_exit,
	led_led1_on,
	led_led1_off,
	led_led2_init,
	led_led2_exit,
	led_led2_on,
	led_led2_off,
	led_led3_init,
	led_led3_exit,
	led_led3_on,
	led_led3_off,
	led_led4_init,
	led_led4_exit,
	led_led4_on,
	led_led4_off,
	led_led5_init,
	led_led5_exit,
	led_led5_on,
	led_led5_off,
	led_led6_init,
	led_led6_exit,
	led_led6_on,
	led_led6_off,
	led_led7_init,
	led_led7_exit,
	led_led7_on,
	led_led7_off,
	led_panic,
} led_event_t;

/* Use this routine to handle LEDs */

#if defined(CONFIG_NEW_LEDS)
extern void (*leds_event) (led_event_t);	/* Mostly for the kernel */
#else
#define leds_event(e)
#endif

#endif				/* _ASM_M68KNOMMU_LEDS_H */
