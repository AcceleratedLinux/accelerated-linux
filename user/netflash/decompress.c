#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <arpa/inet.h>

#include <config/autoconf.h>

#include "decompress.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "util.h"

int doinflate;		/* Decompress the image */

static z_stream z;
static unsigned long zoffset;

static void decompress_skip_bytes(unsigned long len)
{
	if (fb_len() - fb_tell() < len) {
		error("compressed image is too short");
		exit(IMAGE_SHORT);
	}
	fb_seek_inc(len);
}

static void decompress_read(void *p, int len)
{
	if (fb_len() - fb_tell() < len) {
		error("compressed image is too short");
		exit(IMAGE_SHORT);
	}
	fb_read(p, len);
}

static unsigned long decompress_init(void)
{
	uint8_t method, flg, c;
	uint16_t xlen;
	uint32_t size;
	unsigned long length;

	fb_seek_set(0);

	/* Skip over gzip header */
	decompress_skip_bytes(2);

	decompress_read(&method, 1);
	if (method != 8) {
		error("image is compressed, unknown compression method");
		exit(UNKNOWN_COMP);
	}

	decompress_read(&flg, 1);

	/* Skip mod time, extended flag, and os */
	decompress_skip_bytes(6);

	/* Skip extra field */
	if (flg & 0x04) {
		decompress_read(&xlen, 2);
		xlen = ntohs(bswap_16(xlen));
	}

	/* Skip file name */
	if (flg & 0x08) {
		do {
			decompress_read(&c, 1);
		} while (c);
	}

	/* Skip comment */
	if (flg & 0x10) {
		do {
			decompress_read(&c, 1);
		} while (c);
	}

	/* Skip CRC */
	if (flg & 0x02) {
		decompress_skip_bytes(2);
	}

	z.next_in = fb_read_block(&length);
	if (!z.next_in) {
		error("unexpected end of file for decompression");
		exit(BAD_DECOMP);
	}
	z.avail_in = length;
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	zoffset = fb_tell();
	if (inflateInit2(&z, -MAX_WBITS) != Z_OK) {
		error("image is compressed, decompression failed");
		exit(BAD_DECOMP);
	}

	/*
	 * Check the size field at end of archive data stream.
	 * Minimum gzip header is 10 bytes, then we need 4 bytes for length.
	 */
	if (fb_len() >= 10 + 4) {
		fb_seek_end(4);
		fb_read(&size, 4);
		size = ntohl(bswap_32(size));
	} else {
		size = 0;
	}
	if (size <= 0) {
		error("image is compressed, decompressed length is invalid");
		exit(BAD_DECOMP);
	}

	return size;
}


int decompress(void *data, int length)
{
	unsigned long fblength;
	int rc;

	z.next_out = data;
	z.avail_out = length;

	fb_seek_set(zoffset);
	for (;;) {
		if (z.avail_in == 0) {
			z.next_in = fb_read_block(&fblength);
			if (!z.next_in) {
				error("unexpected end of file for decompression");
				exit(BAD_DECOMP);
			}
			z.avail_in = fblength;
			zoffset = fb_tell();
		}

		rc = inflate(&z, Z_SYNC_FLUSH);
		if (rc == Z_OK) {
			if (z.avail_out == 0)
				return length;

			if (z.avail_in != 0) {
				/* Note: This shouldn't happen, but if it does then
				 * need to add code to add another level of buffering
				 * that we append file blocks to...
				 */
				error("decompression deadlock");
				exit(BAD_DECOMP);
			}
		}
		else if (rc == Z_STREAM_END) {
			return length - z.avail_out;
		}
		else {
			error("error during decompression: %x", rc);
			exit(BAD_DECOMP);
		}
	}
}

unsigned long check_decompression(void)
{
	uint8_t gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
	uint8_t header[2];
	unsigned long len;

#ifndef CONFIG_USER_NETFLASH_AUTODECOMPRESS
	if (!doinflate)
		goto noinflate;
#endif

	if (fb_len() < 2)
		goto noinflate;
	if (fb_seek_set(0) != 0) /* this can happen for dothrow */
		goto noinflate;
	if (fb_read(header, 2) != 2)
		goto noinflate;
	if (memcmp(header, gz_magic, 2) != 0)
		goto noinflate;

	fb_meta_trim();
	len = decompress_init();
	notice("image is compressed, decompressed length=%lu\n", len);
	return len;

noinflate:
	return fb_len();
}
