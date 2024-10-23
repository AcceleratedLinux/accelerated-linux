/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 *  Copyright 2023 Digi International Inc.
 */
#ifndef _UAPI_MCU_TX40_H_
#define _UAPI_MCU_TX40_H_

#include <linux/types.h>

#define MCU_TX40_IOCTL_EXEC		0x0801

struct mcu_tx40_msg {
	void __user *tx;	/* Transmit buffer */
	void __user *rx;	/* Receive buffer */
	__u8 tx_len;		/* Transmit buffer's size */
	__u8 rx_len;		/* Receive buffer's size */
	__u8 expected_rx_len;	/* Expected response length */
};

/* The structure used in the MCU_TX40_IOCTL_EXEC function */
struct mcu_tx40_exec_data {
	struct mcu_tx40_msg __user *msgs;	/* Pointer to the messages */
	__u32 nmsgs;	/* Number of msgs */
};

/* Number of max. allowed messages in the struct mcu_tx40_exec_data */
#define MCU_TX40_EXEC_MSGS_MAX		16

#endif /* _UAPI_MCU_TX40_H_ */
