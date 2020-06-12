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
static const u32 bankaddr[] = {
	0,
	0x0209c000,
	0x020a0000,
	0x020a4000,
	0x020a8000,
	0x020ac000,
};
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	/* external GPIO actioned watchdog */
	const u32 addr = bankaddr[CONFIG_SNAPDOG_GPIO_BANK];
	*((volatile u32 *) addr) ^= (1 << CONFIG_SNAPDOG_GPIO_BIT);
}
#endif /* CONFIG_SOC_IMX6UL && CONFIG_SNAPDOG */

#ifndef ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void) {}
#endif

#endif /* _ARCH_UNCOMPRESS_H_ */
