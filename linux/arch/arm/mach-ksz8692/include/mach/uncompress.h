/*
 *  linux/include/asm-arm/arch-ks8692/uncompress.h                * PING_READY *
 *
 *  Copyright (C) 1999 ARM Limited
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <mach/platform.h>
#include <linux/version.h>

/*
 * This does not append a newline
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
static void putc(int c)
{
	while (!((*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_LINE_STATUS))
		& UART_LINES_TX_HOLD_EMPTY));
	(*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_TX_HOLDING)) = (u_int) c;
}

static inline void flush(void)
{
	while (!((*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_LINE_STATUS))
		& UART_LINES_TX_HOLD_EMPTY));
}

#else
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,6))
static void puts(const char *s)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9))
int puts(const char *s)
#else
static void putstr(const char *s)
#endif
{
	char val;

	while (*s) 
    {
		while (!((*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_LINE_STATUS))
			      & UART_LINES_TX_HOLD_EMPTY));
		val = *s;
		(*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_TX_HOLDING)) = (u_int) val;

		if (*s == '\n') 
        {
			while (!((*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_LINE_STATUS))
				      & UART_LINES_TX_HOLD_EMPTY));

			val = '\r';
			(*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_TX_HOLDING)) = (u_int) val;
		}
		s++;
	}

	while (!((*(volatile u_int *) (KS8692_IO_BASE + KS8692_UART1_LINE_STATUS))
		      & UART_LINES_TX_HOLD_EMPTY));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,6)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9))
	return 1;
#endif
}
#endif

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
