#ifndef NETFLASH_CRYPTO_ATMEL_H
#define NETFLASH_CRYPTO_ATMEL_H

#include <config/autoconf.h>
#include <stdint.h>

#define ATMEL_SIGN_MAGIC 0x54361782


typedef struct {
	uint32_t magic_hdr;
#if defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_ECDSA)
	uint8_t r[32];
	uint8_t s[32];
#elif defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_HMAC)
	uint8_t hmac[32];
#else
# error "Invalid ATECC508 algorithm"
#endif
} atmel_signature_t;

/*
 * Slot index:
 *  - Development key
 *  - Production key
 */
#ifndef CONFIG_USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT
# error "Development key slot is not defined"
#endif /* CONFIG_USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT */

#ifndef CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT
# error "Production key slot is not defined"
#endif /* CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT */

void check_crypto_atmel();

#endif
