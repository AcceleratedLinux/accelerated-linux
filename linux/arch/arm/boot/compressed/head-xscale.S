/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linux/arch/arm/boot/compressed/head-xscale.S
 *
 * XScale specific tweaks.  This is merged into head.S by the linker.
 *
 */

#include <linux/linkage.h>
#include <asm/mach-types.h>

		.section        ".start", "ax"

__XScale_start:

		@ Preserve r8/r7 i.e. kernel entry values

		@ Data cache might be active.
		@ Be sure to flush kernel binary out of the cache,
		@ whatever state it is, before it is turned off.
		@ This is done by fetching through currently executed
		@ memory to be sure we hit the same cache.
		bic	r2, pc, #0x1f
		add	r3, r2, #0x10000	@ 64 kb is quite enough...
1:		ldr	r0, [r2], #32
		teq	r2, r3
		bne	1b
		mcr	p15, 0, r0, c7, c10, 4	@ drain WB
		mcr	p15, 0, r0, c7, c7, 0	@ flush I & D caches

		@ disabling MMU and caches
		mrc	p15, 0, r0, c1, c0, 0	@ read control reg
		bic	r0, r0, #0x05		@ clear DC, MMU
		bic	r0, r0, #0x1000		@ clear Icache
		mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_ARCH_IXDP425
		mov	r7, #MACH_TYPE_IXDP425
#endif

#ifdef CONFIG_MACH_IXDP465
		mov r7, #MACH_TYPE_IXDP465 & 0xff
		orr r7, r7, #MACH_TYPE_IXDP465 & 0xff00
#endif

#ifdef CONFIG_MACH_IXDPG425
		mov r7, #MACH_TYPE_IXDPG425 & 0xff
		orr r7, r7, #MACH_TYPE_IXDPG425 & 0xff00
#endif

#ifdef CONFIG_ARCH_PRPMC1100
		mov	r7, #(MACH_TYPE_PRPMC1100 & 0xff)
		orr	r7, r7, #(MACH_TYPE_PRPMC1100 & 0xff00)
#endif

#ifdef CONFIG_ARCH_ADI_COYOTE
		# Set machine ID into r7 (Coyote ID > 255 hence needs
		# two instructions to build the ID safely).
		mov r7, #MACH_TYPE_ADI_COYOTE & 0xff
		orr r7, r7, #MACH_TYPE_ADI_COYOTE & 0xff00
#endif

#ifdef CONFIG_ARCH_SE4000
		mov r7, #MACH_TYPE_SE4000 & 0xff
		orr r7, r7, #MACH_TYPE_SE4000 & 0xff00
#endif

#ifdef CONFIG_MACH_ESS710
		mov r7, #MACH_TYPE_ESS710 & 0xff
		orr r7, r7, #MACH_TYPE_ESS710 & 0xff00
#endif

#ifdef CONFIG_MACH_SG720
		mov r7, #MACH_TYPE_SG720 & 0xff
		orr r7, r7, #MACH_TYPE_SG720 & 0xff00
#endif

#ifdef CONFIG_MACH_MONTEJADE
		mov r7, #MACH_TYPE_MONTEJADE & 0xff
		orr r7, r7, #MACH_TYPE_MONTEJADE & 0xff00
#endif

#ifdef CONFIG_MACH_IVPN
		mov r7, #MACH_TYPE_IVPN & 0xff
		orr r7, r7, #MACH_TYPE_IVPN & 0xff00
#endif

#ifdef CONFIG_MACH_SE5100
		mov r7, #MACH_TYPE_SE5100 & 0xff
		orr r7, r7, #MACH_TYPE_SE5100 & 0xff00
#endif

#ifdef CONFIG_MACH_SG560
		mov r7, #MACH_TYPE_SG560 & 0xff
		orr r7, r7, #MACH_TYPE_SG560 & 0xff00
#endif

#ifdef CONFIG_MACH_SG560USB
		mov r7, #MACH_TYPE_SG560USB & 0xff
		orr r7, r7, #MACH_TYPE_SG560USB & 0xff00
#endif

#ifdef CONFIG_MACH_SG560ADSL
		mov r7, #MACH_TYPE_SG560ADSL & 0xff
		orr r7, r7, #MACH_TYPE_SG560ADSL & 0xff00
#endif

#ifdef CONFIG_MACH_SG565
		mov r7, #MACH_TYPE_SG565 & 0xff
		orr r7, r7, #MACH_TYPE_SG565 & 0xff00
#endif

#ifdef CONFIG_MACH_SG580
		mov r7, #MACH_TYPE_SG580 & 0xff
		orr r7, r7, #MACH_TYPE_SG580 & 0xff00
#endif

#ifdef CONFIG_MACH_SG590
		mov r7, #MACH_TYPE_SG590 & 0xff
		orr r7, r7, #MACH_TYPE_SG590 & 0xff00
#endif

#ifdef CONFIG_MACH_SG640
		mov r7, #MACH_TYPE_SG640 & 0xff
		orr r7, r7, #MACH_TYPE_SG640 & 0xff00
#endif

#ifdef  CONFIG_MACH_GTWX5715
		mov     r7, #(MACH_TYPE_GTWX5715 & 0xff)
		orr     r7, r7, #(MACH_TYPE_GTWX5715 & 0xff00)
#endif

