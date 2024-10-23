/*
 * Copyright (c) 2010, 2011
 * Phillip Lougher <phillip@lougher.demon.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * xz_aes_aes_wrapper.c
 *
 * Support for XZ (LZMA2) compression using XZ Utils liblzma
 * http://tukaani.org/xz/, followed by AES encryption of data
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lzma.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>

#include "squashfs_fs.h"
#include "xz_wrapper.h"
#include "compressor.h"

static struct bcj bcj[] = {
	{ "x86", LZMA_FILTER_X86, 0 },
	{ "powerpc", LZMA_FILTER_POWERPC, 0 },
	{ "ia64", LZMA_FILTER_IA64, 0 },
	{ "arm", LZMA_FILTER_ARM, 0 },
	{ "armthumb", LZMA_FILTER_ARMTHUMB, 0 },
	{ "sparc", LZMA_FILTER_SPARC, 0 },
	{ NULL, LZMA_VLI_UNKNOWN, 0 }
};

static struct comp_opts comp_opts;

static int filter_count = 1;
static int dictionary_size = 0;
static float dictionary_percent = 0;

/*
 * Only support AES-256, so AES key will always be 256 bits (32 bytes).
 */
#define AES_KEY_BITS 256
#define AES_KEY_BYTES (AES_KEY_BITS / 8)

static char *keystr = NULL;
static char *keyhex = NULL;
static unsigned char key[AES_KEY_BYTES];
static AES_KEY aes_enc_key;
static AES_KEY aes_dec_key;


static int xz_aes_options(char *argv[], int argc)
{
	int i;
	char *name;

	if(strcmp(argv[0], "-Xbcj") == 0) {
		if(argc < 2) {
			fprintf(stderr, "xz-aes: -Xbcj missing filter\n");
			goto failed;
		}

		name = argv[1];
		while(name[0] != '\0') {
			for(i = 0; bcj[i].name; i++) {
				int n = strlen(bcj[i].name);
				if((strncmp(name, bcj[i].name, n) == 0) &&
						(name[n] == '\0' ||
						 name[n] == ',')) {
					if(bcj[i].selected == 0) {
				 		bcj[i].selected = 1;
						filter_count++;
					}
					name += name[n] == ',' ? n + 1 : n;
					break;
				}
			}
			if(bcj[i].name == NULL) {
				fprintf(stderr, "xz-aes: -Xbcj unrecognised "
					"filter\n");
				goto failed;
			}
		}
	
		return 1;
	} else if(strcmp(argv[0], "-Xdict-size") == 0) {
		char *b;
		float size;

		if(argc < 2) {
			fprintf(stderr, "xz-aes: -Xdict-size missing dict-size\n");
			goto failed;
		}

		size = strtof(argv[1], &b);
		if(*b == '%') {
			if(size <= 0 || size > 100) {
				fprintf(stderr, "xz-aes: -Xdict-size percentage "
					"should be 0 < dict-size <= 100\n");
				goto failed;
			}

			dictionary_percent = size;
			dictionary_size = 0;
		} else {
			if((float) ((int) size) != size) {
				fprintf(stderr, "xz-aes: -Xdict-size can't be "
					"fractional unless a percentage of the"
					" block size\n");
				goto failed;
			}

			dictionary_percent = 0;
			dictionary_size = (int) size;

			if(*b == 'k' || *b == 'K')
				dictionary_size *= 1024;
			else if(*b == 'm' || *b == 'M')
				dictionary_size *= 1024 * 1024;
			else if(*b != '\0') {
				fprintf(stderr, "xz-aes: -Xdict-size invalid "
					"dict-size\n");
				goto failed;
			}
		}

		return 1;
	} else if (strcmp(argv[0], "-Xkey") == 0) {
		if(argc < 2) {
			fprintf(stderr, "xz-aes: -Xkey needs a key string\n");
			goto failed;
		}
		keystr = argv[1];
		return 1;
	} else if (strcmp(argv[0], "-XKEY") == 0) {
		if(argc < 2) {
			fprintf(stderr, "xz-aes: -XKEY needs hex key\n");
			goto failed;
		}
		keyhex = argv[1];
		return 1;
	}

	return -1;
	
failed:
	return -2;
}


