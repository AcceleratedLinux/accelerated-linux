#include "interface_block.h"
#include "block_options.h"
#include "interfaces.h"
#include "pcapng_private.h"
#include "pcapng.h"
#include "utils.h"
#include "block_header.h"
#include "file.h"

#include <string.h>

static bool process_interface_block_option(void * const context, block_option_t const * const option)
{
    interface_t * const interface = context;
    bool got_end_of_options = false;

    switch (option->option_type)
    {
        case IF_NAME:
        {
            char const * const interface_name = (char *)option->option_data;

            interface_name_set(interface, interface_name, (size_t)option->option_length);
            break;
        }

        case END_OF_OPTIONS:
            got_end_of_options = true;
            break;
    }

    return got_end_of_options;
}

static bool process_interface_block_options(interface_t * const interface, uint8_t const * const options, uint32_t const options_length)
{
    return process_block_options(interface, options, options_length, process_interface_block_option);
}

bool process_interface_block(interface_block_t const * const interface_block, uint32_t const length)
{
    uint8_t const * options_ptr;
    int options_length;
    interface_t * const interface = interface_get_next_free();
    bool processed_ok;

    if (interface == NULL)
    {
        processed_ok = false;
        goto done;
    }
    interface_link_type_set(interface, interface_block->link_type);
    interface_snap_length_set(interface, interface_block->snap_length);

    options_ptr = interface_block->options;
    /* The sizeof(uint32_t) represents the size of the block
     * length field at the end of the block.
     */
    options_length = length - (sizeof *interface_block + sizeof(uint32_t));

    if (!process_interface_block_options(interface, options_ptr, options_length))
    {
        processed_ok = false;
        goto done;
    }

    processed_ok = true;

done:
    return processed_ok;
}

static bool write_interface_block_header(file_st * const file, uint32_t const length)
{
    return write_block_header(file, PCAPNG_INTERFACE_BLOCK, length);
}

/**
 * Write an interface name option to the pcapng file
 */
static int write_interface_name_option(file_st * const file, char const * const interface_name)
{
    block_option_t option;
    int retval = -1;

    option.option_type = IF_NAME;
    option.option_length = strlen(interface_name);

    if (file_write(&option, sizeof option, 1, file) != 1)
    {
        goto error;
    }

    if (file_write(interface_name, option.option_length, 1, file) != 1)
    {
        goto error;
    }

    if (add_padding(file, option.option_length) < 0)
    {
        goto error;
    }

    retval = 0;

error:
    return retval;
}

static uint32_t calculate_interface_block_length(char const * const interface_name)
{
    uint32_t const block_length = sizeof(block_hdr_t)
        + sizeof(interface_block_t)
        + sizeof(block_option_t)
        + ROUND_UP_TO_32_BIT_BOUNDARY(strlen(interface_name))
        + TIME_RES_OPTION_LENGTH
        + sizeof(block_option_t)
        + sizeof block_length;

    return block_length;
}

/**
 * Write an interface time resolution option to the pcapng file
 */
static int write_interface_time_res_option(file_st * const file)
{
    block_option_t option;
    uint8_t const time_resolution = 6;  // i.e. microseconds
    int retval = -1;

    option.option_type = IF_TSRESOL;
    option.option_length = 1;

    if (file_write(&option, sizeof option, 1, file) != 1)
    {
        goto error;
    }

    if (file_write(&time_resolution, sizeof time_resolution, 1, file) != 1)
    {
        goto error;
    }

    if (add_padding(file, sizeof time_resolution) < 0)
    {
        goto error;
    }

    retval = 0;

error:
    return retval;
}

int pcapng_write_interface_block(file_st * const file,
                          char const * const ifname,
                          uint16_t const link_type,
                          uint32_t const snap_length)
{
    interface_block_t if_block;
    uint32_t block_length;
    int retval = -1;

    if (file == NULL)
    {
        goto error;
    }

    if (ifname == NULL)
    {
        goto error;
    }

    if (ifname[0] == '\0')
    {
        goto error;
    }

    block_length = calculate_interface_block_length(ifname);

    if (!write_interface_block_header(file, block_length))
    {
        goto error;
    }

    if_block.link_type = link_type;
    if_block.reserved = 0;
    if_block.snap_length = snap_length;

    if (file_write(&if_block, sizeof if_block, 1, file) != 1)
    {
        goto error;
    }

    if (write_interface_name_option(file, ifname) < 0)
    {
        goto error;
    }

    if (write_interface_time_res_option(file) < 0)
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


