/*
 * mkeccbin.c -- generate an ECC anotated binary file
 *
 * (C) Copyright 2014, Greg Ungerer <gerg@uclinux.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "types.h"
#include "hexdump.h"
#include "bch.h"

/*
 * Default sizes, match common 2k page NAND flash with BCH 4bit.
 */
#define PAGESIZE	2048
#define SPARESIZE	64
#define	ECCSIZE		30
#define ECCOFFSET	32

static unsigned int pagesize;
static unsigned int sparesize;
static unsigned int eccsize;
static unsigned int eccoffset;

int main(int argc, char *argv[])
{
	struct bch_control *bp;
	uint8_t *buf, *eccbuf, *eccmask;
	size_t bufsize, datasize, got;
	int fd, dump, pages, m, t, i;
	char *file;

	dump = 1;
	file = "u-boot.bin";

	pagesize = PAGESIZE;
	sparesize = SPARESIZE;
	eccsize = ECCSIZE;
	eccoffset = ECCOFFSET;

	printf("%s(%d): pagesize=0x%0x(%d)\n", __FILE__, __LINE__, pagesize, pagesize);
	printf("%s(%d): sparesize=0x%0x(%d)\n", __FILE__, __LINE__, sparesize, sparesize);
	printf("%s(%d): eccsize=0x%0x(%d)\n", __FILE__, __LINE__, eccsize, eccsize);
	printf("%s(%d): eccoffset=0x%0x(%d)\n", __FILE__, __LINE__, eccoffset, eccoffset);

	m = fls(1 + 8 * pagesize);
	t = (eccsize * 8) / m;
	bp = init_bch(m, t, 0);
	printf("%s(%d): m=%d t=%d\n", __FILE__, __LINE__, m, t);

	bufsize = pagesize + sparesize;
	buf = malloc(bufsize);
	datasize = pagesize + eccoffset;
	eccbuf = malloc(eccsize);

	/* Caulate ECC mask bits */
	eccmask = malloc(eccsize);
	memset(eccmask, 0, eccsize);
	memset(buf, 0xff, bufsize);
	encode_bch(bp, buf, datasize, eccmask);
	for (i = 0; i < eccsize; i++)
		eccmask[i] ^= 0xff;
	printf("%s(%d): ECC MASK:\n", __FILE__, __LINE__);
	hexdump(eccmask, eccsize);

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: failed to open(%s), %s\n", file, strerror(errno));
		return -1;
	}

	for (pages = 0; ; pages++) {
		got = read(fd, buf, pagesize);
		if (got <= 0)
			break;
		memset(buf + got, 0xff, bufsize - got);
		memset(eccbuf, 0, eccsize);
		encode_bch(bp, buf, datasize, eccbuf);

		for (i = 0; i < eccsize; i++)
			buf[datasize + i] = eccbuf[i] ^ eccmask[i];

		if (dump) {
			printf("PAGE=%d (offset=0x%08x)\n", pages, pages * pagesize);
			hexdump(buf, bufsize);
		}
	}

	close(fd);
	free_bch(bp);
	return 0;
}

