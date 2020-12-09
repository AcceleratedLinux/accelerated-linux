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
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "atecc.h"
#include "check.h"
#include "crypto_atmel.h"
#include "dev_mode.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "sha256.h"
#include "util.h"

#define CRYPTOCHIP_RETRIES 	5

static int signature_valid(atecc_ctx *ctx, int key_slot,
			   const unsigned char *sha256, const atmel_signature_t *signature)
{
	int ret;
#if defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_ECDSA)
	ret = verify(ctx, key_slot, sha256, signature->r, signature->s);
#elif defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_HMAC)
	ret = hmac(ctx, key_slot, sha256, signature->hmac);
#endif

	return ret;
}

void check_crypto_atmel(struct check_opt *opt)
{
	atmel_signature_t signature;
	unsigned char sha256[SHA256_HASH_BYTESIZE];
	int retry, ret = -255;
	atecc_ctx ctx;
	unsigned long blobsize;
	int key_index = CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT;

	check(opt);

	if (fb_seek_end(sizeof(atmel_signature_t)) != 0) {
		error("file is too short");
		exit(IMAGE_SHORT);
	}

	blobsize = fb_tell();

	if (blobsize == 0) {
		error("signature not found (blob parse error)");
		exit(BAD_CRYPT_LEN);
	}

	/* Read atmel signature */
	if (fb_read(&signature, sizeof(signature)) != sizeof(signature)) {
		error("signature not found (unexpected length)");
		exit(BAD_CRYPT_LEN);
	}

	if (htonl(signature.magic_hdr) != ATMEL_SIGN_MAGIC) {
		error("signature not found (bad magic)");
		exit(BAD_CRYPT_MAGIC);
	}

	fb_compute_sha256(blobsize, sha256);

	for (retry = 0; retry < CRYPTOCHIP_RETRIES; retry++) {
		/* If this is not the first attempt, reset the cryptochip */
		if (retry > 0) {
			notice("retrying authentication... (%d/%d)", retry + 1, CRYPTOCHIP_RETRIES);
			atecc_close(&ctx);
			/* Wait 2s ( > t_WATCHDOG) */
			usleep(2 * 1000 * 1000);
		}

		if (atecc_init(&ctx)) {
			ret = CRYPTOCHIP_IO_ERROR;
			continue;
		}

		/* Authenticate signature */
		ret = signature_valid(&ctx, key_index, sha256, &signature);
		if (ret == ATECC_STATUS_OK) {
			break;
		} else if (ret == ATECC_STATUS_VERIFY_FAILED) {
			if (key_index == CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT && is_development_key_allowed()) {
				key_index = CONFIG_USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT;
				retry = -1;
				atecc_close(&ctx);
				continue;
			}

			error("invalid signature");
			ret = BAD_CRYPT_SIGN;
			break;
		} else {
			error("authentication failed (%d, %d)", ret, errno);
			ret = CRYPTOCHIP_IO_ERROR;
			continue;
		}
	}

	atecc_close(&ctx);

	if (ret)
		exit(ret);

	if (opt->doremovesig)
		fb_trim(sizeof(atmel_signature_t));

	notice("authentication successful");
}
