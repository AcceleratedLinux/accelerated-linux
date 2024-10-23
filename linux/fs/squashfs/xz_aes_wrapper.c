// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Squashfs - a compressed read only filesystem for Linux
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010
 * Phillip Lougher <phillip@squashfs.org.uk>
 *
 * xz_aes_wrapper.c
 */

#include <linux/mutex.h>
#include <linux/bio.h>
#include <linux/slab.h>
#include <linux/xz.h>
#include <linux/bitops.h>
#include <crypto/aes.h>

#include "squashfs_fs.h"
#include "squashfs_fs_sb.h"
#include "squashfs.h"
#include "decompressor.h"
#include "page_actor.h"

struct squashfs_xz_aes {
	struct xz_dec *state;
	struct crypto_aes_ctx ctx;
	struct xz_buf buf;
	u8 cipherdata[PAGE_SIZE + AES_BLOCK_SIZE];
	u8 decryptdata[PAGE_SIZE + AES_BLOCK_SIZE];
};

struct disk_comp_opts {
	__le32 dictionary_size;
	__le32 flags;
};

struct comp_opts {
	int dict_size;
};

/*
 * Only support AES-256, so AES key will always be 256 bits (32 bytes).
 * The key is deliberatly left as a global here, so that "other" crypto
 * code can insert a key at early boot time. One future option would be
 * to pass the key in on the kernel command line - but not for now.
 */
#define AESKEYBITS 256
#define AESKEYBYTES (AESKEYBITS / 8)

void get_hw_hash(u8 *hash, int len);
void __weak get_hw_hash(u8 *hash, int len)
{
	pr_warn("SQUASHFS: XZ-AES: no hardware hash available\n");
	memset(hash, 0, len);
}

static void *squashfs_xz_aes_comp_opts(struct squashfs_sb_info *msblk,
	void *buff, int len)
{
	struct disk_comp_opts *comp_opts = buff;
	struct comp_opts *opts;
	int err = 0, n;

	opts = kmalloc(sizeof(*opts), GFP_KERNEL);
	if (opts == NULL) {
		err = -ENOMEM;
		goto out2;
	}

	if (comp_opts) {
		/* check compressor options are the expected length */
		if (len < sizeof(*comp_opts)) {
			err = -EIO;
			goto out;
		}

		opts->dict_size = le32_to_cpu(comp_opts->dictionary_size);

		/* the dictionary size should be 2^n or 2^n+2^(n+1) */
		n = ffs(opts->dict_size) - 1;
		if (opts->dict_size != (1 << n) && opts->dict_size != (1 << n) +
						(1 << (n + 1))) {
			err = -EIO;
			goto out;
		}
	} else
		/* use defaults */
		opts->dict_size = max_t(int, msblk->block_size,
							SQUASHFS_METADATA_SIZE);

	return opts;

out:
	kfree(opts);
out2:
	return ERR_PTR(err);
}


static void *squashfs_xz_aes_init(struct squashfs_sb_info *msblk, void *buff)
{
	struct comp_opts *comp_opts = buff;
	struct squashfs_xz_aes *stream;
	u8 hash[AESKEYBYTES];
	int err;

	stream = kmalloc(sizeof(*stream), GFP_KERNEL);
	if (stream == NULL) {
		err = -ENOMEM;
		goto failed;
	}

	stream->state = xz_dec_init(XZ_PREALLOC, comp_opts->dict_size);
	if (stream->state == NULL) {
		kfree(stream);
		err = -ENOMEM;
		goto failed;
	}

	get_hw_hash(hash, sizeof(hash));
	err = aes_expandkey(&stream->ctx, hash, sizeof(hash));
	if (err) {
		kfree(stream);
		err = -ENOKEY;
		goto failed;

	}

	return stream;

failed:
	ERROR("Failed to initialise xz decompressor\n");
	return ERR_PTR(err);
}


