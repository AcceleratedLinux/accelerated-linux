/*****************************************************************************
* Copyright 2020, Digi International Inc.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/
#include <limits.h>
#include <sqfs/compressor.h>
#include <sqfs/data_reader.h>
#include <sqfs/dir_reader.h>
#include <sqfs/error.h>
#include <sqfs/id_table.h>
#include <sqfs/inode.h>
#include <sqfs/io.h>
#include <sqfs/predef.h>
#include <sqfs/super.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "fileblock.h"
#include "squashfs.h"
#include "util.h"

struct squashfs_img {
	sqfs_file_t			base;
	sqfs_compressor_t		*cmp;
	sqfs_id_table_t			*idtbl;
	sqfs_dir_reader_t		*dirrd;
	sqfs_data_reader_t		*data;
	sqfs_super_t			super;
	sqfs_compressor_config_t	cfg;
};

struct squashfs_file {
	struct squashfs_img		*img;
	sqfs_inode_generic_t		*inode;
	unsigned long			offset;
	unsigned long			file_length;
};

static void squashfs_func_destroy(sqfs_object_t *base)
{
	struct squashfs_img *f = (struct squashfs_img *)base;

	if (f == NULL)
		return;

	if (f->data)
		sqfs_destroy(f->data);
	if (f->dirrd)
		sqfs_destroy(f->dirrd);
	if (f->idtbl)
		sqfs_destroy(f->idtbl);
	if (f->cmp)
		sqfs_destroy(f->cmp);

	free(f);
}

static sqfs_object_t *squashfs_func_copy(const sqfs_object_t *base)
{
	notice("squashfs obj copy not implemented");

	return NULL;
}

static int squashfs_func_read_at(sqfs_file_t *base, sqfs_u64 offset,
				 void *buffer, size_t size)
{
	int ret;

	ret = fb_seek_set(offset);
	if (ret)
		return SQFS_ERROR_IO;

	ret = fb_read(buffer, size);
	if (ret < 0)
		return SQFS_ERROR_IO;

	return (ret == size) ? 0 : SQFS_ERROR_IO;
}

static int squashfs_func_write_at(sqfs_file_t *base, sqfs_u64 offset,
				  const void *buffer, size_t size)
{
	notice("squashfs write not implemented");

	return SQFS_ERROR_UNSUPPORTED;
}

static sqfs_u64 squashfs_func_get_size(const sqfs_file_t *base)
{
	return fb_len();
}

static int squashfs_func_truncate(sqfs_file_t *base, sqfs_u64 size)
{
	notice("squashfs truncate not implemented");

	return SQFS_ERROR_UNSUPPORTED;
}

struct squashfs_img *squashfs_img_open(void)
{
	int ret;
	struct squashfs_img *img;

	img = calloc(1, sizeof(*img));
	if (img == NULL) {
		error("squashfs: no mem");
		goto err;
	}

	img->base.read_at = squashfs_func_read_at;
	img->base.write_at = squashfs_func_write_at;
	img->base.get_size = squashfs_func_get_size;
	img->base.truncate = squashfs_func_truncate;
	img->base.base.copy = squashfs_func_copy;
	img->base.base.destroy = squashfs_func_destroy;

	ret = sqfs_super_read(&img->super, &img->base);
	if (ret) {
		error("squashfs: failed to read super block");
		goto err;
	}

	sqfs_compressor_config_init(&img->cfg, img->super.compression_id,
				    img->super.block_size,
				    SQFS_COMP_FLAG_UNCOMPRESS);

	ret = sqfs_compressor_create(&img->cfg, &img->cmp);
	if (ret != 0) {
		error("squashfs: failed to create compressor");
		goto err;
	}

	img->idtbl = sqfs_id_table_create(0);
	if (img->idtbl == NULL) {
		error("squashfs: failed to create ID table");
		goto err;
	}

	ret = sqfs_id_table_read(img->idtbl, &img->base, &img->super, img->cmp);
	if (ret) {
		error("squashfs: failed to load ID table");
		goto err;
	}

	img->dirrd = sqfs_dir_reader_create(&img->super, img->cmp, &img->base,
					    0);
	if (img->dirrd == NULL) {
		error("squashfs: failed to create dir reader");
		goto err;
	}

	img->data = sqfs_data_reader_create(&img->base, img->super.block_size,
					    img->cmp, 0);
	if (img->data == NULL) {
		error("squashfs: failed to create data reader");
		goto err;
	}

	ret = sqfs_data_reader_load_fragment_table(img->data, &img->super);
	if (ret) {
		error("squashfs: failed to load fragment table");
		goto err;
	}

	return img;

err:
	sqfs_destroy(img);

	return NULL;
}

void squashfs_img_close(struct squashfs_img *img)
{
	if (img)
		sqfs_destroy(img);
}

struct squashfs_file *squashfs_file_open(struct squashfs_img *img,
					 const char *path)
{
	int ret;
	struct squashfs_file *f;

	f = calloc(1, sizeof(*f));
	if (f == NULL) {
		error("squashfs: no mem on file open");
		return NULL;
	}

	f->img = img;

	ret = sqfs_dir_reader_find_by_path(img->dirrd, NULL, path, &f->inode);
	if (ret) {
		error("squashfs: failed to open file");
		goto err;
	}

	if (!S_ISREG(f->inode->base.mode)) {
		error("squashfs: not a regular file");
		goto err_inode;
	}

	return f;

err_inode:
	free(f->inode);
err:
	free(f);

	return NULL;
}

void squashfs_file_close(struct squashfs_file *file)
{
	if (file) {
		free(file->inode);
		free(file);
	}
}

int squashfs_file_read(struct squashfs_file *file, unsigned long offset,
		       void *buffer, unsigned long len)
{
	sqfs_s32 ret;

	if (!file)
		return -1;

	ret = sqfs_data_reader_read(file->img->data, file->inode, offset,
				    buffer, len);
	if (ret < 0) {
		error("squashfs: failed to read file at offset=%d with size=%d",
		      offset, len);
		return -1;
	}

	return ret;
}
