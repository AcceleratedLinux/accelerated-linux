/*
 * mtkmangle -- convert raw NAND binary to programmer NAND binary
 *
 * (C) Copyright 2018, Greg Ungerer <greg.ungerer@digi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char ibuf[2112], obuf[2112];
	char *ifile, *ofile;
	int ifd, ofd;
	ssize_t n;

	if (argc != 3) {
		printf("usage: mtkmangle <uboot-in.bin> <uboot-out.bin>\n");
		return 1;
	}

	ifile = argv[1];
	ofile = argv[2];

	ifd = open(ifile, O_RDONLY);
	if (ifd < 0) {
		printf("ERROR: cannot open input file '%s'\n", ifile);
		return 1;
	}

	ofd = open(ofile, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (ifd < 0) {
		printf("ERROR: cannot open input file '%s'\n", ofile);
		return 1;
	}

	for (;;) {
		n = read(ifd, ibuf, sizeof(ibuf));
		if (n == 0)
			break;
		if (n < 0) {
			printf("ERROR: failed reading, errno=%d\n", errno);
			return 1;
		}
		if (n != sizeof(ibuf)) {
			printf("ERROR: bad uboot input file size\n");
			return 1;
		}

		memcpy(&obuf[0], &ibuf[0], 512);
		memcpy(&obuf[512], &ibuf[2048], 8);
		memcpy(&obuf[520], &ibuf[2080], 8);

		memcpy(&obuf[528], &ibuf[512], 512);
		memcpy(&obuf[1040], &ibuf[2056], 8);
		memcpy(&obuf[1048], &ibuf[2088], 8);

		memcpy(&obuf[1056], &ibuf[1024], 512);
		memcpy(&obuf[1568], &ibuf[2064], 8);
		memcpy(&obuf[1576], &ibuf[2096], 8);

		memcpy(&obuf[1584], &ibuf[1536], 512);
		memcpy(&obuf[2096], &ibuf[2072], 8);
		memcpy(&obuf[2104], &ibuf[2104], 8);

		n = write(ofd, obuf, sizeof(obuf));
	}

	close(ofd);
	close(ifd);
	return 0;
}

