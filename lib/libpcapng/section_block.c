#include "section_block.h"
#include "pcapng_private.h"
#include "pcapng.h"
#include "interfaces.h"
#include "block_header.h"
#include "utils.h"
#include "file.h"

void process_section_block(section_block_t const * const section_block, uint32_t const length)
{
    (void)section_block;
    (void)length;

    clear_interfaces();
}

static bool write_section_block_header(file_st * const file, uint32_t const length)
{
    return write_block_header(file, PCAPNG_SECTION_BLOCK, length);
}

static void section_block_init(section_block_t * const section_block)
{
    section_block->byte_order_magic = PCAPNG_MAGIC_NUMBER;
    section_block->major_version = PCAPNG_MAJOR_NUMBER;
    section_block->minor_version = PCAPNG_MINOR_NUMBER;
    section_block->section_length = (uint64_t)-1; /* Indicates that the section length is unspecified. */
}

static bool write_section_block_data(file_st * const file)
{
    bool wrote_section_block;
    section_block_t section_block;

    section_block_init(&section_block);

    if (file_write(&section_block, sizeof(section_block), 1, file) != 1)
    {
        wrote_section_block = false;
        goto done;
    }
    wrote_section_block = true;

done:
    return wrote_section_block;
}

static uint32_t calculate_section_block_length(void)
{
    uint32_t const block_length = sizeof(block_hdr_t) + sizeof(section_block_t) + sizeof(block_length);

    return block_length;
}

bool pcapng_write_section_block(file_st * const file)
{
    uint32_t block_length;
    bool section_written;

    if (file == NULL)
    {
        section_written = false;
        goto done;
    }

    block_length = calculate_section_block_length();

    if (!write_section_block_header(file, block_length))
    {
        section_written = false;
        goto done;
    }

    if (!write_section_block_data(file))
    {
        section_written = false;
        goto done;
    }

    if (file_write(&block_length, sizeof(block_length), 1, file) != 1)
    {
        section_written = false;
        goto done;
    }
    section_written = true;

done:
    return section_written;
}