static unsigned char toxdigit(char c)
{
	if((c >= '0') && (c <= '9'))
		return c - '0';
	if((c >= 'a') && (c <= 'f'))
		return 10 + c - 'a';
	if((c >= 'A') && (c <= 'F'))
		return 10 + c - 'A';
	return 0;
}

/*
 * Parse the key string from the command line. Take no more than
 * AES_KEY_BYTES worth of characters from it.
 */
static void parse_key_string(void)
{
	int n;

	for(n = 0; (n < AES_KEY_BYTES) && (keystr[n] != 0); n++)
		key[n] = keystr[n];
}

/*
 * Parse the hex key string from the command line. Convert the hex digit
 * characters into actual hex bytes. Take no more than AES_KEY_BYTES worth
 * of total bytes.
 */
static void parse_hex_string(void)
{
	int n, i;

	for(n = 0, i = 0; n < AES_KEY_BYTES; n++) {
		if(keyhex[i])
			key[n] = toxdigit(keyhex[i++]);
		else
			break;
		if(keyhex[i])
			key[n] = (key[n] << 4) | toxdigit(keyhex[i++]);
		else
			break;
	}
}

static int xz_aes_options_post(int block_size)
{
	/*
	 * if -Xdict-size has been specified use this to compute the datablock
	 * dictionary size
	 */
	if(dictionary_size || dictionary_percent) {
		int n;

		if(dictionary_size) {
			if(dictionary_size > block_size) {
				fprintf(stderr, "xz-aes: -Xdict-size is larger than"
				" block_size\n");
				goto failed;
			}
		} else
			dictionary_size = block_size * dictionary_percent / 100;

		if(dictionary_size < 8192) {
			fprintf(stderr, "xz-aes: -Xdict-size should be 8192 bytes "
				"or larger\n");
			goto failed;
		}

		/*
		 * dictionary_size must be storable in xz header as either
		 * 2^n or as  2^n+2^(n+1)
	 	*/
		n = ffs(dictionary_size) - 1;
		if(dictionary_size != (1 << n) && 
				dictionary_size != ((1 << n) + (1 << (n + 1)))) {
			fprintf(stderr, "xz-aes: -Xdict-size is an unsupported "
				"value, dict-size must be storable in xz "
				"header\n");
			fprintf(stderr, "as either 2^n or as 2^n+2^(n+1).  "
				"Example dict-sizes are 75%%, 50%%, 37.5%%, "
				"25%%,\n");
			fprintf(stderr, "or 32K, 16K, 8K etc.\n");
			goto failed;
		}

	} else
		/* No -Xdict-size specified, use defaults */
		dictionary_size = block_size;

	if(keystr) {
		parse_key_string();
	} else if (keyhex) {
		parse_hex_string();
	} else {
		fprintf(stderr, "xz-aes: must supply the 256 bit AES key");
		fprintf(stderr, " (with either \"-Xkey\" or \"-XKEY\")\n");
		goto failed;
	}
	AES_set_encrypt_key(key, AES_KEY_BITS, &aes_enc_key);
	AES_set_decrypt_key(key, AES_KEY_BITS, &aes_dec_key);

	return 0;

failed:
	return -1;
}


static void *xz_aes_dump_options(int block_size, int *size)
{
	int flags = 0, i;

	/*
	 * don't store compressor specific options in file system if the
	 * default options are being used - no compressor options in the
	 * file system means the default options are always assumed
	 *
	 * Defaults are:
	 *  metadata dictionary size: SQUASHFS_METADATA_SIZE
	 *  datablock dictionary size: block_size
	 *  1 filter
	 */
	if(dictionary_size == block_size && filter_count == 1)
		return NULL;

	for(i = 0; bcj[i].name; i++)
		flags |= bcj[i].selected << i;

	comp_opts.dictionary_size = dictionary_size;
	comp_opts.flags = flags;

	SQUASHFS_INSWAP_COMP_OPTS(&comp_opts);

	*size = sizeof(comp_opts);
	return &comp_opts;
}


