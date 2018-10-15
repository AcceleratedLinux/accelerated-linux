/* vi: set sw=4 ts=4: */
/*
 * Mini sync implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define sync_trivial_usage
//usage:       ""
//usage:#define sync_full_usage "\n\n"
//usage:       "Write all buffered blocks to disk"

#include <config/autoconf.h>
#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

int sync_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int sync_main(int argc UNUSED_PARAM, char **argv IF_NOT_DESKTOP(UNUSED_PARAM))
{
#ifdef CONFIG_USER_FLATFSD_FLATFSD
	int   verbose = 0;
	int   flash = 0;
	int   opt;

	while ((opt=getopt(argc, argv, "vf")) != -1) {
		switch(opt) {
			case 'v':
				verbose = 1;
				break;
			case 'f':
				flash = 1;
				break;
			default:
				bb_show_usage();
		}
	}

	/* get the pid of flatfsd */
	if (flash) {
		if (verbose)
			puts("sync: flash");
		system("exec flatfsd -s");
	}
	if (verbose)
		puts("sync: file systems");
#else
	/* coreutils-6.9 compat */
	bb_warn_ignoring_args(argv[1]);
#endif

	sync();

	return EXIT_SUCCESS;
}
