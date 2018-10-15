/*
 * hexdump.c -- hexdump library routine
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
#include <string.h>
#include <stdlib.h>
#include "types.h"

void hexdump(const uint8_t *buf, size_t len)
{
	unsigned int i;
        char c[20];

        memset(&c[0], 0, sizeof(c));

        for (i = 0; i < len; i++, buf++) {
                if ((i % 16) == 0)
                        printf("0x%08x:", i);
                printf(" %02x", *buf);
                c[i & 0xf] = ((*buf >= 0x20) && (*buf <= 0x7e)) ? *buf : '.';
                if (((i + 1) % 16) == 0)
                        printf("  %s\n", c);
        }
        if ((i % 16) != 0) {
                c[i & 0xf] = 0;
                for (i &= 0xf; i < 16; i++)
                        printf("   ");
                printf("    %s\n", c);
        }

}

