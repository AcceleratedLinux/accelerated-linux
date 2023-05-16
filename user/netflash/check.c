#include <config/autoconf.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#ifdef CONFIG_USER_NETFLASH_SHA256
#include <openssl/sha.h>
#endif

#include "check.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "util.h"
#include "versioning.h"
#ifdef CONFIG_USER_NETFLASH_HMACMD5
#include "hmacmd5.h"
#endif

static unsigned int calc_checksum;

void update_chksum(unsigned char *data, int length)
{
	while (length > 0) {
		calc_checksum += *data++;
		length--;
	}
}

/*
 *	Generate a checksum over the data.
 */
void calc_chksum(void)
{
	unsigned char *data;
	unsigned long length;

	calc_checksum = 0;
	fb_seek_set(0);
	while ((data = fb_read_block(&length)) != NULL)
		update_chksum(data, length);
}

/*
 *	Validate the checksum
 */
void chksum(void)
{
	uint32_t file_checksum;
	void *p;

	if (fb_seek_end(sizeof(file_checksum)) != 0) {
		error("image is too short to contain a checksum");
		exit(IMAGE_SHORT);
	}

	fb_read(&file_checksum, sizeof(file_checksum));
	fb_trim(sizeof(file_checksum));
	file_checksum = ntohl(file_checksum);

	for (p = &file_checksum; p < (void *)(&file_checksum + 1); p++)
		calc_checksum -= *(unsigned char *)p;

	calc_checksum = (calc_checksum & 0xffff) + (calc_checksum >> 16);
	calc_checksum = (calc_checksum & 0xffff) + (calc_checksum >> 16);

	if (calc_checksum != file_checksum) {
		error("bad image checksum=0x%04x, expected checksum=0x%04x",
			calc_checksum, file_checksum);
		exit(BAD_CHECKSUM);
	}
}

#ifdef CONFIG_USER_NETFLASH_HMACMD5
static int check_hmac_md5(char *key)
{
	HMACMD5_CTX ctx;
	unsigned char hash[16];
	unsigned char fb_hash[16];
	unsigned char *data;
	unsigned long length;

	if (fb_seek_end(16) == 0) {
		fb_read(fb_hash, 16);
		fb_trim(16);

		HMACMD5Init(&ctx, (unsigned char *)key, strlen(key));
		fb_seek_set(0);
		while ((data = fb_read_block(&length)) != NULL)
			HMACMD5Update(&ctx, data, length);
		HMACMD5Final(hash, &ctx);

		if (memcmp(hash, fb_hash, 16) != 0) {
			error("bad HMAC MD5 signature");
			exit(BAD_HMAC_SIG);
		}

		notice("HMAC MD5 signature ok");
	}

	return 0;
}
#endif

#ifdef CONFIG_USER_NETFLASH_SHA256
static int check_sha256_sum(void)
{
	SHA256_CTX ctx;
	unsigned char hash[32];
	unsigned char fb_hash[32];
	unsigned long hash_length, fblength, length, total = 0;
	unsigned char *data;

	if (fb_seek_end(32) == 0) {
		hash_length = fb_tell();
		fb_read(fb_hash, 32);

		SHA256_Init(&ctx);
		fb_seek_set(0);
		while ((data = fb_read_block(&fblength)) != NULL) {
			length = fblength;
			if (length > (hash_length - total)) {
				length = hash_length - total;
			}
			SHA256_Update(&ctx, data, length);
			if (length != fblength)
				break;
			total += length;
		}
		SHA256_Final(hash, &ctx);

		if (memcmp(hash, fb_hash, 32) != 0) {
			error("bad SHA256 digest");
			exit(BAD_HMAC_SIG);
		}

		notice("SHA256 digest ok");
	}

	/* record the 32-byte offset from the end for later use */
	fb_meta_add(32);

	return 0;
}
#endif

