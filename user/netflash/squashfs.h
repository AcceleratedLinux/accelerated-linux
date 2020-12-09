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

#ifndef NETFLASH_SQUASHFS_H
#define NETFLASH_SQUASHFS_H

struct squashfs_img;
struct squashfs_file;

struct squashfs_img *squashfs_img_open(void);
void squashfs_img_close(struct squashfs_img *img);
struct squashfs_file *squashfs_file_open(struct squashfs_img *img,
					 const char *path);
void squashfs_file_close(struct squashfs_file *file);
int squashfs_file_read(struct squashfs_file *file, unsigned long offset,
		       void *buffer, unsigned long len);

#endif /* NETFLASH_SQUASHFS_H */
