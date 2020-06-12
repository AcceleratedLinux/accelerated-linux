/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * arch/arm/mach-ixp4xx/include/mach/uncompress.h 
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 */

#ifndef _ARCH_UNCOMPRESS_H_
#define _ARCH_UNCOMPRESS_H_

#include "ixp4xx-regs.h"
#include "ixp4xx-gpio.h"
#include <asm/mach-types.h>
#include <linux/serial_reg.h>

#define TX_DONE (UART_LSR_TEMT|UART_LSR_THRE)

#if defined(CONFIG_MACH_ESS710) || defined(CONFIG_MACH_IVPN) || \
	defined(CONFIG_MACH_SG560) || defined(CONFIG_MACH_SG560USB) || \
	defined(CONFIG_MACH_SG560ADSL) || defined(CONFIG_MACH_SG565) || \
	defined(CONFIG_MACH_SG580) || defined(CONFIG_MACH_SG720) || \
	defined(CONFIG_MACH_SHIVA1100) || defined(CONFIG_MACH_SG590) || \
	defined(CONFIG_MACH_SG8100)
#define	CONSOLE_OUTPUT	0
#else
#define	CONSOLE_OUTPUT	1
#endif

volatile u32* uart_base;

static inline void putc(int c)
{
	/* Check THRE and TEMT bits before we transmit the character.
	 */
	if (CONSOLE_OUTPUT) {
		while ((uart_base[UART_LSR] & TX_DONE) != TX_DONE)
			barrier();

		*uart_base = c;
	}
}

static void flush(void)
{
}

static __inline__ void __arch_decomp_setup(unsigned long arch_id)
{
	/*
	 * Some boards are using UART2 as console
	 */
	if (machine_is_adi_coyote() || machine_is_gtwx5715() ||
	    machine_is_gateway7001() || machine_is_wg302v2() ||
	    machine_is_devixp() || machine_is_miccpt() || machine_is_mic256())
		uart_base = (volatile u32*) IXP4XX_UART2_BASE_PHYS;
	else
		uart_base = (volatile u32*) IXP4XX_UART1_BASE_PHYS;
}

/*
 * arch_id is a variable in decompress_kernel()
 */
#define arch_decomp_setup()	__arch_decomp_setup(arch_id)

#define ARCH_HAS_DECOMP_WDOG
#if defined(CONFIG_MACH_SG560) || defined(CONFIG_MACH_SG580) || \
    defined(CONFIG_MACH_ESS710) || defined(CONFIG_MACH_SG720) || \
    defined(CONFIG_MACH_SG590) || defined(CONFIG_MACH_IVPN)
#define arch_decomp_wdog() \
	*((volatile u32 *)(IXP4XX_GPIO_BASE_PHYS+IXP4XX_GPIO_GPOUTR_OFFSET)) ^= 0x00004000
#elif defined(CONFIG_MACH_SG560USB) || defined(CONFIG_MACH_SG560ADSL) || \
      defined(CONFIG_MACH_SG565) || defined(CONFIG_MACH_SHIVA1100)
#define arch_decomp_wdog() \
	*((volatile u8 *) (0x50000000 + (7 * 16*1024*1024))) = 0
#else
#define arch_decomp_wdog()
#endif

#endif
