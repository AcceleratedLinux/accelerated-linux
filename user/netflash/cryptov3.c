#include <config/autoconf.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include "cryptov3.h"
#include "check.h"
#include "fileblock.h"
#include "exit_codes.h"
#include "util.h"

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
#define X509_STORE_get0_param(store) (store->param)
#endif

static X509_STORE *load_ca(const char *path)
{
	X509_STORE *store;
	X509_LOOKUP *lookup;
	X509_VERIFY_PARAM *param;

	store = X509_STORE_new();
	if (!store) {
		error("cannot create X509 store");
		exit(BAD_CRYPT_CA);
	}

	param = X509_STORE_get0_param(store);
	X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_X509_STRICT);

	lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
	if (!lookup) {
		error("cannot create X509 lookup");
		exit(BAD_CRYPT_CA);
	}

	if (X509_LOOKUP_load_file(lookup, path, X509_FILETYPE_PEM) != 1) {
		error("cannot load CA: %s", path);
		exit(BAD_CRYPT_CA);
	}

	return store;
}

static void load_crl(X509_STORE *store, const char *path)
{
	FILE *f;
	X509_CRL *crl;
	X509_VERIFY_PARAM *param;
	unsigned long flags;

	if (!path)
		return;

	f = fopen(path, "r");
	if (!f) {
		error("cannot open CRL: %s", path);
		exit(BAD_CRYPT_CRL);
	}

	crl = PEM_read_X509_CRL(f, NULL, NULL, "");
	if (!crl) {
		error("cannot read CRL");
		exit(BAD_CRYPT_CRL);
	}

	fclose(f);

	if (X509_STORE_add_crl(store, crl) != 1) {
		error("cannot add CRL to store");
		exit(BAD_CRYPT_CRL);
	}

	param = X509_STORE_get0_param(store);
	flags = X509_VERIFY_PARAM_get_flags(param);
	flags |= X509_V_FLAG_CRL_CHECK;
	X509_VERIFY_PARAM_set_flags(param, flags);
}

static X509 *check_cert(const unsigned char *buf, unsigned int len, X509_STORE *store)
{
	X509_STORE_CTX *ctx;
	X509 *cert;

	cert = d2i_X509(NULL, &buf, len);
	if (!cert) {
		error("cannot read public cert");
		exit(BAD_CRYPT_CERT);
	}

	ctx = X509_STORE_CTX_new();
	if (!ctx) {
		error("cannot create X509 context");
		exit(BAD_CRYPT_CERT);
	}

	if (X509_STORE_CTX_init(ctx, store, cert, NULL) != 1) {
		error("cannot create X509 context");
		exit(BAD_CRYPT_CERT);
	}

	if (X509_verify_cert(ctx) != 1) {
		error("X509 certificate verification failed");
		exit(BAD_CRYPT_CERT);
	}

	X509_STORE_CTX_free(ctx);
	return cert;
}

static void check_sign_sha256(unsigned char *sig, unsigned int siglen,
		X509 *cert, unsigned long length)
{
	unsigned long fblength;
	unsigned char *data;
	EVP_MD_CTX *ctx;
	EVP_PKEY *key;

	if (!cert) {
		error("missing cert before signature");
		exit(BAD_CRYPT_CERT);
	}

	key = X509_get_pubkey(cert);
	if (!key) {
		error("cannot get public key from cert");
		exit(BAD_CRYPT_CERT);
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx) {
		error("cannot create digest context");
		exit(BAD_CRYPT_SIGN);
	}

	if (EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, key) != 1) {
		error("cannot initialise digest context");
		exit(BAD_CRYPT_SIGN);
	}

	if (fb_seek_set(0) < 0) {
		error("cannot verify signature when -t option is used");
		exit(BAD_CRYPT_SIGN);
	}

	while (length) {
		data = fb_read_block(&fblength);
		if (!data) {
			error("signature verification failed");
			exit(BAD_CRYPT_SIGN);
		}
		if (fblength > length)
			fblength = length;
		if (EVP_DigestVerifyUpdate(ctx, data, fblength) != 1) {
			error("signature verification failed");
			exit(BAD_CRYPT_SIGN);
		}
		length -= fblength;
	}

	if (EVP_DigestVerifyFinal(ctx, sig, siglen) != 1) {
		error("signature verification failed");
		exit(BAD_CRYPT_SIGN);
	}

	EVP_PKEY_free(key);
	EVP_MD_CTX_destroy(ctx);
}

