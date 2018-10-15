/*
 *	fiq.c -- some simple FIQ handling for debug
 *
 *	(C) Copyright 2007, Greg Ungerer <gerg@snapgear.com>
 *
 *	A few bits of code in here taken from arch/arm/kernel/fiq.c which is:
 *	Copyright (C) 1998 Russell King
 *	Copyright (C) 1998, 1999 Phil Blundell
 */

/*
 * This fiq code is designed to turn the factory "ERASE" button on
 * SG products into an NMI trap. So if you are trying to debug a kernel
 * lockup this could be a great help.
 *
 * The erase button GPIO line is set to be an FIQ instead of
 * the usual IRQ. This has highest CPU priority - and will trap even if
 * interrupts are locked out, of if stuck in an interrupt handler.
 * (Obviously it can't recover from a bus lock up or other seriaous
 * hardware hang situation).
 *
 * Once this driver runs it takes over the erase button IRQ.
 * Once the button is pushed the FIQ handler will print the saved PC
 * at the time of the FIQ. It would be possible to print all registers,
 * but I haven't bother to implement this just yet.
 *
 * Return from this FIQ handler is not 100% clean, so don't expect a
 * reliably running system on return. This is not an issue for the types
 * problems this is designed to help debug :-)
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/pgalloc.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#define	FIQ_VECTOR	(CONFIG_VECTORS_BASE + 0x1c)

static inline void unprotect_page_0(void)
{
        modify_domain(DOMAIN_USER, DOMAIN_MANAGER);
}

static inline void protect_page_0(void)
{
        modify_domain(DOMAIN_USER, DOMAIN_CLIENT);
}

void fiq_die_handler(unsigned int savepc)
{
	/*struct pt_regs r;*/
	console_verbose();
	printk("PC=0x%08x\n", savepc);
	/*die("FIQ", &r, 0);*/
}

void fiqasm(void);

void fiq(void)
{
	asm("						\n\
		.globl fiqasm				\n\
		fiqasm:					\n\
		mov	r8, #0xf0000000			\n\
		orr	r8, r8, #0x000fe000		\n\
		mov	r9, #0x41			\n\
		str	r9, [r8,#4]			\n\
							\n\
		mov	r0, lr @ save lr		\n\
							\n\
		mrs     r13, cpsr			\n\
		bic     r13, r13, #0x1f			\n\
		orr     r13, r13, #0x80 | 0x13		\n\
		msr     spsr_c, r13 @ switch to SVC_32 	\n\
							\n\
		ldr	lr, handler			\n\
		movs	pc, lr				\n\
							\n\
		subs	pc, lr, #4			\n\
							\n\
handler:	.word	fiq_die_handler			\n\
	    ");
}

static int __init fiq_init(void)
{
	u32 intmc, inten;

	printk("FIQ: installing ERASE button debug FIQ handler\n");

#if 0
	printk("CURRENT FIQ = %08x\n", *((unsigned int *) FIQ_VECTOR));
#endif

	/* Configure Erase switch as IRQ/FIQ input */
	ks8695_gpio_interrupt(KS8695_GPIO_0, IRQ_TYPE_EDGE_FALLING);

	intmc = __raw_readl(KS8695_IO_VA + 0xF0000 + 0xe200);
	printk("%s(%d): INTMC=%x\n", __FILE__, __LINE__, intmc);
	__raw_writel(intmc | 0x4, (KS8695_IO_VA + 0xF0000 + 0xe200));
	intmc = __raw_readl(KS8695_IO_VA + 0xF0000 + 0xe200);
	printk("%s(%d): INTMC=%x\n", __FILE__, __LINE__, intmc);

	inten = __raw_readl(KS8695_IO_VA + 0xF0000 + 0xe204);
	printk("%s(%d): INTEN=%x\n", __FILE__, __LINE__, inten);
	__raw_writel(inten | 0x4, (KS8695_IO_VA + 0xF0000 + 0xe204));
	inten = __raw_readl(KS8695_IO_VA + 0xF0000 + 0xe204);
	printk("%s(%d): INTEN=%x\n", __FILE__, __LINE__, inten);

	unprotect_page_0();
	memcpy(FIQ_VECTOR, fiqasm, 96);
        protect_page_0();
        flush_icache_range(FIQ_VECTOR, FIQ_VECTOR + 96);

#if 0
	printk("CURRENT FIQ = %08x\n", *((unsigned int *) FIQ_VECTOR));
#endif

	return 0;
}

static void __exit fiq_exit(void)
{
	printk("%s(%d): fiq_exit()\n", __FILE__, __LINE__);
}

module_init(fiq_init);
module_exit(fiq_exit);

