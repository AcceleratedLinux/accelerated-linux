/*
 * Copyright (C) 2004  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $ISC-Id:platform.h,v 1.5.12.6 2004/04/19 06:39:56 marka Exp $ */
/* $Id: platform.h,v 1.1 2005/08/05 08:32:32 mcr Exp $ */

#ifndef ISC_PLATFORM_H
#define ISC_PLATFORM_H 1

/*****
 ***** Platform-dependent defines.
 *****/

#undef ISC_PLATFORM_USETHREADS
#define ISC_PLATFORM_NEEDIPV6

/***
 *** Network.
 ***/

#undef  ISC_PLATFORM_HAVESALEN 

#define ISC_PLATFORM_HAVEIN6PKTINFO
#define ISC_PLATFORM_NEEDPORTT
#undef MSG_TRUNC
#define ISC_PLATFORM_NEEDNTOP
#define ISC_PLATFORM_NEEDPTON
#define ISC_PLATFORM_NEEDATON

#define ISC_PLATFORM_QUADFORMAT "I64"

#define ISC_PLATFORM_NEEDSTRSEP
#define ISC_PLATFORM_NEEDSTRLCPY

/*
 * Used to control how extern data is linked; needed for Win32 platforms.
 */
#define ISC_PLATFORM_USEDECLSPEC 1

/*
 * Define this here for now as winsock2.h defines h_errno
 * and we don't want to redeclare it.
 */
#define ISC_PLATFORM_NONSTDHERRNO

 /*
 * Set up a macro for importing and exporting from the DLL
 */

#define LIBISC_EXTERNAL_DATA
#define LIBISCCFG_EXTERNAL_DATA
#define LIBISCCC_EXTERNAL_DATA
#define LIBDNS_EXTERNAL_DATA
#define LIBBIND9_EXTERNAL_DATA

#endif /* ISC_PLATFORM_H */
