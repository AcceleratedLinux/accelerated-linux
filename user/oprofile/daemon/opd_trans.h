/**
 * @file daemon/opd_trans.h
 * Processing the sample buffer
 *
 * @remark Copyright 2002 OProfile authors
 * @remark Read the file COPYING
 *
 * @author John Levon
 * @author Philippe Elie
 */

#ifndef OPD_TRANS_H
#define OPD_TRANS_H

#include "opd_cookie.h"
#include "op_types.h"

struct sfile;
struct anon_mapping;

enum tracing_type {
	TRACING_OFF,
	TRACING_START,
	TRACING_ON
};

/**
 * Transient values used for parsing the event buffer.
 * Note that these are reset for each buffer read, but
 * that should be ok as in the kernel, cpu_buffer_reset()
 * ensures that a correct context starts off the buffer.
 */
struct transient {
	char const * buffer;
	size_t remaining;
	enum tracing_type tracing;
	struct sfile * current;
	struct sfile * last;
	struct anon_mapping * anon;
	struct anon_mapping * last_anon;
	cookie_t cookie;
	cookie_t app_cookie;
	vma_t pc;
	vma_t last_pc;
	unsigned long event;
	int in_kernel;
	unsigned long cpu;
	pid_t tid;
	pid_t tgid;
};

void opd_process_samples(char const * buffer, size_t count);

/** used when we need to clear data that's been freed */
void clear_trans_last(struct transient * trans);

/** used when we need to clear data that's been freed */
void clear_trans_current(struct transient * trans);

#endif /* OPD_TRANS_H */
