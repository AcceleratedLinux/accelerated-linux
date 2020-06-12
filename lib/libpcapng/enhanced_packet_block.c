#include "enhanced_packet_block.h"
#include "block_options.h"
#include "interfaces.h"
#include "pcapng_private.h"
#include "pcapng.h"
#include "utils.h"
#include "block_header.h"
#include "debug.h"

#include <limits.h>

#if defined(UNIT_TEST)
#include "pcapng_unit_test.h"
#endif

static bool process_enhanced_packet_option(void * const context, block_option_t const * const option)
{
    pcapng_packet_t * const packet_info = context;
    bool got_end_of_options = false;

    switch (option->option_type)
    {
        case EPB_FLAGS:
        {
            /* XXX - Need to concern ourselves with alignment? Need to
             * deal with endianness?
             */
            uint32_t const * const epb_flags = (uint32_t *)option->option_data;

            /* Ensure the option length is long enough. */
            if (option->option_length == sizeof *epb_flags)
            {
                packet_info->direction = (*epb_flags) & EPB_DIRECTION_MASK;
            }

            break;
        }

        case END_OF_OPTIONS:
            got_end_of_options = true;
            break;
    }

    return got_end_of_options;
}

static bool process_enhanced_packet_options(pcapng_packet_t * const packet_info, uint8_t * const options, uint32_t const options_length)
{
    return process_block_options(packet_info, options, options_length, process_enhanced_packet_option);
}

static void assign_interface_info(pcapng_packet_t * const packet_info, enhanced_packet_block_t * const packet_block)
{
    interface_t * const interface = interface_get_by_index(packet_block->interface_id);

    if (interface != NULL)
    {
        packet_info->datalink_type = interface_link_type_get(interface);
        packet_info->if_name = interface_name_get(interface);
    }
    else
    {
        packet_info->datalink_type = 0;
        packet_info->if_name = NULL;
    }
}

static void assign_timestamp(pcapng_packet_t * const packet_info, enhanced_packet_block_t * const packet_block)
{
    uint64_t  timestamp;
#define MAKE_64_BIT_TIMESTAMP(high, low)  (((uint64_t)(high) << 32) | (low))

    timestamp = MAKE_64_BIT_TIMESTAMP(packet_block->timestamp_high, packet_block->timestamp_low);

    packet_info->timestamp.tv_sec  = timestamp / USECS_PER_SEC;
    packet_info->timestamp.tv_usec = timestamp % USECS_PER_SEC;
}

bool process_enhanced_packet_block(enhanced_packet_block_t * const packet_block, uint32_t const length, void (* packet_cb)(pcapng_packet_t * const packet, void * const user_context), void * const user_context)
{
    pcapng_packet_t packet_info;
    uint32_t padded_capture_length;
    uint32_t options_length;
    bool processed_ok;

    padded_capture_length = ROUND_UP_TO_32_BIT_BOUNDARY(packet_block->captured_length);

    packet_info.packet_data = packet_block->packet_data;
    packet_info.packet_length = packet_block->packet_length;
    packet_info.captured_length = packet_block->captured_length;

    assign_interface_info(&packet_info, packet_block);
    assign_timestamp(&packet_info, packet_block);

    /* Note - The sizeof(uint32_t) at the end represents the size of
     * the trailing block length field.
     */
    options_length = length - ((sizeof *packet_block) + padded_capture_length + sizeof(uint32_t));

    if (options_length > INT_MAX)
    {
        ERROR("Invalid length %u:%u:%u\n", length, options_length, packet_block->captured_length);
        processed_ok = false;
        goto done;
    }

    if (!process_enhanced_packet_options(&packet_info, packet_block->packet_data + padded_capture_length, options_length))
    {
        processed_ok = false;
        goto done;
    }

    if (packet_cb != NULL)
    {
        packet_cb(&packet_info, user_context);
    }

    processed_ok = true;

done:
    return processed_ok;
}

