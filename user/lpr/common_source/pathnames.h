/*	$OpenBSD: pathnames.h,v 1.4 2003/06/02 23:36:53 millert Exp $	*/
/*	$NetBSD: pathnames.h,v 1.5 1995/11/28 19:43:27 jtc Exp $	*/

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)pathnames.h	8.1 (Berkeley) 6/6/93
 */

#include <paths.h>

#define	_PATH_DEFDEVLP		"/dev/lp0"
#define	_PATH_DEFSPOOL		"/var/spool/lpd"
#define	_PATH_HOSTSEQUIV	"/etc/hosts.equiv"
#define	_PATH_HOSTSLPD		"/etc/hosts.lpd"
#define	_PATH_MASTERLOCK	"/var/run/lpd.pid"
#define	_PATH_PR		"/usr/bin/pr"
#ifdef EMBED
#define _PATH_PRINTCAP		"/etc/config/printcap"
#define _PATH_SOCKETNAME	"/var/run/printer"
#else
#define	_PATH_PRINTCAP		"/etc/printcap"
#define	_PATH_SOCKETNAME	"/dev/printer"
#endif
#define	_PATH_VFONT		"/usr/libdata/vfont/"
#define	_PATH_VFONTB		"/usr/libdata/vfont/B"
#define	_PATH_VFONTI		"/usr/libdata/vfont/I"
#define	_PATH_VFONTR		"/usr/libdata/vfont/R"
#define	_PATH_VFONTS		"/usr/libdata/vfont/S"
