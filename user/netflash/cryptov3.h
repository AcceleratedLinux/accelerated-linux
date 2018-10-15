#ifndef NETFLASH_CRYPTOV3_H
#define NETFLASH_CRYPTOV3_H

#include <stdint.h>

#define CRYPTO_V3_MAGIC			0xd5e8b29c
#define CRYPTO_V3_TRAILER_MAGIC		0x8f7d66ab

#define CRYPTO_V3_TAG_CERT		0x01
#define CRYPTO_V3_TAG_SIGN_SHA256	0x02

struct crypto_v3_header {
	uint32_t magic;
	uint8_t flags;
} __attribute__ ((packed));

struct crypto_v3_trailer {
	uint32_t hlen;  /* Length of header and tags */
	uint32_t magic; /* Magic number for identification purposes */
};

#ifdef CONFIG_USER_NETFLASH_NETFLASH
struct check_opt;
void check_crypto_v3(const struct check_opt *opt);
#endif

#endif
