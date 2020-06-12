#ifndef __BLOCK_OPTIONS_H__
#define __BLOCK_OPTIONS_H__

#include "file.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t option_type;
    uint16_t option_length;
    uint8_t option_data[0];
} block_option_t;

bool process_block_options(void * const context,
                           uint8_t const * const options,
                           uint32_t const options_length,
                           bool (* process_option_cb)(void * const context, block_option_t const * const option));
int write_end_of_options(file_st * const file);

#endif /* __BLOCK_OPTIONS_H__ */
