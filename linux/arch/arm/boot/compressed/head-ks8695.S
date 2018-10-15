#include <asm/mach-types.h>

	.section	".start", "ax"

	mov	r7, #(MACH_TYPE_KS8695 & 0xff)
	orr	r7, r7, #(MACH_TYPE_KS8695 & 0xff00)

#ifdef CONFIG_MACH_LITE300
	mov	r7, #(MACH_TYPE_LITE300 & 0xff)
	orr	r7, r7, #(MACH_TYPE_LITE300 & 0xff00)
#endif
#ifdef CONFIG_MACH_SG310
	mov	r7, #(MACH_TYPE_SG310 & 0xff)
	orr	r7, r7, #(MACH_TYPE_SG310 & 0xff00)
#endif
#ifdef CONFIG_MACH_PFW
	mov	r7, #(MACH_TYPE_PFW & 0xff)
	orr	r7, r7, #(MACH_TYPE_PFW & 0xff00)
#endif
#ifdef CONFIG_MACH_SE4200
	mov	r7, #(MACH_TYPE_SE4200 & 0xff)
	orr	r7, r7, #(MACH_TYPE_SE4200 & 0xff00)
#endif
#ifdef CONFIG_MACH_CM4002
	mov	r7, #(MACH_TYPE_CM4002 & 0xff)
	orr	r7, r7, #(MACH_TYPE_CM4002 & 0xff00)
#endif
#ifdef CONFIG_MACH_CM4008
	mov	r7, #(MACH_TYPE_CM4008 & 0xff)
	orr	r7, r7, #(MACH_TYPE_CM4008 & 0xff00)
#endif
#ifdef CONFIG_MACH_CM41XX
	mov	r7, #(MACH_TYPE_CM41XX & 0xff)
	orr	r7, r7, #(MACH_TYPE_CM41XX & 0xff00)
#endif
#ifdef CONFIG_MACH_IM4004
	mov	r7, #(MACH_TYPE_IM4004 & 0xff)
	orr	r7, r7, #(MACH_TYPE_IM4004 & 0xff00)
#endif

