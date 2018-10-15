/*
 *  $Id: libnet-asn1.h,v 1.1.1.1 2000/05/25 00:28:49 route Exp $
 *
 *  libnet-asn1.h - Network routine library ASN.1 header file
 *
 *  Definitions for Abstract Syntax Notation One, ASN.1
 *  As defined in ISO/IS 8824 and ISO/IS 8825
 *
 *  Copyright 1988, 1989 by Carnegie Mellon University
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation, and that the name of CMU not be
 *  used in advertising or publicity pertaining to distribution of the
 *  software without specific, written prior permission.
 *
 *  CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 *  CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 *  SOFTWARE.
 *
 *  Copyright (c) 1998 - 2001 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __LIBNET_ASN1_H
#define __LIBNET_ASN1_H

#ifndef EIGHTBIT_SUBIDS
typedef u_long  oid;
#define MAX_SUBID   0xFFFFFFFF
#else
typedef u_char  oid;
#define MAX_SUBID   0xFF
#endif

#define MAX_OID_LEN         64  /* max subid's in an oid */

#define ASN_BOOLEAN         (0x01)
#define ASN_INTEGER         (0x02)
#define ASN_BIT_STR         (0x03)
#define ASN_OCTET_STR       (0x04)
#define ASN_NULL            (0x05)
#define ASN_OBJECT_ID       (0x06)
#define ASN_SEQUENCE        (0x10)
#define ASN_SET             (0x11)

#define ASN_UNIVERSAL       (0x00)
#define ASN_APPLICATION     (0x40)
#define ASN_CONTEXT         (0x80)
#define ASN_PRIVATE         (0xC0)

#define ASN_PRIMITIVE       (0x00)
#define ASN_CONSTRUCTOR     (0x20)

#define ASN_LONG_LEN        (0x80)
#define ASN_EXTENSION_ID    (0x1F)
#define ASN_BIT8            (0x80)

#define IS_CONSTRUCTOR(byte)  ((byte) & ASN_CONSTRUCTOR)
#define IS_EXTENSION_ID(byte) (((byte) & ASN_EXTENSION_ID) = ASN_EXTENSION_ID)

/*
 *  All of the build_asn1_* (build_asn1_length being an exception) functions
 *  take the same first 3 arguments:
 *
 *  u_char *data:   This is a pointer to the start of the data object to be
 *                  manipulated.
 *  int *datalen:   This is a pointer to the number of valid bytes following
 *                  "data".  This should be not be exceeded in any function.
 *                  Upon exiting a function, this value will reflect the
 *                  changed "data" and then refer to the new number of valid
 *                  bytes until the end of "data".
 *  u_char type:    The ASN.1 object type.
 */


/*
 *  Builds an ASN object containing an integer.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */

u_char *
libnet_build_asn1_int(
    u_char *,           /* Pointer to the output buffer */
    int *,              /* Number of valid bytes left in the buffer */
    u_char,             /* ASN object type */
    long *,             /* Pointer to a long integer */
    int                 /* Size of a long integer */
    );


/*
 *  Builds an ASN object containing an unsigned integer.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */
u_char *
libnet_build_asn1_uint(
    u_char *,           /* Pointer to the output buffer */
    int *,              /* Number of valid bytes left in the buffer */
    u_char,             /* ASN object type */
    u_long *,           /* Pointer to an unsigned long integer */
    int                 /* Size of a long integer */
    );


/*
 *  Builds an ASN object containing an octect string.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */

u_char *
libnet_build_asn1_string(
    u_char *,           /* Pointer to the output buffer */
    int *,              /* Number of valid bytes left in the buffer */
    u_char,             /* ASN object type */
    u_char *,           /* Pointer to a string to be built into an object */
    int                 /* Size of the string */
    );


/*
 *  Builds an ASN header for an object with the ID and length specified.  This
 *  only works on data types < 30, i.e. no extension octets.  The maximum
 *  length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object or
 *  NULL upon error
 */

u_char *
libnet_build_asn1_header(
    u_char *,       /* Pointer to the start of the object */
    int *,          /* Number of valid bytes left in buffer */
    u_char,         /* ASN object type */
    int             /* ASN object length */
    );


u_char *
libnet_build_asn1_length(
    u_char *,       /* Pointer to start of object */
    int *,          /* Number of valid bytes in buffer */
    int             /* Length of object */
    );


/*
 *  Builds an ASN header for a sequence with the ID and length specified.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */

u_char *
libnet_build_asn1_sequence(
    u_char *,
    int *,
    u_char,
    int
    );


/*
 *  Builds an ASN object identifier object containing the input string.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */

u_char *
libnet_build_asn1_objid(
    u_char *,
    int *,
    u_char,
    oid *,
    int
    );


/*
 *  Builds an ASN null object.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */

u_char *
libnet_build_asn1_null(
    u_char *,
    int *,
    u_char
    );


/*
 *  Builds an ASN bitstring.
 *
 *  Returns NULL upon error or a pointer to the first byte past the end of
 *  this object (the start of the next object).
 */

u_char *
libnet_build_asn1_bitstring(
    u_char *,
    int *,
    u_char,
    u_char *,       /* Pointer to the input buffer */
    int             /* Length of the input buffer */
    );


#endif  /* __LIBNET_ASN1_H */

/* EOF */
