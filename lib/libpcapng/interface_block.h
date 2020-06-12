#ifndef __INTERFACE_BLOCK_H__
#define __INTERFACE_BLOCK_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t link_type;
    uint16_t reserved;
    uint32_t snap_length;
    uint8_t options[0];
} interface_block_t;

bool process_interface_block(interface_block_t const * const interface_block, uint32_t const length);

#endif /* __INTERFACE_BLOCK_H__ */
