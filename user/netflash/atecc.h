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

#ifndef NETFLASH_ATECC_H
#define NETFLASH_ATECC_H

#include <stddef.h>
#include <stdint.h>

typedef struct  {
	int fd;
} atecc_ctx;

#define ATECC_STATUS_OK			0x00
#define ATECC_STATUS_VERIFY_FAILED	0x01
#define ATECC_STATUS_PARSE_ERROR	0x03
#define ATECC_STATUS_ECC_FAULT		0x05
#define ATECC_STATUS_EXECUTION_ERR	0x0F
#define ATECC_STATUS_WAKE_OK		0x11
#define ATECC_STATUS_WATCHDOG_SOON	0xEE
#define ATECC_STATUS_CRC_ERROR		0xFF

int atecc_init(atecc_ctx *ctx);
void atecc_close(atecc_ctx *ctx);
int verify(atecc_ctx *ctx, uint8_t slot_id, const uint8_t *message,
	   const uint8_t *r, const uint8_t *s);
int hmac(atecc_ctx *ctx, uint8_t slot_id,
	 const uint8_t *message, const uint8_t *hmac);

#endif
