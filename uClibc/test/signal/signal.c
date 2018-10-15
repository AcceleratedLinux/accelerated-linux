/* vi: set sw=4 ts=4: */
/*
 * signal testing function for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>


/* -------------------------------------------------*/
/* This stuff is common to all the testing routines */
/* -------------------------------------------------*/
const char *it = "<UNSET>";		/* Routine name for message routines. */
size_t errors = 0;

static void check(int thing, int number)
{
	if (!thing) {
		printf("%s: flunked test %d\n", it, number);
		++errors;
	}
}

#if 0
static void equal(const char *a, const char *b, int number)
{
	check(a != NULL && b != NULL && (strcmp(a, b) == 0), number);
}
#endif


/* -------------------------------------------------*/
/* Let the tests begin....                          */
/* -------------------------------------------------*/

int global_int = 0;

static void set_global_int_to_one(int signum)
{
	printf ("Received signal %d (%s).\n", signum, strsignal(signum));
	global_int = 1;
	return;
}

static void signal_test_1(void)
{
	global_int = 0;

	it = "global variable set from signal handler";
	if (signal(SIGUSR1, set_global_int_to_one) == SIG_ERR) {
		perror("signal(SIGUSR1) failed");
		exit(-1);
	}
	raise(SIGUSR1);

	/* This should already have jumped to the signal handler */
	check((global_int == 1), 1);

	global_int = 0;
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
		perror("signal(SIGUSR1) failed");
		exit(-1);
	}
	raise(SIGUSR1);
	/* This should not go to the signal handler this time since we  */
	check((global_int == 0), 1);
}


int main(void)
{
	int status;

	signal_test_1();

	if (errors == 0) {
		status = EXIT_SUCCESS;
		printf("No errors.\n");
	} else {
		status = EXIT_FAILURE;
		printf("%lu errors.\n", (unsigned long)errors);
	}
	exit(status);
}
