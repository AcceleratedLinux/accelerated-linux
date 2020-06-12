#ifndef __PCAPNG_UNIT_TEST_H__
#define __PCAPNG_UNIT_TEST_H__

#include "file.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

bool get_next_pcapng_block(FILE * const fp, uint32_t * const type, uint8_t * * const block_data, uint32_t * const length);
int write_epb_flags(file_st * const file, int const direction);
int write_interface_name_option(file_st * const file, char const * const interface_name);
int write_interface_time_res_option(file_st * const file);

#endif /* __PCAPNG_UNIT_TEST_H__ */
