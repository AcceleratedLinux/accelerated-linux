/**
 * @file op_model_ppro.h
 * pentium pro / P6 model-specific MSR operations
 *
 * @remark Copyright 2002 OProfile authors
 * @remark Read the file COPYING
 *
 * @author John Levon
 * @author Philippe Elie
 * @author Graydon Hoare
 */

#include "op_x86_model.h"
#include "op_arch.h"
#include "op_msr.h"

#define NUM_COUNTERS 2
#define NUM_CONTROLS 2

#define CTR_READ(l, h, msrs, c) do {rdmsr(msrs->counters.addrs[(c)], (l), (h));} while (0)
#define CTR_WRITE(l, msrs, c) do {wrmsr(msrs->counters.addrs[(c)], -(u32)(l), -1);} while (0)
#define CTR_OVERFLOWED(n) (!((n) & (1U<<31)))

#define CTRL_READ(l, h, msrs, c) do {rdmsr((msrs->controls.addrs[(c)]), (l), (h));} while (0)
#define CTRL_WRITE(l, h, msrs, c) do {wrmsr((msrs->controls.addrs[(c)]), (l), (h));} while (0)
#define CTRL_SET_ACTIVE(n) (n |= (1<<22))
#define CTRL_SET_INACTIVE(n) (n &= ~(1<<22))
#define CTRL_CLEAR(x) (x &= (1<<21))
#define CTRL_SET_ENABLE(val) (val |= 1<<20)
#define CTRL_SET_USR(val, u) (val |= ((u & 1) << 16))
#define CTRL_SET_KERN(val, k) (val |= ((k & 1) << 17))
#define CTRL_SET_UM(val, m) (val |= (m << 8))
#define CTRL_SET_EVENT(val, e) (val |= e)

 
static void ppro_fill_in_addresses(struct op_msrs * const msrs)
{
	msrs->counters.addrs[0] = MSR_P6_PERFCTR0;
	msrs->counters.addrs[1] = MSR_P6_PERFCTR1;
	
	msrs->controls.addrs[0] = MSR_P6_EVNTSEL0;
	msrs->controls.addrs[1] = MSR_P6_EVNTSEL1;
}


static void ppro_setup_ctrs(struct op_msrs const * const msrs)
{
	uint low, high;
	int i;

	/* clear all counters */
	for (i = 0 ; i < NUM_CONTROLS; ++i) {
		CTRL_READ(low, high, msrs, i);
		CTRL_CLEAR(low);
		CTRL_WRITE(low, high, msrs, i);
	}
	
	/* avoid a false detection of ctr overflows in NMI handler */
	for (i = 0; i < NUM_COUNTERS; ++i) {
		CTR_WRITE(1, msrs, i);
	}

	/* enable active counters */
	for (i = 0; i < NUM_COUNTERS; ++i) {
		if (sysctl.ctr[i].enabled) {

			CTR_WRITE(sysctl.ctr[i].count, msrs, i);

			CTRL_READ(low, high, msrs, i);
			CTRL_CLEAR(low);
			CTRL_SET_ENABLE(low);
			CTRL_SET_USR(low, sysctl.ctr[i].user);
			CTRL_SET_KERN(low, sysctl.ctr[i].kernel);
			CTRL_SET_UM(low, sysctl.ctr[i].unit_mask);
			CTRL_SET_EVENT(low, sysctl.ctr[i].event);
			CTRL_WRITE(low, high, msrs, i);
		}
	}
}

 
static void ppro_check_ctrs(uint const cpu, 
			    struct op_msrs const * const msrs,
			    struct pt_regs * const regs)
{
	ulong low, high;
	int i;
	for (i = 0 ; i < NUM_COUNTERS; ++i) {
		CTR_READ(low, high, msrs, i);
		if (CTR_OVERFLOWED(low)) {
			op_do_profile(cpu, instruction_pointer(regs), IRQ_ENABLED(regs), i);
			CTR_WRITE(oprof_data[cpu].ctr_count[i], msrs, i);
		}
	}
}

 
static void ppro_start(struct op_msrs const * const msrs)
{
	uint low, high;
	CTRL_READ(low, high, msrs, 0);
	CTRL_SET_ACTIVE(low);
	CTRL_WRITE(low, high, msrs, 0);
}


static void ppro_stop(struct op_msrs const * const msrs)
{
	uint low, high;
	CTRL_READ(low, high, msrs, 0);
	CTRL_SET_INACTIVE(low);
	CTRL_WRITE(low, high, msrs, 0);
}


struct op_x86_model_spec const op_ppro_spec = {
	.num_counters = NUM_COUNTERS,
	.num_controls = NUM_CONTROLS,
	.fill_in_addresses = &ppro_fill_in_addresses,
	.setup_ctrs = &ppro_setup_ctrs,
	.check_ctrs = &ppro_check_ctrs,
	.start = &ppro_start,
	.stop = &ppro_stop
};
