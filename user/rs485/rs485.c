/*
 * rs485.c -- report and set the RS-485 configuration on a port
 *
 * (C) Copyright 2020, 2013, Greg Ungerer <gerg@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

void usage(void)
{
	printf("usage: rs485 [-hscBAR] [-p <delay>] [-P <delay>] [-d <device>]\n"
		"\t-h\t\tprint this help\n"
		"\t-s\t\tset port into RS-485 mode\n"
		"\t-c\t\tclear port from RS-485 mode\n"
		"\t-B\t\tRTS on send flag\n"
		"\t-A\t\tRTS after send flag\n"
		"\t-R\t\tRX during TX flag\n"
		"\t-p <delay>\tdelay before send\n"
		"\t-P <delay>\tdelay after send\n"
		"\t-d <device>\tuse <device> as port\n");
}

int main(int argc, char *argv[])
{
	struct serial_rs485 rs485conf;
	unsigned int flags, predelay, postdelay;
	char *device;
	int fd, c, set;

	device = NULL;
	flags = 0;
	predelay = 0;
	postdelay = 0;
	set = 0;
	fd = 0; /*stdin*/

	while ((c = getopt(argc, argv, "hscBARp:P:d:")) > 0) {
		switch (c) {
		case 's':
			flags |= SER_RS485_ENABLED;
			set = 1;
			break;
		case 'c':
			flags &= ~SER_RS485_ENABLED;
			set = 1;
			break;
		case 'B':
			flags |= SER_RS485_RTS_ON_SEND;
			set = 1;
			break;
		case 'A':
			flags |= SER_RS485_RTS_AFTER_SEND;
			set = 1;
			break;
		case 'R':
			flags |= SER_RS485_RX_DURING_TX;
			set = 1;
			break;
		case 'p':
			predelay = atoi(optarg);
			set = 1;
			break;
		case 'P':
			postdelay = atoi(optarg);
			set = 1;
			break;
		case 'd':
			device = optarg;
			break;
		case 'h':
			usage();
			return 0;
		default:
			usage();
			return 1;
		}
	}

	if (device == NULL) {
		device = "<stdin>";
	} else {
		fd = open(device, O_RDWR | O_NDELAY);
		if (fd < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
				device, errno);
			return 1;
		}
	}

	if (set) {
		memset(&rs485conf, 0, sizeof(rs485conf));

		rs485conf.flags = flags;
		rs485conf.delay_rts_before_send = predelay;
		rs485conf.delay_rts_after_send = postdelay;

		if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
			printf("ERROR: failed to ioctl(TIOCSRS485) on %s, "
				"errno=%d\n", device, errno);
			return 1;
		}

		printf("rs485: %s RS-485 mode on %s\n",
			(flags & SER_RS485_ENABLED) ? "set" : "clear",
			device);

	} else {
		if (ioctl (fd, TIOCGRS485, &rs485conf) < 0) {
			printf("ERROR: failed to ioctl(TIOCGRS485) on %s, "
				"errno=%d\n", device, errno);
			return 1;
		}

		printf("rs485: port %s\n", device);
		printf("\tflags = ");
		if (rs485conf.flags & SER_RS485_ENABLED)
			printf("SER_RS485_ENABLED ");
		if (rs485conf.flags & SER_RS485_RTS_ON_SEND)
			printf("SER_RS485_RTS_ON_SEND ");
		if (rs485conf.flags & SER_RS485_RTS_AFTER_SEND)
			printf("SER_RS485_RTS_AFTER_SEND ");
		if (rs485conf.flags & SER_RS485_RX_DURING_TX)
			printf("SER_RS485_RX_DURING_TX ");
		printf("\n");
		printf("\tdelay_rts_before_send = %d\n",
				rs485conf.delay_rts_before_send);
		printf("\tdelay_rts_after_send = %d\n",
				rs485conf.delay_rts_after_send);
	}

	close(fd);
	return 0;
}

