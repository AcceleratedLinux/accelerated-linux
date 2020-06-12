#include "utils.h"
#include "pcapng_private.h"
#include "block_header.h"

#include <sys/stat.h>
#include <errno.h>

/**
 * Writes up to 3 padding '\0' bytes to the file
 */
int add_padding(file_st * const file, unsigned int const length)
{
    uint8_t pad[3] = { 0, 0, 0 };
    size_t pad_length;
    int retval;

    pad_length = ROUND_UP_TO_32_BIT_BOUNDARY(length) - length;

    if (pad_length > 0)
    {
        if (file_write(pad, pad_length, 1, file) != 1)
        {
            retval = -1;
            goto done;
        }
    }
    retval = 0;

done:
    return retval;
}

bool write_block_header(file_st * const file, uint32_t const type, uint32_t const length)
{
    block_hdr_t block_hdr;
    bool wrote_header;

    block_hdr.block_type = type;
    block_hdr.block_length = length;

    if (file_write(&block_hdr, sizeof block_hdr, 1, file) != 1)
    {
        wrote_header = false;
        goto done;
    }
    wrote_header = true;

done:
    return wrote_header;
}


