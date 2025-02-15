/*****************************************************************************/

/*
 *	discard.c -- accept all reads and discard data.
 *
 *	(C) Copyright 1999, Greg Ungerer.
 */

/*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>

/*
 *	Temporary buffer to use when working.
 */

/*****************************************************************************/

unsigned char	buf[8192];

int main(int argc, char *argv[])
{
	int	n;

	for (;;) {
		if ((n = read(0, buf, sizeof(buf))) <= 0)
			exit(1);
	}
}

/*****************************************************************************/
