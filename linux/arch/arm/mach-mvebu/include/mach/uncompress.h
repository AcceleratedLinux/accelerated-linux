/*
 * uncompress.h 
 *
 * Copyright (C) 2014, Greg Ungerer <greg.ungerer@accelerated.com>
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

#if defined(CONFIG_MACH_8300) || defined(CONFIG_MACH_6300CX) || \
    defined(CONFIG_MACH_6350SR) || defined(CONFIG_MACH_6330MX)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xd0018140)) ^= 0x00400000;
}
#endif /* CONFIG_MACH_8300 || CONFIG_MACH_6300CX */

#if defined(CONFIG_MACH_CM71xx)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xd0018140)) ^= 0x00004000;
}
#endif /* CONFIG_MACH_CM71xx */

#if defined(CONFIG_MACH_ACM700x)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xd0018140)) |= 0x10000000;
	*((volatile u32 *)(0xd0018140)) ^= 0x20000000;
}
#endif /* CONFIG_MACH_ACM700x */

#if defined(CONFIG_MACH_IM72xx)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xf1010100)) ^= 0x00000080;
}
#endif /* CONFIG_MACH_IM72xx */

#if defined(CONFIG_MACH_ARMADA_38X)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xf1018140)) ^= 0x00200000;
}
#endif /* CONFIG_MACH_ARMADA_38X */

#if defined(CONFIG_MACH_5400_RM_DT)
#define ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void)
{
	*((volatile u32 *)(0xf1010140)) ^= 0x00000010;
}
#endif /* CONFIG_MACH_5400_RM_DT */

#ifndef ARCH_HAS_DECOMP_WDOG
static inline void arch_decomp_wdog(void) {}
#endif

#endif /* _ARCH_UNCOMPRESS_H_ */
