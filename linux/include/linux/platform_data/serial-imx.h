/*
 * Copyright (C) 2008 by Sascha Hauer <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef ASMARM_ARCH_UART_H
#define ASMARM_ARCH_UART_H

#define IMXUART_HAVE_RTSCTS (1<<0)
#define IMXUART_DTE         (1<<1)
#define IMXUART_HAVE_DTRDCD (1<<2)
#define IMXUART_HAVE_DTR_GPIO (1<<3)
#define IMXUART_HAVE_DCD_GPIO (1<<4)

struct imxuart_platform_data {
	unsigned int flags;
	unsigned int dtr_gpio;
	unsigned int dcd_gpio;
};

#endif
