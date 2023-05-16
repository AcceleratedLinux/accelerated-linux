/*****************************************************************************/

/*
 *	hd -- simple hexdump utility
 *
 *	(C) Copyright 2000-2001, Greg Ungerer (gerg@moreton.com.au)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 * 
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*****************************************************************************/

char	*version = "1.0.3";
char	*progname;

/*
 *	Specify output types.
 */
#define	HEX	0
#define	DECIMAL	1
#define	OCTAL	2

#define	BIT8	1
#define	BIT16	2
#define	BIT32	4

int	obase = HEX;
int	osize = BIT32;
int	swap = 0;

/*****************************************************************************/

void usage(int rc)
{
	printf("usage: %s [-?vmodxbcwlyp] [-s offset] [-n num] [<filename>]\n\n", progname);
	printf( "\t-?\t-- this help\n"
		"\t-v\t-- print version\n"
		"\t-s\t-- skip offset bytes from start\n"
		"\t-n\t-- number of items to print\n"
		"\t-m\t-- minimal output (only data, no address or ascii)\n"
		"\t-o\t-- output in octal\n"
		"\t-d\t-- output in decimal\n"
		"\t-x\t-- output in hex (default)\n"
		"\t-b\t-- output as bytes\n"
		"\t-c\t-- output as bytes\n"
		"\t-w\t-- output as 16 bit words\n"
		"\t-l\t-- output as 32 bit words (default)\n"
		"\t-y\t-- byte swap (for 16 and 32 bit values)\n"
		"\t-p\t-- add number prefix (0x, 0, or nothing)\n");
	exit(rc);
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	int		fd, c, len, i;
	int		numsize, numperline, maximal, addprefix;
	char		*addrfmt, *fill, *prefix;
	char		numspaces[32], numfmt[10];
	char		numprtc;
	unsigned short	sval;
	unsigned char	ibuf[16];
	unsigned int	val;
	unsigned long	addr, offset = 0, num = 0, total = 0;

	progname = argv[0];
	maximal = 1;
	addprefix = 0;
	num--;

	while ((c = getopt(argc, argv, "?hvmodxbcwlyps:n:")) != EOF) {
		switch (c) {
		case 'v':
			printf("%s: version %s\n", progname, version);
			exit(0);
			break;
		case 'o':
			obase = OCTAL;
			break;
		case 's':
			if (sscanf(optarg, "%li", &offset) != 1) {
				printf("ERROR: invalid 'skip' value\n");
				usage(1);
			}
			break;
		case 'n':
			if (sscanf(optarg, "%li", &num) != 1) {
				printf("ERROR: invalid 'number' value\n");
				usage(1);
			}
			break;
		case 'm':
			maximal = 0;
			break;
		case 'd':
			obase = DECIMAL;
			break;
		case 'x':
			obase = HEX;
			break;
		case 'b':
		case 'c':
			osize = BIT8;
			break;
		case 'w':
			osize = BIT16;
			break;
		case 'l':
			osize = BIT32;
			break;
		case 'y':
			swap = 1;
			break;
		case 'p':
			addprefix = 1;
			break;
		case 'h':
		case '?':
			usage(0);
			break;
		default:
			fprintf(stderr, "%s: unknown option '%c'\n",
				progname, c);
			usage(1);
			break;
		}
	}

	if (optind < argc) {
		if ((fd = open(argv[optind], O_RDONLY)) < 0) {
			fprintf(stderr, "ERROR: could not open %s\n",
				argv[optind]);
			exit(1);
		}
	} else {
		fd = 0;
	}

	/*
	 *	Calculate formatting strings, line sizes, etc
	 */
	numperline = 16;
	prefix = "";

	if (obase == DECIMAL) {
		addrfmt = "%08ld: ";
		numprtc = 'u';
		fill = "";
		if (osize == BIT8) {
			numsize = 3;
			numperline = 8;
		} else if (osize == BIT16) {
			numsize = 5;
		} else {
			numsize = 10;
		}
		addprefix = 0;
	} else if (obase == OCTAL) {
		addrfmt = "%08lo: ";
		numprtc = 'o';
		numsize = (8 * osize) / 3 + 1;
                fill = "0";
		if ((osize == BIT8) || (osize == BIT16))
			numperline = 8;
		if (addprefix) {
			prefix = "0";
			numperline = 8;
			addprefix = 1;
		}
	} else {
		addrfmt = "%08lx: ";
		numprtc = 'x';
		numsize = 2 * osize;
		fill = "0";
		if (addprefix) {
			prefix = "0x";
			addprefix = 2;
			if ((osize == BIT8) || (osize == BIT16))
				numperline = 8;
		}
	}

	sprintf(&numfmt[0], "%s%%%s%d%c ", prefix, fill, numsize, numprtc);
	memcpy(&numspaces[0], "                    ", 20);
	numspaces[numsize + addprefix + 1] = 0;

	if (offset > 0)
		lseek(fd, offset, SEEK_SET);
	else if (offset < 0)
		lseek(fd, offset, SEEK_END);
	addr = lseek(fd, 0, SEEK_CUR);
	if (addr == (off_t)-1)
		addr = 0;

	/*
	 *	Do the actual printing.
	 */
	while (total < num) {
		memset(&ibuf[0], 0, sizeof(ibuf));

		len = read(fd, &ibuf[0], numperline);
		if (len <= 0)
			break;

		if (maximal)
			printf(addrfmt, addr);

		for (i = 0; (i < len); i += osize) {
			if (osize == BIT8) {
				val = (unsigned long) ibuf[i];
			} else if (osize == BIT16) {
				sval = *((unsigned short *) &ibuf[i]);
				if (swap)
					sval = (sval << 8) | (sval >> 8);
				val = (unsigned long) sval;
			} else {
				val = *((unsigned int *) &ibuf[i]);
				if (swap) {
					val = ((val & 0xff) << 24) |
						((val & 0xff00) << 8) |
						((val & 0xff0000) >> 8) |
						((val & 0xff000000) >> 24);
				}
			}

			printf(numfmt, val);

			if (++total >= num)
				len = i;
		}

		if (maximal) {
			for (; (i < numperline); i += osize)
				fputs(numspaces, stdout);

			fputs("    ", stdout);

			for (i = 0; (i < len); i++) {
				if ((ibuf[i] >= 0x20) && (ibuf[i] <= 0x7e))
					putchar(ibuf[i]);
				else
					putchar('.');
			}
		}

		putchar('\n');
		addr += len;
	}

	close(fd);
	return 0;
}

/*****************************************************************************/
