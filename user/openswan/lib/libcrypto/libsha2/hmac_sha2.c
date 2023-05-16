#ifdef __KERNEL__
# include <linux/types.h>
# include <linux/string.h>
#else
# include <sys/types.h>
# include <string.h>
# ifdef HAVE_LIBNSS
#  include <pk11pub.h>
# endif
#endif
#include "hmac_generic.h"
#include "sha2.h"
#include "hmac_sha2.h"

void inline sha256_result(sha256_context *ctx, u_int8_t * hash, int hashlen) {
#ifdef HAVE_LIBNSS
	unsigned int len;
	SECStatus s = PK11_DigestFinal(ctx->ctx_nss, hash, &len, hashlen);
	PR_ASSERT(len==hashlen);
	PR_ASSERT(s==SECSuccess);
	PK11_DestroyContext(ctx->ctx_nss, PR_TRUE);
#else
	sha256_final(ctx);
	memcpy(hash, &ctx->sha_out[0], hashlen);
#endif
}

void inline sha512_result(sha512_context *ctx, u_int8_t * hash, int hashlen) {
#ifdef HAVE_LIBNSS
	unsigned int len;
	SECStatus s = PK11_DigestFinal(ctx->ctx_nss, hash, &len, hashlen);
	PR_ASSERT(len==hashlen);
	PR_ASSERT(s==SECSuccess);
	PK11_DestroyContext(ctx->ctx_nss, PR_TRUE);
#else
	sha512_final(ctx);
	memcpy(hash, &ctx->sha_out[0], hashlen);
#endif
}

#ifndef HAVE_LIBNSS
HMAC_SET_KEY_IMPL (sha256_hmac_set_key, 
		sha256_hmac_context, SHA256_BLOCKSIZE, 
		sha256_init, sha256_write)
HMAC_HASH_IMPL (sha256_hmac_hash, 
		sha256_hmac_context, sha256_context, SHA256_HASHLEN,
		sha256_write, sha256_result)

HMAC_SET_KEY_IMPL (sha512_hmac_set_key, 
		sha512_hmac_context, SHA512_BLOCKSIZE, 
		sha512_init, sha512_write)
HMAC_HASH_IMPL (sha512_hmac_hash, 
		sha512_hmac_context, sha512_context, SHA512_HASHLEN,
		sha512_write, sha512_result)
#endif