static int xz_aes_extract_options(int block_size, void *buffer, int size)
{
	struct comp_opts *comp_opts = buffer;
	int flags, i, n;

	if(size == 0) {
		/* set defaults */
		dictionary_size = block_size;
		flags = 0;
	} else {
		/* check passed comp opts struct is of the correct length */
		if(size != sizeof(struct comp_opts))
			goto failed;
					 
		SQUASHFS_INSWAP_COMP_OPTS(comp_opts);

		dictionary_size = comp_opts->dictionary_size;
		flags = comp_opts->flags;

		/*
		 * check that the dictionary size seems correct - the dictionary
		 * size should 2^n or 2^n+2^(n+1)
		 */
		n = ffs(dictionary_size) - 1;
		if(dictionary_size != (1 << n) && 
				dictionary_size != ((1 << n) + (1 << (n + 1))))
			goto failed;
	}

	filter_count = 1;
	for(i = 0; bcj[i].name; i++) {
		if((flags >> i) & 1) {
			bcj[i].selected = 1;
			filter_count ++;
		} else
			bcj[i].selected = 0;
	}

	return 0;

failed:
	fprintf(stderr, "xz-aes: error reading stored compressor options from "
		"filesystem!\n");

	return -1;
}


static int xz_aes_init(void **strm, int block_size, int datablock)
{
	int i, j, filters = datablock ? filter_count : 1;
	struct filter *filter = malloc(filters * sizeof(struct filter));
	struct xz_stream *stream;

	if(filter == NULL)
		goto failed;

	stream = *strm = malloc(sizeof(struct xz_stream));
	if(stream == NULL)
		goto failed2;

	stream->filter = filter;
	stream->filters = filters;

	memset(filter, 0, filters * sizeof(struct filter));

	stream->dictionary_size = datablock ? dictionary_size :
		SQUASHFS_METADATA_SIZE;

	filter[0].filter[0].id = LZMA_FILTER_LZMA2;
	filter[0].filter[0].options = &stream->opt;
	filter[0].filter[1].id = LZMA_VLI_UNKNOWN;

	for(i = 0, j = 1; datablock && bcj[i].name; i++) {
		if(bcj[i].selected) {
			filter[j].buffer = malloc(block_size);
			if(filter[j].buffer == NULL)
				goto failed3;
			filter[j].filter[0].id = bcj[i].id;
			filter[j].filter[1].id = LZMA_FILTER_LZMA2;
			filter[j].filter[1].options = &stream->opt;
			filter[j].filter[2].id = LZMA_VLI_UNKNOWN;
			j++;
		}
	}

	return 0;

failed3:
	for(i = 1; i < filters; i++)
		free(filter[i].buffer);
	free(stream);

failed2:
	free(filter);

failed:
	return -1;
}

static int xz_aes_compress(void *strm, void *dest, void *src,  int size,
	int block_size, int *error)
{
	int i, plaintextsize, part, pos;
        lzma_ret res = 0;
	struct xz_stream *stream = strm;
	struct filter *selected = NULL;
	void *plaintext;

	stream->filter[0].buffer = dest;

	for(i = 0; i < stream->filters; i++) {
		struct filter *filter = &stream->filter[i];

        	if(lzma_lzma_preset(&stream->opt, LZMA_PRESET_DEFAULT))
                	goto failed;

		stream->opt.dict_size = stream->dictionary_size;

		filter->length = 0;
		res = lzma_stream_buffer_encode(filter->filter,
			LZMA_CHECK_CRC32, NULL, src, size, filter->buffer,
			&filter->length, block_size);
	
		if(res == LZMA_OK) {
			if(!selected || selected->length > filter->length)
				selected = filter;
		} else if(res != LZMA_BUF_ERROR)
			goto failed;
	}

	if(!selected)
		/*
	 	 * Output buffer overflow.  Return out of buffer space
	 	 */
		return 0;

	/*
	 * Compression has been carried out, next AES encrypt that data.
	 * If the compressed data is already sitting within the "dest"
	 * buffer then copy it out to a temporary buffer first.
	 *
	 * Keep in mind that AES encrption can only be done on whole
	 * AES_BLOCK_SIZE bytes of data. So we may need to pad out the
	 * compressed data to a whole number of AES blocks. We must
	 * return all of that data here, so return with the padded
	 * number of bytes (which of course could be larger than just
	 * the compressed data was).
	 */
	if(selected->buffer == dest) {
		plaintext = alloca(selected->length + AES_BLOCK_SIZE);
		memcpy(plaintext, selected->buffer, selected->length);
	} else {
		plaintext = selected->buffer;
	}

	plaintextsize = selected->length;
	part = plaintextsize % AES_BLOCK_SIZE;
	if(part) {
		/* Pad buffer to AES_BLOCK_SIZE */
		memset(plaintext + plaintextsize, 0, AES_BLOCK_SIZE - part);
		plaintextsize += AES_BLOCK_SIZE - part;
	}

	for(pos = 0; pos < plaintextsize; pos += AES_BLOCK_SIZE)
		AES_encrypt(plaintext + pos, dest + pos, &aes_enc_key);

	return plaintextsize;

failed:
	/*
	 * All other errors return failure, with the compressor
	 * specific error code in *error
	 */
	*error = res;
	return -1;
}


