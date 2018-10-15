/*
 * arch/arm/mach-ksz8692/time.c
 *
 * Micrel Pegasus clocksource, clockevents, and timer interrupt handlers.
 * Copyright (c) 2008 Micrel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef CONFIG_GENERIC_CLOCKEVENTS
#include <linux/clocksource.h>
#endif
#include <linux/clockchips.h>


#ifdef CONFIG_GENERIC_CLOCKEVENTS

#ifdef CONFIG_LEDS
#include <asm/leds.h>

static inline void do_leds(void)
{
	static unsigned int count = HZ/2;

	if (--count == 0) {
		count = HZ/2;
		leds_event(led_timer);
	}
}
#else
#define	do_leds()
#endif

static inline void disable_timer1 ( void )
{
	unsigned long reg;

	reg = KS8692_READ_REG( KS8692_TIMER_CTRL );
	reg &= ~( TIMER_TIME1_ENABLE );
	KS8692_WRITE_REG( KS8692_TIMER_CTRL, reg );
}  /* disable_timer1 */


static inline void enable_timer1 ( void )
{
	unsigned long reg;

	reg = KS8692_READ_REG( KS8692_TIMER_CTRL );
	reg |= TIMER_TIME1_ENABLE;
	KS8692_WRITE_REG( KS8692_TIMER_CTRL, reg );
}  /* enable_timer1 */


static inline void setup_timer1 (
	unsigned long interval )
{
	unsigned long pcount;
	unsigned long value;

	pcount = interval >> 1;
	value = interval - pcount;
	KS8692_WRITE_REG( KS8692_TIMER1, value );
	KS8692_WRITE_REG( KS8692_TIMER1_PCOUNT, pcount );
	enable_timer1();
}  /* setup_timer1 */


static int
pegasus_set_next_event (unsigned long delta, struct clock_event_device *dev)
{
	setup_timer1(delta);
	return 0;
}

static void
pegasus_set_mode (enum clock_event_mode mode, struct clock_event_device *dev)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		__raw_writel( TIMER_DATA_VALUE, VIO( KS8692_TIMER1 ));
		__raw_writel( TIMER_PULSE_VALUE, VIO( KS8692_TIMER1_PCOUNT ));
		enable_timer1();
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		disable_timer1();
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		/* initializing, released, or preparing for suspend */
		disable_timer1();
		break;
	}
}

static struct clock_event_device ckevt_pegasus = {
	.name		= "pegasus",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.rating		= 200,
	.cpumask	= CPU_MASK_CPU0,
	.set_next_event	= pegasus_set_next_event,
	.set_mode	= pegasus_set_mode,
};


static irqreturn_t
pegasus_timer_interrupt (int irq, void *dev_id)
{
	struct clock_event_device *c = dev_id;

        __raw_writel( KS8692_INTMASK_TIMERINT1, VIO( KS8692_INT_STATUS2 ));

#ifdef CONFIG_TICK_ONESHOT
	if (c->mode == CLOCK_EVT_MODE_ONESHOT) {
		disable_timer1();
		c->event_handler(c);
	}
	else if (c->mode == CLOCK_EVT_MODE_PERIODIC) {
		c->event_handler(c);
	}
#else
	c->event_handler(c);
	do_leds();
#endif

	return IRQ_HANDLED;
}

static struct irqaction pegasus_timer_irq = {
	.name		= "timer",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= pegasus_timer_interrupt,
	.dev_id		= &ckevt_pegasus,
};
#endif


static cycle_t pegasus_read_cycle (void)
{
	return( KS8692_READ_REG( KS8692_TIMER1_COUNTER ));
}

static struct clocksource cksrc_pegasus = {
	.name           = "pegasus",
	.rating         = 200,
	.read           = pegasus_read_cycle,
	.mask           = CLOCKSOURCE_MASK(32),
	.shift          = 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};


void __init pegasus_timer_init (void)
{
	cksrc_pegasus.mult =
		clocksource_khz2mult(ksz_system_bus_clock,
			cksrc_pegasus.shift);
	clocksource_register(&cksrc_pegasus);

#ifdef CONFIG_GENERIC_CLOCKEVENTS
	__raw_writel( 0xFF00, VIO( KS8692_GPIO_MODE ));
	__raw_writel( 0x7000, VIO( KS8692_GPIO_DATA ));

	/*
	 * Initialise to a known state (all timers off)
	 */
        __raw_writel( 0, VIO( KS8692_TIMER_CTRL ));

	/*
	 * Make irqs happen for the system timer
	 */
	setup_irq((KS8692_INT_TIMERINT1 + LOW_IRQS), &pegasus_timer_irq);

	system_time_tick = KS8692_READ_REG( 0xE424 );

	ckevt_pegasus.mult =
		div_sc(TICKS_PER_uSEC * 1000000, NSEC_PER_SEC,
			ckevt_pegasus.shift);
	ckevt_pegasus.max_delta_ns =
		clockevent_delta2ns(0xffffffff, &ckevt_pegasus);
	ckevt_pegasus.min_delta_ns =
		clockevent_delta2ns(0x200, &ckevt_pegasus);
	clockevents_register_device(&ckevt_pegasus);
#endif
}

#endif
