/*****************************************************************************
* Copyright 2020, Digi International Inc.
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
#ifndef NETFLASH_CRYPTO_ECDSA_SW_H
#define NETFLASH_CRYPTO_ECDSA_SW_H

#include <stdint.h>

/*
 * NOTE: it is re-using the Atmel's structure, as it supposed to be a SW
 * alternative for that
 */
#define ECDSA_SW_SIGN_MAGIC 0x54361782

struct ecdsa_sw_signature {
	uint32_t magic_hdr;
	uint8_t r[32];
	uint8_t s[32];
} ;

void check_crypto_ecdsa_sw();

#endif
