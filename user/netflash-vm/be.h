
/*
 * be32toh() and be64toh()
 * Only use this header if your system doesn't have a working <endian.h>.
 * David Leonard, 2020. CC0
 */

#pragma once

#include <stdint.h>

static inline
int
host_is_be(void)
{
	uint16_t n = 0x100;
	return *(unsigned char *)&n;
}

static inline
uint32_t
swap32(uint32_t v)
{
	return ((v << 24) & 0xff000000u)
	     | ((v <<  8) & 0x00ff0000u)
	     | ((v >>  8) & 0x0000ff00u)
	     | ((v >> 24) & 0x000000ffu);
}

static inline
uint64_t
swap64(uint64_t v)
{
	return swap32(v >> 32) | (uint64_t)swap32(v) << 32;
}

static inline
uint32_t
be32toh(uint32_t v)
{
	return host_is_be() ? v : swap32(v);
}

static inline
uint64_t
be64toh(uint64_t v)
{
	return host_is_be() ? v : swap64(v);
}
