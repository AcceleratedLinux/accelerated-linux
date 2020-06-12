#ifndef __UTILS_H__
#define __UTILS_H__

#include "file.h"

#include <stdbool.h>
#include <stdint.h>

int add_padding(file_st * const file, unsigned int const length);
bool write_block_header(file_st * const file, uint32_t const type, uint32_t const length);


#endif /* __UTILS_H__ */
