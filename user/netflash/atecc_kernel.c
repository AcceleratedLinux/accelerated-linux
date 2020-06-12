/*****************************************************************************
* Copyright 2019, Digi International Inc.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#include "atecc.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <linux/atsha204-i2c.h>
#include <sys/ioctl.h>

#define NONCE_SIZE		32

#define ATECC_DEV		"/dev/atsha0"

/* ATECC OP codes */
#define ATECC_OP_CHECKMAC	0x28
#define ATECC_OP_COUNTER	0x24
#define ATECC_OP_DERIVEKEY	0x1C
#define ATECC_OP_ECDH		0x43
#define ATECC_OP_GENDIG		0x15
#define ATECC_OP_GENKEY		0x40
#define ATECC_OP_HMAC		0x11
#define ATECC_OP_INFO		0x30
#define ATECC_OP_LOCK		0x17
#define ATECC_OP_MAC		0x08
#define ATECC_OP_NONCE		0x16
#define ATECC_OP_PAUSE		0x01
#define ATECC_OP_PRIVWRITE	0x46
#define ATECC_OP_RANDOM		0x1B
#define ATECC_OP_READ		0x02
#define ATECC_OP_SIGN		0x41
#define ATECC_OP_SHA		0x47
#define ATECC_OP_UPDATEEXTRA	0x20
#define ATECC_OP_VERIFY		0x45
#define ATECC_OP_WRITE		0x12

int atecc_init(atecc_ctx *ctx)
{
	ctx->fd = open(ATECC_DEV, O_RDWR);
	if (ctx->fd < 0)
		return ctx->fd;

	return 0;
}

void atecc_close(atecc_ctx *ctx)
{
	close(ctx->fd);
	ctx->fd = -1;
}

/*
 * return value:
 *  < 0: IO error
 *    0: Verification OK
 *  > 0: Verification FAILED (see 'atecc_parse_status')
 */
int verify(atecc_ctx *ctx, uint8_t slot_id, const uint8_t *message,
	   const uint8_t *r, const uint8_t *s)
{
	int ret;
	uint8_t rx_buff_nonce[2];
	uint8_t rx_buff_verify[2] = {0, -1};
	uint8_t cmd_nonce[4 + NONCE_SIZE] = {
		ATECC_OP_NONCE,
		0x03, 		/* Param1: Pass-through mode */
		0x00,		/* Param2: (2 bytes)         */
		0x00		/*         no TempKey usage  */
	};
	uint8_t cmd_verify[4 + 32 + 32] = {
		ATECC_OP_VERIFY,
		0x00, 		/* stored mode        */
		slot_id,	/* key slot (2 bytes) */
		0x00
		/* 32 bytes for r */
		/* 32 bytes for s */
	};
	struct atsha204_i2c_exec_data msgset;
	struct atsha204_i2c_msg msgs[2];

	memcpy(cmd_nonce + 4, message, NONCE_SIZE);

	memcpy(cmd_verify + 4, r, 32);
	memcpy(cmd_verify + 4 + 32, s, 32);

	msgs[0].cmd = cmd_nonce;
	msgs[0].cmd_len = sizeof(cmd_nonce);
	msgs[0].resp = rx_buff_nonce;
	msgs[0].resp_len = sizeof(rx_buff_nonce);

	msgs[1].cmd = cmd_verify;
	msgs[1].cmd_len = sizeof(cmd_verify);
	msgs[1].resp = rx_buff_verify;
	msgs[1].resp_len = sizeof(rx_buff_verify);

	msgset.msgs = msgs;
	msgset.nmsgs = 2;

	ret = ioctl(ctx->fd, ATSHA204_I2C_IOCTL_EXEC, &msgset);
	if (ret < 0)
		return ret;

	return rx_buff_verify[1];
}

/*
 * return value:
 *  < 0: IO error
 *    0: Verification OK
 *  > 0: Verification FAILED (see 'atecc_parse_status')
 */
int hmac(atecc_ctx *ctx, uint8_t slot_id,
	 const uint8_t *message, const uint8_t *hmac)
{
	int ret;
	uint8_t rx_buff[2] = {0, -1};
	uint8_t cmd[4 + 32 + 32 + 13] = {
		ATECC_OP_CHECKMAC,
		0x00, 		/* mode: sources: ClientChal and Slot[SlotID] */
		slot_id,	/* key slot (2 bytes) */
		0x00
		/* 32 bytes for digest */
		/* 32 bytes for signature */
		/* 13 bytes for other data (not used, should be zero) */
	};
	struct atsha204_i2c_exec_data msgset;
	struct atsha204_i2c_msg msg;

	/* Copy digest */
	memcpy(cmd + 4, message, 32);
	/* Copy signature */
	memcpy(cmd + 4 + 32, hmac, 32);
	/* Other data is 0 */
	memset(cmd + 4 + 32 + 32, 0, 13);

	msg.cmd = cmd;
	msg.cmd_len = sizeof(cmd);
	msg.resp = rx_buff;
	msg.resp_len = sizeof(rx_buff);

	msgset.msgs = &msg;
	msgset.nmsgs = 1;

	ret = ioctl(ctx->fd, ATSHA204_I2C_IOCTL_EXEC, &msgset);
	if (ret < 0)
		return ret;

	return rx_buff[1];
}