static void squashfs_xz_aes_free(void *strm)
{
	struct squashfs_xz_aes *stream = strm;

	if (stream) {
		xz_dec_end(stream->state);
		kfree(stream);
	}
}

/*
 * Data from the block device was comrpessed and then encrypted, so we
 * need to carry out the reverse. Keep in mind that encrypted data will
 * always be in AES_BLOCK_SIZE chunks (which is 16 bytes for us with AES256).
 * this may not come in whole sized chunks from the block device, so we may
 * need to be able to keep track of partial buffers between reading more
 * from block device.
 */
static int squashfs_xz_aes_uncompress(struct squashfs_sb_info *msblk, void *strm,
	struct bio *bio, int offset, int length,
	struct squashfs_page_actor *output)
{
	struct bvec_iter_all iter_all = {};
	struct bio_vec *bvec = bvec_init_iter_all(&iter_all);
	int total = 0, error = 0;
	int cipherlen, cipherpos;
	struct squashfs_xz_aes *stream = strm;

	xz_dec_reset(stream->state);
	stream->buf.in_pos = 0;
	stream->buf.in_size = 0;
	stream->buf.out_pos = 0;
	stream->buf.out_size = PAGE_SIZE;
	stream->buf.out = squashfs_first_page(output);
	if (IS_ERR(stream->buf.out)) {
		error = PTR_ERR(stream->buf.out);
		goto finish;
	}
	cipherpos = 0;

	for (;;) {
		enum xz_ret xz_aes_err;

		if (stream->buf.in_pos == stream->buf.in_size) {
			void *data;
			int pos, avail;

			if (!bio_next_segment(bio, &iter_all)) {
				/* XZ_STREAM_END must be reached. */
				error = -EIO;
				break;
			}

			avail = min(length, ((int)bvec->bv_len) - offset);
			if (avail > sizeof(stream->cipherdata)) {
				error = -EIO;
				break;
			}

			data = bvec_virt(bvec);
			memcpy(stream->cipherdata + cipherpos, data + offset, avail);

			avail += cipherpos;
			cipherlen = avail / AES_BLOCK_SIZE * AES_BLOCK_SIZE;

			for (pos = 0; pos < cipherlen; pos += AES_BLOCK_SIZE)
				aes_decrypt(&stream->ctx, stream->decryptdata + pos, stream->cipherdata + pos);

			stream->buf.in = stream->decryptdata;
			stream->buf.in_size = cipherlen;
			stream->buf.in_pos = 0;
			length -= cipherlen;
			offset = 0;

			if (cipherlen != avail) {
				/* Copy partial AES_BLOCK of encrypted data */
				cipherpos = avail - cipherlen;
				memcpy(stream->cipherdata, stream->cipherdata + cipherlen, cipherpos);
			}
		}

		if (stream->buf.out_pos == stream->buf.out_size) {
			stream->buf.out = squashfs_next_page(output);
			if (IS_ERR(stream->buf.out)) {
				error = PTR_ERR(stream->buf.out);
				break;
			} else if (stream->buf.out != NULL) {
				stream->buf.out_pos = 0;
				total += PAGE_SIZE;
			}
		}

		xz_aes_err = xz_dec_run(stream->state, &stream->buf);
		if (xz_aes_err == XZ_STREAM_END)
			break;
		if (xz_aes_err != XZ_OK) {
			error = -EIO;
			break;
		}
	}

finish:
	squashfs_finish_page(output);

	return error ? error : total + stream->buf.out_pos;
}

const struct squashfs_decompressor squashfs_xz_aes_comp_ops = {
	.init = squashfs_xz_aes_init,
	.comp_opts = squashfs_xz_aes_comp_opts,
	.free = squashfs_xz_aes_free,
	.decompress = squashfs_xz_aes_uncompress,
	.id = XZ_AES_COMPRESSION,
	.name = "xz-aes",
	.alloc_buffer = 1,
	.supported = 1
};
