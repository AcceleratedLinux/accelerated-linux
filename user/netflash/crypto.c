#include <config/autoconf.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/sha.h>

#include "crypto.h"
#include "check.h"
#include "fileblock.h"
#include "exit_codes.h"
#include "util.h"

#define CRYPTO_CHECK_OK			0
#define CRYPTO_CHECK_NO_PUBLICKEY	1
#define CRYPTO_CHECK_NO_HEADER		2

#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
static SHA256_CTX sha_ctx;
#else
static MD5_CTX md5_ctx;
#endif

/* Used if the -t option is in effect to calculate the hash as blocks are received */
static unsigned long crypto_hash_total;
static int crypto_hash_init;

static int load_public_key(RSA **pkey)
{
	/* Load public key */
	BIO *in;
	struct stat st;

	if (stat(PUBLIC_KEY_FILE, &st) == -1 && errno == ENOENT) {
#ifdef CONFIG_USER_NETFLASH_CRYPTO_OPTIONAL
		notice("WARNING: can't open public key file %s",
			PUBLIC_KEY_FILE);
		return 0;
#else
		error("can't open public key file %s", PUBLIC_KEY_FILE);
		exit(BAD_PUB_KEY);
#endif
	}
	in = BIO_new(BIO_s_file());
	if (in == NULL) {
		error("cannot allocate a bio structure");
		exit(BAD_DECRYPT);
	}
	if (BIO_read_filename(in, PUBLIC_KEY_FILE) <= 0) {
		error("cannot open public key file");
		exit(BAD_PUB_KEY);
	}
	*pkey = PEM_read_bio_RSA_PUBKEY(in, NULL, NULL, NULL);
	if (*pkey == NULL) {
		error("cannot read public key");
		exit(BAD_PUB_KEY);
	}
	return 1;
}

static int decode_header_info(struct header *hdr, RSA *pkey, int *img_len)
{
	struct little_header lhdr;

	/* Decode header information */
	if (fb_seek_end(sizeof(lhdr)) != 0) {
		error("image not cryptographically enabled");
		exit(NO_CRYPT);
	}
	fb_read(&lhdr, sizeof(lhdr));
	if (lhdr.magic != htons(LITTLE_CRYPTO_MAGIC)) {
#ifdef CONFIG_USER_NETFLASH_CRYPTO_OPTIONAL
		notice("WARNING: no crypto header found\n");
		return 0;
#else
		error("size magic incorrect");
		exit(BAD_CRYPT_MAGIC);
#endif
	}
	{
		unsigned short hlen = ntohs(lhdr.hlen);
		unsigned char tmp[hlen];
		unsigned char t2[hlen];
		int len;

		if (fb_seek_end(sizeof(lhdr) + hlen) != 0) {
			error("crypt header length invalid");
			exit(BAD_CRYPT_LEN);
		}
		fb_read(tmp, hlen);
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
		fb_meta_add(sizeof(lhdr) + hlen);
		*img_len = fb_len() - fb_meta_len();
#else
		fb_trim(sizeof(lhdr) + hlen);
		*img_len = fb_len();
#endif
		len = RSA_public_decrypt(hlen, tmp, t2,
				pkey, RSA_PKCS1_PADDING);
		if (len == -1) {
			error("decrypt failed");
			exit(BAD_DECRYPT);
		}
		if (len != sizeof(struct header)) {
			error("length mismatch %d %d\n", (int)sizeof(struct header), len);
		}
		memcpy(hdr, t2, sizeof(struct header));
	}
	if (hdr->magic != htonl(CRYPTO_MAGIC)) {
		error("image not cryptographically enabled");
		exit(NO_CRYPT);
	}
	return 1;
}

void update_crypto_hash(unsigned char *data, unsigned long length)
{
	if (!crypto_hash_init) {
		crypto_hash_init = 1;
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
		SHA256_Init(&sha_ctx);
#else
		MD5_Init(&md5_ctx);
#endif
	}

#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
	SHA256_Update(&sha_ctx, data, length);
#else
	MD5_Update(&md5_ctx, data, length);
#endif
	crypto_hash_total += length;
}

/*
 *	Check the crypto signature on the image...
 *	This always includes a public key encrypted header and an MD5
 *	(or SHA256) checksum. It optionally includes AES encryption of
 *	the image.
 */
