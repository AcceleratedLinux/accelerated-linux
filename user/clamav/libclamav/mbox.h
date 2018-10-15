/*
 *  Copyright (C) 2007-2008 Sourcefire, Inc.
 *
 *  Authors: Nigel Horne
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

/* See RFC1521 */
typedef	enum {
	NOMIME, APPLICATION, AUDIO, IMAGE, MESSAGE, MULTIPART, TEXT, VIDEO, MEXTENSION
} mime_type;

typedef enum {
	NOENCODING, QUOTEDPRINTABLE, BASE64, EIGHTBIT, BINARY, UUENCODE, YENCODE, EEXTENSION, BINHEX
} encoding_type;

/* tk: shut up manager.c warning */
#include "clamav.h"

/* classes supported by this system */
typedef enum {
	INVALIDCLASS, BLOBCLASS
} object_type;

#ifdef C_BSD
#define UNIX
#endif

#include "table.h"
#include "blob.h"
#include "line.h"
#include "text.h"
#include "message.h"
#include "uuencode.h"

size_t	strstrip(char *s);	/* remove trailing white space */
int	cli_mbox(const char *dir, int desc, cli_ctx *ctx);
