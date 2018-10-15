/* linux/include/asm-arm/arch-ks8692/debug-macro.S
 *
 * Debugging macro include header
 *
 *  Copyright (C) 2006 Micrel, Inc.
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/
#include <mach/platform.h>

	.macro	addruart,rp,rv,tmp
		ldr	\rp, =(KS_IO_BASE + KS8692_UART1_RX_BUFFER)
		ldr	\rv, =(KS_VIO_BASE + KS8692_UART1_RX_BUFFER)
	.endm

	.macro	senduart,rd,rx
		str	\rd, [\rx, #4]
	.endm

	.macro	waituart,rd,rx
1001:		ldr	\rd, [\rx, #0x14]	@ UART LINE STATUS
		tst	\rd, #1 << 5		@ UART RUTHRE - 0 when full
		beq	1001b
	.endm

	.macro	busyuart,rd,rx
1001:		ldr	\rd, [\rx, #0x14]	@ UART LINE STATUS
		tst	\rd, #1 << 6		@ UART URTE - 0 when busy
		beq	1001b
	.endm
