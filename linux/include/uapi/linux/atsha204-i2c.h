/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * ATSHA204 / ATECCx08 I2C driver - user space header file
 *
 * Copyright (C) 2020 Digi International Inc.
 */
#ifndef __ATSHA204_I2C_UAPI_H_
#define __ATSHA204_I2C_UAPI_H_

#include <linux/types.h>

/* /dev/atshax ioctl commands */

/*
 * Execute multiple commands through the crypto chip. Using this command
 * guarantees that all the commands will be sent after each other (atomically).
 * Parameter is 'struct atsha204_i2c_exec_data'
 */
#define ATSHA204_I2C_IOCTL_EXEC		0x0501

struct atsha204_i2c_msg {
	__u8 __user	*cmd;		/* Command buffer */
	__u8 __user	*resp;		/* Response buffer */
					/* (<length> + <data_bytes>, no CRC) */
	__u8		cmd_len;	/* Command buffer's size */
	__u8		resp_len;	/* Response buffer's size */
};

/* The structure used in the ATSA204_I2C_IOCTL_EXEC function */
struct atsha204_i2c_exec_data {
	struct atsha204_i2c_msg __user	*msgs;	/* Pointer to the messages */
	__u32				nmsgs;	/* Number of msgs */
};

/* Number of max. allowed messages in the struct atsha204_i2c_exec_data */
#define ATSHA204_I2C_EXEC_MSGS_MAX	16

#endif /* __ATSHA204_I2C_UAPI_H_ */
