#ifndef NETFLASH_CRYPTO_ATMEL_H
#define NETFLASH_CRYPTO_ATMEL_H

#include <stdint.h>

#define ATMEL_SIGN_MAGIC 0x54361782

typedef struct {
	uint32_t magic_hdr;
	uint8_t r[32];
	uint8_t s[32];
} atmel_signature_t;

/*
 * Slot index:
 *  - Development key: Slot 9
 *  - Production key:  Slot 15
 */
#define DEV_KEY_SLOT            9
#define PRODUCTION_KEY_SLOT     15

void check_crypto_atmel();

#endif
