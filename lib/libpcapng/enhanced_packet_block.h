#ifndef __ENHANCED_PACKET_BLOCK_H__
#define __ENHANCED_PACKET_BLOCK_H__

#include "pcapng.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t interface_id;
    uint32_t timestamp_high;
    uint32_t timestamp_low;
    uint32_t captured_length;
    uint32_t packet_length;
    uint8_t packet_data[0];

} enhanced_packet_block_t;


bool process_enhanced_packet_block(enhanced_packet_block_t * const packet_block,
                                   uint32_t const length,
                                   void (* packet_cb)(pcapng_packet_t * const packet, void * const user_context),
                                   void * const user_context);

#endif /* __ENHANCED_PACKET_BLOCK_H__ */
