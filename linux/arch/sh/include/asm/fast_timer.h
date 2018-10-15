#ifndef __ASM_SH_FAST_TIMER_H
#define __ASM_SH_FAST_TIMER_H

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/clock.h>
#include <asm/snapgear.h>

#define FAST_POLL_INTR

#define FASTTIMER_IRQ   17
#define FASTTIMER_IPR_ADDR  INTC_IPRA
#define FASTTIMER_IPR_POS    2
#define FASTTIMER_PRIORITY   3

#ifdef FAST_POLL_INTR
#define TMU1_TCR_INIT	0x0020
#else
#define TMU1_TCR_INIT	0
#endif
#define TMU_TSTR_INIT	1
#define TMU1_TCR_CALIB	0x0000
#define TMU_TOCR	0xffd80000	/* Byte access */
#define TMU_TSTR	0xffd80004	/* Byte access */
#define TMU1_TCOR	0xffd80014	/* Long access */
#define TMU1_TCNT	0xffd80018	/* Long access */
#define TMU1_TCR	0xffd8001c	/* Word access */

static irqreturn_t
fast_timer_interrupt(int irq, void *dev_id)
{
	unsigned long timer_status;

	timer_status = __raw_readw(TMU1_TCR);
	timer_status &= ~0x100;
	__raw_writew(timer_status, TMU1_TCR);

	do_fast_timer();
	return IRQ_HANDLED;
}

static void fast_timer_set(void)
{
	unsigned long interval;
	struct clk *clk = clk_get(NULL, "module_clk");

#ifdef FAST_POLL_INTR
	if (clk) {
		interval = (clk_get_rate(clk)/4 + fast_timer_rate/2) / fast_timer_rate;
	} else
#endif
	interval = 0xffffffff;

	__raw_writeb(__raw_readb(TMU_TSTR) & ~0x2, TMU_TSTR); /* disable timer 1 */
	__raw_writew(TMU1_TCR_INIT, TMU1_TCR);
	__raw_writel(interval, TMU1_TCOR);
	__raw_writel(interval, TMU1_TCNT);
	__raw_writeb(__raw_readb(TMU_TSTR) | 0x2, TMU_TSTR); /* enable timer 1 */
}

static int __init fast_timer_setup(void)
{
#ifdef FAST_POLL_INTR
	static struct ipr_data fast_timer_ipr_map[] = { 
		{ FASTTIMER_IRQ, FASTTIMER_IPR_ADDR, FASTTIMER_IPR_POS, FASTTIMER_PRIORITY},
	};
	static unsigned long fast_timer_offsets[] = {
		INTC_IPRD,
	};
	static struct ipr_desc fast_timer_ipr_irq_desc = {
		.ipr_offsets	= fast_timer_offsets,
		.nr_offsets	= ARRAY_SIZE(fast_timer_offsets),
		.ipr_data	= fast_timer_ipr_map,
		.nr_irqs	= ARRAY_SIZE(fast_timer_ipr_map),
		.chip = {
			.name	= "IPR-fast-timer",
		},
	};

	register_ipr_controller(&fast_timer_ipr_irq_desc);

	if (request_irq(FASTTIMER_IRQ, fast_timer_interrupt, 0,
			"fast timer", NULL))
		return -EBUSY;
#endif

	fast_timer_rate = cpu_data->type == CPU_SH7751R ? 2000 : 1000;
	fast_timer_set();

#ifdef FAST_POLL_INTR
	printk("fast timer: %d Hz, IRQ %d\n", fast_timer_rate,
			FASTTIMER_IRQ);
#else
	printk("fast timer: %d Hz\n", fast_timer_rate);
#endif
	return 0;
}

static void __exit fast_timer_cleanup(void)
{
	__raw_writeb(__raw_readb(TMU_TSTR) & ~0x2, TMU_TSTR); /* disable timer 1 */
	free_irq(FASTTIMER_IRQ, NULL);
}

#if 0
/*
 * return the current ticks on the fast timer
 */

unsigned long fast_timer_count(void)
{
	return(__raw_readl(TMU1_TCNT));
}
#endif

#endif