int check_version_info(int offset, int doversion, int dohardwareversion, int removeversion, int failifnoversion, int dominimumcheck)
{
	int rc, len;

	rc = check_vendor(offset, &len);

	if (removeversion)
		fb_trim(len);
	else
		fb_meta_add(len);
#if defined(CONFIG_USER_NETFLASH_MINIMUM_VERSION)
	if( dominimumcheck && rc == 7 )
	{
		error("VERSION - you are trying to upgrade to an "
				"incompatible version.");
		exit(VERSION_INCOMPATIBLE);
	}
#endif

#if defined(CONFIG_USER_NETFLASH_VERSION) || defined(CONFIG_USER_NETFLASH_HARDWARE)
	if (doversion || dohardwareversion) {
		switch (rc){
		case 5:
			if (failifnoversion) {
				error("VERSION - you are trying to load an "
					"image that does not\n         "
					"contain valid version information.");
				exit(NO_VERSION);
			}
		default:
			break;
		}
	}

	if (doversion) {
		switch (rc){
#ifndef CONFIG_USER_NETFLASH_VERSION_ALLOW_CURRENT
		case 3:
			error("VERSION - you are trying to upgrade "
				"with the same firmware\n"
				"         version that you already have.");
			exit(ALREADY_CURRENT);
#endif /* !CONFIG_USER_NETFLASH_VERSION_ALLOW_CURRENT */
#ifndef CONFIG_USER_NETFLASH_VERSION_ALLOW_OLDER
		case 4:
			error("VERSION - you are trying to upgrade "
				"with an older version of\n"
				"         the firmware.");
			exit(VERSION_OLDER);
#endif /* !CONFIG_USER_NETFLASH_VERSION_ALLOW_OLDER */
		case 6:
			error("VERSION - you are trying to load an "
				"image for a different language.");
			exit(BAD_LANGUAGE);
		case 0:
		default:
			break;
		}
	}

	if (dohardwareversion) {
		switch (rc){
		case 1:
			error("VERSION - product name incorrect.");
			exit(WRONG_PRODUCT);
		case 2:
			error("VERSION - vendor name incorrect.");
			exit(WRONG_VENDOR);
		case 0:
		default:
			break;
		}
	}

	return rc;
#endif /* CONFIG_USER_NETFLASH_VERSION || CONFIG_USER_NETFLASH_HARDWARE */
}

void check(const struct check_opt *opt)
{
#ifdef CONFIG_USER_NETFLASH_HMACMD5
	if (opt->hmacmd5key)
		check_hmac_md5(opt->hmacmd5key);
	else
#endif
	if (opt->dochecksum) {
		chksum();

		/*
		 * Check the version information. Checks if the version info is
		 * present and correct, and fails/exits if not. If
		 * 'doremoveversion' is true, will strip the version info as
		 * well.
		 */
		if (opt->doversion || opt->dohardwareversion ||
		    opt->doremoveversion || opt->dominimumcheck)
			check_version_info(0, opt->doversion,
					   opt->dohardwareversion,
					   opt->doremoveversion, 1,
					   opt->dominimumcheck);
	}

#ifdef CONFIG_USER_NETFLASH_SHA256
	/*
	 * To be backword compatible with our images we leave the trailing
	 * old style checksum and product ID "as is". They are stripped of
	 * in the above code. Leaving now the SHA256 checksum. We want to
	 * leave this in place, and have it written to the flash with the
	 * actual image.
	 */
	if (opt->dochecksum && opt->dosha256sum) {
		check_sha256_sum();
		/*
		 * If there is SHA256 or crypto info, there should also be an extra
		 * copy of the version info just before it. (ie. a signed/checksummed
		 * copy.) If we care about version info (and there's a crypto header
		 * present), check this stuff too.
		 */
		if (opt->doversion || opt->dohardwareversion) {
			int rc = check_version_info(fb_meta_len(), opt->doversion,
					opt->dohardwareversion, 0, 0);
			if (rc == 5)
				notice("Warning: no signed version information present in image.");
		}
	}
#endif
}

#ifdef CONFIG_USER_NETFLASH_VERIFY_FW_PRODUCT_INFO
void check_fw_product_info(const struct check_opt *opt)
{
	static const char squashfs_magic[4] = "hsqs";
	char buff[sizeof(squashfs_magic)];
	int ret;

	if (!opt->dohardwareversion)
		return;

	/*
	 * This feature is only available for squashfs-based firmware images.
	 * First 4 characters should match with the squashfs magic. If not,
	 * won't fail, as this is not a firmware image
	 */
	fb_seek_set(0);
	fb_read(buff, sizeof(squashfs_magic));

	if (memcmp(buff, squashfs_magic, sizeof(squashfs_magic)) != 0)
		return;

	ret = check_vendor_from_squashfs();
	if (ret != 0) {
		switch (ret) {
		case -1:
			error("failed to read version file.");
			break;
		case 1:
			error("product name is incorrect.");
			break;
		case 2:
			error("vendor name is incorrect.");
			break;
		default:
			error("unknown error while verifying vendor/product name.");
			break;
		}

		exit(BAD_ROOTFS);
	}

	notice("vendor and product names are verified.");
}
#endif
