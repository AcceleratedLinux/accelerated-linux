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
#ifndef NETFLASH_UTIL_H
#define NETFLASH_UTIL_H

#include <config/autoconf.h>
#include <stdio.h>
#include <stdint.h>

#define notice(a...) do { fprintf(stdout, "netflash: " a); fprintf(stdout, "\n"); fflush(stdout); } while (0)
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
#define error(a...) do { fprintf(stdout, "netflash: " a); fprintf(stdout, "\n"); fflush(stdout); } while (0)
#else
#define error(a...) do { fprintf(stderr, "netflash: " a); fprintf(stderr, "\n"); fflush(stderr); } while (0)
#endif

extern int time_rate_100;
extern void time_start(void);
extern unsigned long time_elapsed(void);

#if defined(CONFIG_USER_NETFLASH_ATECC508A) || \
    defined(CONFIG_USER_NETFLASH_CRYPTO_ECDSA_SW)
char *arg_from_kernel_cmdline(const char *arg);
void fb_compute_sha256(uint32_t size, unsigned char *sha256);
#endif

#endif
