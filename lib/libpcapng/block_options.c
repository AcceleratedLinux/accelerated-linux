#include "block_options.h"
#include "pcapng_private.h"
#include <inttypes.h>
#include <stdio.h>

bool process_block_options(void * const context, uint8_t const * const options,
                           uint32_t const options_length,
                           bool (* process_option_cb)(void * const context, block_option_t const * const option))
{
    bool processed_ok = true;

    if (options_length > 0)
    {
        uint8_t const * data = options;
        uint32_t length = options_length;
        bool got_last_option = false;

        while (processed_ok && !got_last_option && length >= sizeof(block_option_t))
        {
            block_option_t const * const option = (block_option_t *)data;
            uint32_t const option_length = sizeof(block_option_t) + ROUND_UP_TO_32_BIT_BOUNDARY(option->option_length);

            if (length >= option_length)
            {
                got_last_option = process_option_cb(context, option);

                length -= option_length;
                data += option_length;
            }
            else
            {
                processed_ok = false;
            }
        }
    }

    return processed_ok;
}

/**
 * Write an "end of options" to the pcapng file
 */
int write_end_of_options(file_st * const file)
{
    block_option_t option;
    int retval = -1;

    option.option_type = END_OF_OPTIONS;
    option.option_length = 0;

    if (file_write(&option, sizeof option, 1, file) != 1)
    {
        goto error;
    }

    retval = 0;

error:
    return retval;
}


