
#include <sys/time.h>
#include "util.h"

static struct timeval time_started;

int time_rate_100 = 25;

void time_start(void)
{
	gettimeofday(&time_started, NULL);
}

/* returns 1/100's off seconds elapsed */
unsigned long time_elapsed(void)
{
	struct timeval now;
	unsigned long val;

	gettimeofday(&now, NULL);

	val = (now.tv_sec - time_started.tv_sec) * 100;
	if (now.tv_usec < time_started.tv_usec) {
		val += (1000000L + now.tv_usec - time_started.tv_usec) / 10000;
		val -= 100;
	} else
		val += (now.tv_usec - time_started.tv_usec) / 10000;

	return val;
}