static int xz_aes_uncompress(void *dest, void *src, int size, int block_size,
	int *error)
{
	size_t src_pos = 0;
	size_t plain_pos = 0;
	size_t dest_pos = 0;
	uint64_t memlimit = MEMLIMIT;
	void *plaintext = alloca(block_size);

	/* Should always be a whole number of AES blocks */
	for (src_pos = 0; src_pos < size; src_pos += AES_BLOCK_SIZE)
		AES_decrypt(src + src_pos, plaintext + src_pos, &aes_dec_key);

	lzma_ret res = lzma_stream_buffer_decode(&memlimit, 0, NULL,
			plaintext, &plain_pos, size, dest, &dest_pos, block_size);

	*error = res;
	if (res == LZMA_OK) {
		/* Allow for padded AES block, actual may be slightly smaller */
		if (size - plain_pos < AES_BLOCK_SIZE)
			return (int) dest_pos;
	}

	return -1;
}


void xz_aes_usage()
{
	fprintf(stderr, "\t  -Xbcj filter1,filter2,...,filterN\n");
	fprintf(stderr, "\t\tCompress using filter1,filter2,...,filterN in");
	fprintf(stderr, " turn\n\t\t(in addition to no filter), and choose");
	fprintf(stderr, " the best compression.\n");
	fprintf(stderr, "\t\tAvailable filters: x86, arm, armthumb,");
	fprintf(stderr, " powerpc, sparc, ia64\n");
	fprintf(stderr, "\t  -Xdict-size <dict-size>\n");
	fprintf(stderr, "\t\tUse <dict-size> as the XZ dictionary size.  The");
	fprintf(stderr, " dictionary size\n\t\tcan be specified as a");
	fprintf(stderr, " percentage of the block size, or as an\n\t\t");
	fprintf(stderr, "absolute value.  The dictionary size must be less");
	fprintf(stderr, " than or equal\n\t\tto the block size and 8192 bytes");
	fprintf(stderr, " or larger.  It must also be\n\t\tstorable in the xz");
	fprintf(stderr, " header as either 2^n or as 2^n+2^(n+1).\n\t\t");
	fprintf(stderr, "Example dict-sizes are 75%%, 50%%, 37.5%%, 25%%, or");
	fprintf(stderr, " 32K, 16K, 8K\n\t\tetc.\n");
	fprintf(stderr, "\t  -Xkey <key-string>\n");
	fprintf(stderr, "\t\tUse the plain text string <key-string> as the");
	fprintf(stderr, " AES 256-bit\n\t\tencryption key. The key will be");
	fprintf(stderr, " padded with '0' if required\n\t\tand truncated");
	fprintf(stderr, " past 32 characters if longer than that.\n");
	fprintf(stderr, "\t  -XKEY <hex-string>\n");
	fprintf(stderr, "\t\tUse the hexadecimal <hex-string> digits as the");
	fprintf(stderr, " AES 256-bit\n\t\tencryption key. The key will be");
	fprintf(stderr, " padded with '0' if required\n\t\tand truncated");
	fprintf(stderr, " past 256 bits if longer than that.\n");
}


struct compressor xz_aes_comp_ops = {
	.init = xz_aes_init,
	.compress = xz_aes_compress,
	.uncompress = xz_aes_uncompress,
	.options = xz_aes_options,
	.options_post = xz_aes_options_post,
	.dump_options = xz_aes_dump_options,
	.extract_options = xz_aes_extract_options,
	.usage = xz_aes_usage,
	.id = XZ_AES_COMPRESSION,
	.name = "xz-aes",
	.supported = 1
};