void check_crypto_v3(const struct check_opt *opt)
{
	X509_STORE *store;
	X509 *cert = NULL;
	int validsign = 0;
	struct crypto_v3_header header;
	struct crypto_v3_trailer trailer;
	unsigned long hlen;
	unsigned long pos, prevpos;
	unsigned char *buf;
	uint8_t tag;
	uint32_t len;

	if (!opt->ca) {
		check(opt);
		return;
	}

	OpenSSL_add_all_algorithms();
	store = load_ca(opt->ca);
	load_crl(store, opt->crl);

	if (fb_seek_end(sizeof(trailer)) != 0) {
		error("image not cryptographically enabled");
		exit(NO_CRYPT);
	}
	fb_read(&trailer, sizeof(trailer));
	trailer.hlen = ntohl(trailer.hlen);

	if (trailer.magic != htonl(CRYPTO_V3_TRAILER_MAGIC)) {
#ifdef CONFIG_USER_NETFLASH_CRYPTO_OPTIONAL
		notice("WARNING: no crypto header found");
		check(opt);
		return;
#else
		error("size magic incorrect");
		exit(BAD_CRYPT_MAGIC);
#endif
	}

	if (fb_seek_end(sizeof(trailer) + trailer.hlen) != 0) {
		error("crypt header length invalid");
		exit(BAD_CRYPT_LEN);
	}

	pos = fb_tell();
	hlen = trailer.hlen;

	fb_read(&header, sizeof(header));
	pos += sizeof(header);
	hlen -= sizeof(header);

	if (header.magic != htonl(CRYPTO_V3_MAGIC)) {
		error("crypt magic incorrect");
		exit(BAD_CRYPT_MAGIC);
	}

	if (header.flags != 0) {
		error("unknown crypt flags");
		exit(BAD_CRYPT);
	}

	for (;;) {
		/* Remember the start of this entry, so that we know
		 * how much to hash */
		prevpos = pos;

		/* Seek every time, in case processing needed to
		 * read elsewhere */
		fb_seek_set(pos);

		if (hlen < sizeof(tag)) {
			error("missing tag in crypt data");
			exit(BAD_CRYPT_LEN);
		}
		fb_read(&tag, sizeof(tag));
		pos += sizeof(tag);
		hlen -= sizeof(tag);

		/* Zero tag means end of data (and no length field) */
		if (tag == 0)
			break;

		if (hlen < sizeof(len)) {
			error("missing len in crypt data");
			exit(BAD_CRYPT_LEN);
		}
		fb_read(&len, sizeof(len));
		len = ntohl(len);
		pos += sizeof(len);
		hlen -= sizeof(len);

		if (hlen < len) {
			error("invalid len in crypt data");
			exit(BAD_CRYPT_LEN);
		}
		buf = malloc(len);
		fb_read(buf, len);
		pos += len;
		hlen -= len;

		switch (tag) {
		case CRYPTO_V3_TAG_CERT:
			cert = check_cert(buf, len, store);
			break;
		case CRYPTO_V3_TAG_SIGN_SHA256:
			check_sign_sha256(buf, len, cert, prevpos);
			validsign = 1;
			break;
		default:
			error("unknown crypt data entry");
			exit(BAD_CRYPT_MAGIC);
		}
		free(buf);
	}

	if (hlen) {
		error("unexpected end of crypt data: %d", hlen);
		exit(BAD_CRYPT_LEN);
	}

	if (!validsign) {
		error("missing signature");
		exit(BAD_CRYPT_SIGN);
	}

	/* Keep the signature so it is written to flash. */
	fb_meta_add(sizeof(trailer) + trailer.hlen);

	/* Ignore an extra 4 bytes for the old checksum
	 * (which we don't bother checking) */
	fb_meta_add(4);

	check_version_info(fb_meta_len(), opt->doversion, opt->dohardwareversion, 0, 1, opt->dominimumcheck);
}
