#ifndef __BLOCK_HEADER_H__
#define __BLOCK_HEADER_H__

#include <stdint.h>

typedef struct
{
	uint32_t  block_type;
	uint32_t  block_length;
} block_hdr_t;

#endif /* __BLOCK_HEADER_H__ */
