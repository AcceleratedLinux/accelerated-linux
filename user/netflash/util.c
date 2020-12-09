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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "util.h"

#if defined(CONFIG_USER_NETFLASH_ATECC508A) || \
    defined(CONFIG_USER_NETFLASH_CRYPTO_ECDSA_SW)
#include <arpa/inet.h>
#include "fileblock.h"
#include "sha256.h"
#endif

static struct timeval time_started;

int time_rate_100 = 25;

void time_start(void)
{
	gettimeofday(&time_started, NULL);
}

/* returns 1/100's off seconds elapsed */
unsigned long time_elapsed(void)
{
	struct timeval now;
	unsigned long val;

	gettimeofday(&now, NULL);

	val = (now.tv_sec - time_started.tv_sec) * 100;
	if (now.tv_usec < time_started.tv_usec) {
		val += (1000000L + now.tv_usec - time_started.tv_usec) / 10000;
		val -= 100;
	} else
		val += (now.tv_usec - time_started.tv_usec) / 10000;

	return val;
}

#if defined(CONFIG_USER_NETFLASH_ATECC508A) || \
    defined(CONFIG_USER_NETFLASH_CRYPTO_ECDSA_SW)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
 * Search argument in the kernel command line
 * @arg: argument's name. Must end with '='. E.g.: "dev_mode="
 *
 * Returns with the argument's value if found, NULL otherwise.
 * Note: the returned string pointer must be freed up after use!
 */
char *arg_from_kernel_cmdline(const char *arg)
{
	FILE *f;
	size_t len;
	size_t n = 0;
	char *line = NULL;
	char *s;
	size_t arglen = strlen(arg);
	char *value = NULL;

	f = fopen("/proc/cmdline", "r");
	if (!f)
		return NULL;

	len = getline(&line, &n, f);
	if (len != -1) {
		s = strtok(line, " \n");
		while (s != NULL) {
			if (strncmp(s, arg, arglen) == 0) {
				value = strdup(s + arglen);
				break;
			}
			s = strtok(NULL, " \n");
		}
		free(line);
	}

	fclose(f);

	return value;
}

#define SHA256_BLOCKSIZE	1024

void fb_compute_sha256(uint32_t size, unsigned char *sha256)
{
	struct sha256_ctx ctx;
	uint8_t buffer[SHA256_BLOCKSIZE];
	int remaining = size;
	int read;

	fb_seek_set(0);

	sha256_init_ctx(&ctx);
	while (remaining > 0) {
		read = fb_read(buffer, MIN(remaining, SHA256_BLOCKSIZE));
		sha256_process_bytes(buffer, read, &ctx);
		remaining -= read;
	}

	sha256_finish_ctx(&ctx, sha256);
}
#endif
