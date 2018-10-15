/*
 * sigs.c -- report and set the RS-232 signals on a port
 *
 * (C) Copyright 2013, Greg Ungerer <gerg@uclinux.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

void usage(void)
{
	printf("usage: sigs [-hs] [-D <dtr>] [-R <rts>] [-d <device>]\n"
		"\t-h\t\tprint this help\n"
		"\t-s\t\tuse TIOCM_SET instead of TIOCM_MBIS/TIOCM_MBIC\n"
		"\t-D\t\tset DTR to <dtr> value\n"
		"\t-R\t\tset RTS to <rts> value\n"
		"\t-d <device>\tuse <device> as port\n");
}

int main(int argc, char *argv[])
{
	unsigned int val;
	char *device;
	int dtr, rts, set;
	int fd, c;

	fd = 0; /*stdin*/
	device = NULL;
	set = 0;
	dtr = -1;
	rts = -1;

	while ((c = getopt(argc, argv, "shd:D:R:")) > 0) {
		switch (c) {
		case 'd':
			device = optarg;
			break;
		case 'D':
			dtr = atoi(optarg);
			break;
		case 'R':
			rts = atoi(optarg);
			break;
		case 's':
			set++;
			break;
		case 'h':
			usage();
			return 0;
		default:
			usage();
			return 1;
		}
	}

	if (device != NULL) {
		fd = open(device, O_RDWR | O_NDELAY);
		if (fd < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
				device, errno);
			return 1;
		}
	}

	if ((dtr >= 0) || (rts >= 0)) {
		if (set) {
			val = 0;
			if (dtr > 0)
				val |= TIOCM_DTR;
			if (rts > 0)
				val |= TIOCM_RTS;
			if (ioctl(fd, TIOCMSET, &val) < 0)
				printf("ERROR: TIOCMSET(%s) failed, "
					"errno=%d\n", device, errno);
		} else {
			val = 0;
			if (dtr == 0)
				val |= TIOCM_DTR;
			if (rts == 0)
				val |= TIOCM_RTS;
			if (ioctl(fd, TIOCMBIC, &val) < 0)
				printf("ERROR: TIOCMBIC(%s) failed, "
					"errno=%d\n", device, errno);

			val = 0;
			if (dtr > 0)
				val |= TIOCM_DTR;
			if (rts > 0)
				val |= TIOCM_RTS;
			if (ioctl(fd, TIOCMBIS, &val) < 0)
				printf("ERROR: TIOCMBIS(%s) failed, "
					"errno=%d\n", device, errno);
		}
	} else {
		if (ioctl(fd, TIOCMGET, &val) < 0) {
			printf("ERROR: TIOCMGET(%s) failed, errno=%d\n",
				device, errno);
			return 1;
		}

		printf("SIGS: dtr=%d dcd=%d rts=%d cts=%d dsr=%d ri=%d\n",
			(val & TIOCM_DTR) ? 1 : 0,
			(val & TIOCM_CAR) ? 1 : 0,
			(val & TIOCM_RTS) ? 1 : 0,
			(val & TIOCM_CTS) ? 1 : 0,
			(val & TIOCM_DSR) ? 1 : 0,
			(val & TIOCM_RI) ? 1 : 0);
	}

	close(fd);
	return 0;
}