/**
 * Writes an Enhanced Packet Block (EPB) flags
 */
static int write_epb_flags(file_st * const file, int const direction)
{
    block_option_t option;
    uint32_t epb_flags;
    int retval = -1;

    option.option_type = EPB_FLAGS;
    option.option_length = sizeof epb_flags;

    epb_flags = direction;

    if (file_write(&option, sizeof option, 1, file) != 1)
    {
        goto error;
    }

    if (file_write(&epb_flags, sizeof epb_flags, 1, file) != 1)
    {
        goto error;
    }

    retval = 0;

error:
    return retval;
}

static void enhanced_packet_block_init(enhanced_packet_block_t * const packet_block,
                                       int const if_index,
                                       uint64_t const timestamp,
                                       uint32_t const captured_length,
                                       uint32_t const packet_length)
{
    packet_block->interface_id = if_index;
    packet_block->timestamp_low = timestamp & 0xFFFFFFFFL;
    packet_block->timestamp_high = (timestamp >> 32) & 0xFFFFFFFFL;
    packet_block->captured_length = captured_length;
    packet_block->packet_length = packet_length;
}

static bool write_packet_block(file_st * const file,
                               int const if_index,
                               uint64_t const timestamp,
                               struct pcap_pkthdr const * const phdr)
{
    enhanced_packet_block_t packet_block;
    bool packet_block_written;

    enhanced_packet_block_init(&packet_block, if_index, timestamp, phdr->caplen, phdr->len);

    if (file_write(&packet_block, sizeof packet_block, 1, file) != 1)
    {
        packet_block_written = false;
        goto done;
    }
    packet_block_written = true;

done:
    return packet_block_written;
}

static uint32_t calculate_enhanced_packet_block_length(uint32_t const captured_length)
{
    uint32_t const block_length = sizeof(block_hdr_t)
        + sizeof(enhanced_packet_block_t)
        + ROUND_UP_TO_32_BIT_BOUNDARY(captured_length)
        + EPB_FLAGS_LENGTH
        + END_OF_OPTIONS_LENGTH
        + sizeof block_length;

    return block_length;
}

static bool write_enhanced_packet_block_header(file_st * const file, uint32_t const length)
{
    return write_block_header(file, PCAPNG_ENHANCED_PACKET_BLOCK, length);
}

/**
 * Writes an enhanced packet block to the pcapng file
 */
int pcapng_write_enhanced_packet_block(file_st * const file,
                                uint8_t const * const data,
                                struct pcap_pkthdr const * const phdr,
                                int const direction,
                                int const if_index)
{
    uint64_t timestamp;
    uint32_t block_length;
    int retval = -1;

    if (file == NULL)
    {
        goto error;
    }

    if (data == NULL)
    {
        goto error;
    }

    if (phdr == NULL)
    {
        goto error;
    }

    if (direction != EPB_UNKNOWN && direction != EPB_INBOUND && direction != EPB_OUTBOUND)
    {
        goto error;
    }

    block_length = calculate_enhanced_packet_block_length(phdr->caplen);

    if (!write_enhanced_packet_block_header(file, block_length))
    {
        goto error;
    }

    timestamp = phdr->ts.tv_sec;
    timestamp *= USECS_PER_SEC;
    timestamp += phdr->ts.tv_usec;

    if (!write_packet_block(file, if_index, timestamp, phdr))
    {
        goto error;
    }

    if (file_write(data, phdr->caplen, 1, file) != 1)
    {
        goto error;
    }

    if (add_padding(file, phdr->caplen) < 0)
    {
        goto error;
    }

    if (write_epb_flags(file, direction) < 0)
    {
        goto error;
    }

    if (write_end_of_options(file) < 0)
    {
        goto error;
    }

    if (file_write(&block_length, sizeof block_length, 1, file) != 1)
    {
        goto error;
    }

    retval = 0;

error:
    return retval;
}