static int check_crypto_signature(void)
{
	struct header hdr;
	int hash_length;
	unsigned long fblength, length;
	unsigned char *data;
	RSA *pkey;
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
	unsigned char hash[SHA256_DIGEST_LENGTH];
#else
	unsigned char hash[MD5_DIGEST_LENGTH];
#endif

	if (!load_public_key(&pkey))
		return CRYPTO_CHECK_NO_PUBLICKEY;
	if (!decode_header_info(&hdr, pkey, &hash_length))
		return CRYPTO_CHECK_NO_HEADER;
	RSA_free(pkey);

	/* Decrypt image if needed */
	if (hdr.flags & FLAG_ENCRYPTED) {
		unsigned char cin[AES_BLOCK_SIZE];
		unsigned char cout[AES_BLOCK_SIZE];
		unsigned long s;
		AES_KEY key;

		if (fb_seek_set(0) < 0) {
			error("Can not decrypt encrypted image when -t option is used.");
			exit(BAD_CRYPT);
		}

		if ((hash_length % AES_BLOCK_SIZE) != 0) {
			error("image size not miscable with cryptography");
			exit(BAD_CRYPT);
		}
		AES_set_decrypt_key(hdr.aeskey, AESKEYSIZE * 8, &key);
		/* Convert the body of the file */
		for (s = 0; s < hash_length; s += AES_BLOCK_SIZE) {
			fb_peek(cin, AES_BLOCK_SIZE);
			AES_decrypt(cin, cout, &key);
			fb_write(cout, AES_BLOCK_SIZE);
		}
	}
	if (hdr.flags & FLAG_CBCENCRYPTED) {
#define CHUNK (AES_BLOCK_SIZE * 128)
		unsigned char iv[AES_BLOCK_SIZE];
		unsigned char cin[CHUNK];
		unsigned char cout[CHUNK];
		long s, len;
		AES_KEY key;

		if (fb_seek_set(0) < 0) {
			error("Can not decrypt encrypted image when -t option is used.");
			exit(BAD_CRYPT);
		}

		if ((hash_length % AES_BLOCK_SIZE) != 0) {
			error("image size not miscable with cryptography");
			exit(BAD_CRYPT);
		}
		AES_set_decrypt_key(hdr.aeskey, AESKEYSIZE * 8, &key);

		hash_length -= AES_BLOCK_SIZE;
		len = (hash_length > CHUNK) ? CHUNK : hash_length;
		for (s = hash_length - len; len > 0; s -= len) {
			if (s == 0) {
				fb_seek_set(hash_length);
				fb_read(iv, AES_BLOCK_SIZE);
			} else {
				fb_seek_set(s - AES_BLOCK_SIZE);
				fb_read(iv, AES_BLOCK_SIZE);
			}
			fb_seek_set(s);
			fb_peek(cin, len);
			AES_cbc_encrypt(cin, cout, len, &key, iv, 0);
			fb_write(cout, len);

			if (s < CHUNK)
				len = s;
		}
		fb_seek_set(hash_length + AES_BLOCK_SIZE);

		/* remove the IV data from then end of the real data */
		fb_trim(AES_BLOCK_SIZE);
	}

	/* Remove padding */
	if (hdr.padsize) {
		hash_length -= hdr.padsize;
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
		fb_meta_add(hdr.padsize);
#else
		fb_trim(hdr.padsize);
#endif
	}

	if (crypto_hash_init) {
		if (crypto_hash_total > hash_length) {
			error("hashed too much, try without -t");
			exit(BAD_MD5_SIG);
		}
		fb_seek_set(crypto_hash_total);
	} else {
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
		SHA256_Init(&sha_ctx);
#else
		MD5_Init(&md5_ctx);
#endif
		fb_seek_set(0);
	}

	while ((data = fb_read_block(&fblength)) != NULL) {
		length = fblength;
		if (length > (hash_length - crypto_hash_total)) {
			length = hash_length - crypto_hash_total;
		}
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
		SHA256_Update(&sha_ctx, data, length);
#else
		MD5_Update(&md5_ctx, data, length);
#endif
		if (length != fblength)
			break;
		crypto_hash_total += length;
	}

#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
	SHA256_Final(hash, &sha_ctx);
	if (memcmp(hdr.hash, hash, SHA256_DIGEST_LENGTH) != 0) {
		error("bad SHA256 signature");
		exit(BAD_MD5_SIG);
	}
#else
	MD5_Final(hash, &md5_ctx);
	if (memcmp(hdr.md5, hash, MD5_DIGEST_LENGTH) != 0) {
		error("bad MD5 signature");
		exit(BAD_MD5_SIG);
	}
#endif

	notice("signed image approved");
	return CRYPTO_CHECK_OK;
}

#ifdef CONFIG_USER_NETFLASH_CRYPTO_V2
void check_crypto_v2(const struct check_opt *opt)
{
	if (opt->dochecksum) {
		chksum();

		if (opt->doversion || opt->dohardwareversion ||
		    opt->doremoveversion || opt->dominimumcheck)
			check_version_info(0, opt->doversion,
					   opt->dohardwareversion,
					   opt->doremoveversion, 1,
					   opt->dominimumcheck);
	}

	/*
	 * Modern signed image support is backward compatible, so we don't
	 * do the crypto check until this point. (That is we have stripped
	 * of old style 16bit checksum and the product/version information).
	 * We also leave the sign structures on the image data, so they get
	 * written to flash as well. However, if it is a gzipped image, we
	 * will need to trim off the signature before we decompress.
	 */
	if (opt->dochecksum) {
		int cryptorc = check_crypto_signature();
		/*
		 * If there is SHA256 or crypto info, there should also be an extra
		 * copy of the version info just before it. (ie. a signed/checksummed
		 * copy.) If we care about version info (and there's a crypto header
		 * present), check this stuff too.
		 */
		if ((opt->doversion || opt->dohardwareversion) && cryptorc == CRYPTO_CHECK_OK) {
			int rc = check_version_info(fb_meta_len(), opt->doversion, opt->dohardwareversion, 0, 0, 0);
			if (rc == 5)
				notice("Warning: no signed version information present in image.");
		}
	}
}
#else
void check_crypto_v1(const struct check_opt *opt)
{
	check_crypto_signature();
	if (opt->dochecksum) {
		chksum();

		if (opt->doversion || opt->dohardwareversion ||
		    opt->doremoveversion || opt->dominimumcheck)
			check_version_info(0, opt->doversion,
					   opt->dohardwareversion,
					   opt->doremoveversion, 1,
					   opt->dominimumcheck);
	}
}
#endif
