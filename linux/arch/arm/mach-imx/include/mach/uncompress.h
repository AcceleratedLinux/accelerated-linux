/*
 * uncompress.h 
 *
 * Copyright (C) 2014-2017, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _ARCH_UNCOMPRESS_H_
#define _ARCH_UNCOMPRESS_H_

#ifdef CONFIG_DEBUG_UNCOMPRESS
extern void putc(int c);
#else
static inline void putc(int c) {}
#endif
static inline void flush(void) {}
static inline void arch_decomp_setup(void) {}

#if defined(CONFIG_SOC_IMX6UL) && defined(CONFIG_SNAPDOG)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
#ifdef CONFIG_SNAPDOG_CONNECTITMINI
	/* external watchdog is on GPIO1 IO9 */
	*((volatile u32 *)(0x0209c000)) ^= 0x00000200;
#else
	/* external watchdog is on GPIO2 IO16 */
	*((volatile u32 *)(0x020a0000)) ^= 0x00010000;
#endif
}
#endif /* CONFIG_SOC_IMX6UL && CONFIG_SNAPDOG */

#ifndef ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void) {}
#endif

#endif /* _ARCH_UNCOMPRESS_H_ */
