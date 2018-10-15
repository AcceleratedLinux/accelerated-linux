/* Timer helper functions
 *
 * Copyright (C) 2017  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <errno.h>
#include <signal.h>
#include <string.h>		/* memset() */
#include <stdlib.h>		/* malloc() */
#include <unistd.h>		/* read()/write() */
#include <time.h>

#include "log.h"
#include "queue.h"
#include "socket.h"

/*
 * TODO
 * - Timers should ideally be sorted in priority order, and/or
 * - Investigate using the pipe to notify which timer expired
 */
struct timer {
	LIST_ENTRY(timer) link;

	int             period;	/* period time in seconds */
	struct timespec timeout;

	void (*cb)(void *arg);
	void *arg;
};

static timer_t timer;
static int timerfd[2];
static LIST_HEAD(, timer) tl = LIST_HEAD_INITIALIZER();


static void set(struct timer *t, struct timespec *now)
{
	t->timeout.tv_sec  = now->tv_sec + t->period;
	t->timeout.tv_nsec = now->tv_nsec;
}

static int expired(struct timer *t, struct timespec *now)
{
	if (t->timeout.tv_sec < now->tv_sec)
		return 1;

	if (t->timeout.tv_sec == now->tv_sec && t->timeout.tv_nsec <= now->tv_nsec)
		return 1;

	return 0;
}

static struct timer *compare(struct timer *a, struct timer *b)
{
	if (a->timeout.tv_sec <= b->timeout.tv_sec) {
		if (a->timeout.tv_nsec <= b->timeout.tv_nsec)
			return a;

		return b;
	}

	return b;
}

static int start(struct timespec *now)
{
	struct timer *next, *entry;
	struct itimerspec it;

	if (LIST_EMPTY(&tl))
		return -1;

	next = LIST_FIRST(&tl);
	LIST_FOREACH(entry, &tl, link)
		next = compare(next, entry);

	memset(&it, 0, sizeof(it));
	it.it_value.tv_sec  = next->timeout.tv_sec - now->tv_sec;
	it.it_value.tv_nsec = 0;
	timer_settime(timer, 0, &it, NULL);

	return 0;
}

/* callback for activity on pipe */
static void run(int sd, void *arg)
{
	char dummy;
	struct timespec now;
	struct timer *entry;

	(void)arg;
	if (read(sd, &dummy, 1) < 0)
		smclog(LOG_DEBUG, "Failed read(pipe): %s", strerror(errno));

	clock_gettime(CLOCK_MONOTONIC, &now);
	LIST_FOREACH(entry, &tl, link) {
		if (expired(entry, &now)) {
			if (entry->cb)
				entry->cb(entry->arg);
			set(entry, &now);
		}
	}

	start(&now);
}

/* write to pipe to create an event for select() on SIGALRM */
static void handler(int signo)
{
	(void)signo;
	if (write(timerfd[1], "!", 1) < 0)
		smclog(LOG_DEBUG, "Failed write(pipe): %s", strerror(errno));
}

/*
 * register signal pipe and callbacks
 */
int timer_init(void)
{
	struct sigaction sa;

	if (pipe(timerfd))
		return -1;

	if (socket_register(timerfd[0], run, NULL) < 0)
		return -1;
	if (socket_register(timerfd[1], NULL, NULL) < 0)
		return -1;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	if (timer_create(CLOCK_MONOTONIC, NULL, &timer)) {
		socket_close(timerfd[0]);
		socket_close(timerfd[1]);
		return -1;
	}

	return 0;
}

/*
 * create periodic timer (seconds)
 */
int timer_add(int period, void (*cb)(void *), void *arg)
{
	struct timer *t;
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
		return -1;

	t = malloc(sizeof(*t));
	if (!t)
		return -1;

	t->period = period;
	t->cb     = cb;
	t->arg    = arg;

	set(t, &now);

	LIST_INSERT_HEAD(&tl, t, link);

	return start(&now);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
