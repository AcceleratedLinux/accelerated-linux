/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2008 by Sascha Hauer <kernel@pengutronix.de>
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
