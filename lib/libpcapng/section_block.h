#ifndef __SECTION_BLOCK_H__
#define __SECTION_BLOCK_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


typedef struct
{
    uint32_t byte_order_magic;
    uint16_t major_version;
    uint16_t minor_version;
    uint64_t section_length;
    uint8_t option[0];
} section_block_t; 

void process_section_block(section_block_t const * const section_block, uint32_t const length);

#endif /* __SECTION_BLOCK_H__ */
