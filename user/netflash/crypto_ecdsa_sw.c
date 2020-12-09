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
#include <arpa/inet.h>
#include <string.h>
#include <openssl/pem.h>

#include "check.h"
#include "crypto_ecdsa_sw.h"
#include "dev_mode.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "sha256.h"
#include "util.h"

static EC_KEY *pubkey_buffer_to_eckey(const uint8_t *pubkey, size_t pubkey_len)
{
	EC_KEY *key;

	key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (key == NULL) {
		error("failed to create EC key");
		return NULL;
	}

	key = o2i_ECPublicKey(&key, &pubkey, pubkey_len);
	if (key == NULL) {
		error("invalid public key");
		EC_KEY_free(key);
		return NULL;
	}

	return key;
}

#ifdef CONFIG_USER_NETFLASH_PUBKEY_KERNEL_CMDLINE

static int hexstr_to_bin(const char *str, size_t str_len, void *buf)
{
	int i;
	uint8_t *ptr = buf;

	for (i = 0; i < str_len; i++) {
		uint8_t val;
		uint8_t c = str[i];

		if (c >= '0' && c <= '9')
			val = c - '0';
		else if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			val = c - 'a' + 10;
		else
			return -1;

		if ((i % 2) == 0) {
			*ptr = val;
			*ptr <<= 4;
		} else {
			*ptr++ |= val;
		}
	}

	return 0;
}

static int is_pubkey_str_valid(const char *key_str, size_t key_len)
{
	/*
	 * Hex string key length should be:
	 * - 33 * 2 (compressed key + 1-byte prefix)
	 * - or 65 * 2 (uncompressed key + 1-byte prefix)
	 */
	if (key_len != (33 * 2) && key_len != (65 * 2))
		return 0;

	/*
	 * Possible prefixes:
	 * - 0x02: compressed
	 * - 0x03: compressed
	 * - 0x04: uncompressed
	 */
	if (key_str[0] != '0' || key_str[1] < '2' || key_str[1] > '4')
		return 0;

	return 1;
}

/*
 * Loads a public key from the kernel command line
 * @key_argname: kernel command line argument name
 * @key:         loaded key
 *
 * Returns:
 * 1: Loaded successfully
 * 0: Key not found in the command line
 * <0: Error
 */
static int load_pubkey_one(const char *key_argname, EC_KEY **key)
{
	char *key_str;
	size_t key_len;
	/* Max. key length is 65 bytes */
	uint8_t key_buf[65];

	*key = NULL;

	key_str = arg_from_kernel_cmdline(key_argname);
	if (key_str == NULL)
		return 0;

	key_len = strlen(key_str);
	if (!is_pubkey_str_valid(key_str, key_len))
		goto err;

	if (hexstr_to_bin(key_str, key_len, key_buf) != 0)
		goto err;

	*key = pubkey_buffer_to_eckey(key_buf, key_len / 2);
err:
	free(key_str);
	return *key ? 1 : -1;
}

static void load_pubkeys(EC_KEY **key_prod, EC_KEY **key_dev)
{
	int ret;

	*key_prod = *key_dev = NULL;

	ret = load_pubkey_one("pk_prod=", key_prod);
	if (ret < 0) {
		/*
		 * Production key found, but failed to load it because of some
		 * reason. This is always a fatal failure
		 */
		error("failed to load production key");
		exit(BAD_PUB_KEY);
	}
	else if (ret == 0) {
#ifndef CONFIG_USER_NETFLASH_CRYPTO_ECDSA_SW_OPTIONAL
		/*
		 * Production key not found: this is a failure now, as it is
		 * configured to be mandatory
		 */
		error("failed to find production key");
		exit(BAD_PUB_KEY);
#else
		/* Just notice the user */
		notice("failed to find production key");
#endif
	}

	/* Don't load dev-key, if it is disabled */
	if (!is_development_key_allowed())
		return;

	notice("development keys allowed");

	ret = load_pubkey_one("pk_dev=", key_dev);
	if (ret < 0)
		notice("failed to load development key");
	else if (ret == 0)
		notice("failed to find development key");
}
#else
#error "Public key loading method not defined"
#endif

static ECDSA_SIG *load_sig(struct ecdsa_sw_signature *signature)
{
	ECDSA_SIG *sig;
	BIGNUM *r, *s;

	sig = ECDSA_SIG_new();
	if (sig == NULL) {
		error("failed to create signature");
		exit(BAD_DECRYPT);
	}

	r = BN_bin2bn(signature->r, sizeof(signature->r), NULL);
	if (r == NULL) {
		error("failed to load signature R");
		exit(BAD_CRYPT_SIGN);
	}
	s = BN_bin2bn(signature->s, sizeof(signature->s), NULL);
	if (s == NULL) {
		error("failed to load signature S");
		exit(BAD_CRYPT_SIGN);
	}

	ECDSA_SIG_set0(sig, r, s);

	return sig;
}

void check_crypto_ecdsa_sw(struct check_opt *opt)
{
	struct ecdsa_sw_signature sig_raw;
	unsigned char sha256[SHA256_HASH_BYTESIZE];
	int ret = -255;
	unsigned long blobsize;
	EC_KEY *key_prod, *key_dev;
	ECDSA_SIG *sig;

	check(opt);

	if (fb_seek_end(sizeof(struct ecdsa_sw_signature)) != 0) {
		error("file is too short");
		exit(IMAGE_SHORT);
	}

	blobsize = fb_tell();

	if (blobsize == 0) {
		error("signature not found (blob parse error)");
		exit(BAD_CRYPT_LEN);
	}

	load_pubkeys(&key_prod, &key_dev);

	/*
	 * If production key is configured to be mandatory, and we failed to
	 * find it, we already failed and exit and this point. So it is safe
	 * to check for both keys' existence, and return with OK, if neither
	 * were found
	 */
	if (key_prod == NULL && key_dev == NULL) {
		notice("no public keys found");
		return;
	}

	/* Read signature */
	if (fb_read(&sig_raw, sizeof(sig_raw)) != sizeof(sig_raw)) {
		error("signature not found (unexpected length)");
		exit(BAD_CRYPT_LEN);
	}

	if (htonl(sig_raw.magic_hdr) != ECDSA_SW_SIGN_MAGIC) {
		error("signature not found (bad magic)");
		exit(BAD_CRYPT_MAGIC);
	}

	sig = load_sig(&sig_raw);

	fb_compute_sha256(blobsize, sha256);

	/* Verify signature against the production key first (if loaded) */
	ret = (key_prod == NULL) ? -1 :
		ECDSA_do_verify(sha256, sizeof(sha256), sig, key_prod);

	if (ret != 1 && key_dev != NULL)
		ret = ECDSA_do_verify(sha256, sizeof(sha256), sig, key_dev);

	switch (ret) {
	case 1:
		notice("authentication successful");
		break;
	case 0:
		error("invalid signature");
		exit(BAD_CRYPT_SIGN);
	default:
		error("authentication failed");
		exit(BAD_CRYPT_SIGN);
	}

	ECDSA_SIG_free(sig);
	EC_KEY_free(key_prod);
	EC_KEY_free(key_dev);

	if (opt->doremovesig)
		fb_trim(sizeof(struct ecdsa_sw_signature));
}
