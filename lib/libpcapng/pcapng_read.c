/*****************************************************************************
 * Copyright (c) 2019 Digi International Inc., All Rights Reserved
 *
 * This software contains proprietary and confidential information of Digi
 * International Inc.  By accepting transfer of this copy, Recipient agrees
 * to retain this software in confidence, to prevent disclosure to others,
 * and to make no use of this software other than that for which it was
 * delivered.  This is an unpublished copyrighted work of Digi International
 * Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
 * prohibited.
 *
 * Restricted Rights Legend
 *
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
 * Technical Data and Computer Software clause at DFARS 252.227-7031 or
 * subparagraphs (c)(1) and (2) of the Commercial Computer Software -
 * Restricted Rights at 48 CFR 52.227-19, as applicable.
 *
 * Digi International 9350 Excelsior Blvd. Suite 700 Hopkins, MN 55343
 *
 *****************************************************************************/

#include "pcapng_private.h"
#include "pcapng.h"
#include "interfaces.h"
#include "block_header.h"
#include "interface_block.h"
#include "enhanced_packet_block.h"
#include "section_block.h"
#include "debug.h"

#include <limits.h>

#if defined(UNIT_TEST)
#include "pcapng_unit_test.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#define BLOCK_READ_SIZE(length)  ((length) - sizeof (block_hdr_t))

static bool is_supported_block_type(uint32_t const type)
{
    bool supported;

    /* Validate the block type */
    switch (type)
    {
        case PCAPNG_SECTION_BLOCK:
        case PCAPNG_INTERFACE_BLOCK:
        case PCAPNG_ENHANCED_PACKET_BLOCK:
            supported = true;
            break;

        case PCAPNG_SIMPLE_PACKET_BLOCK:
        case PCAPNG_NAME_RESOLUTION_BLOCK:
        case PCAPNG_INTERFACE_STATS_BLOCK:
            supported = false;
            break;

        default:
            /* Don't recognise this block type */
            supported = false;
            break;
    }

    return supported;
}


static bool get_next_pcapng_block(FILE * const fp, uint32_t * const type, uint8_t * * const block_data, uint32_t * const length)
{
    bool got_block = false;
    block_hdr_t block_hdr;
    uint32_t block_length;

    if (fread(&block_hdr, 1, sizeof block_hdr, fp) != sizeof block_hdr)
    {
        if (feof(fp) == 0)
        {
            ERROR("Block header read failed\n");
        }
        goto done;
    }

    *type = block_hdr.block_type;
    *length = block_hdr.block_length;

    if ((*length <= sizeof block_hdr) || (*length >= INT_MAX)) goto done;
    if (!is_supported_block_type(*type)) goto done;

    block_length = BLOCK_READ_SIZE(*length);

    *block_data = (uint8_t *)malloc(block_length);
    if (*block_data == NULL)
    {
        goto done;
    }

    if (fread(*block_data, 1, block_length, fp) != block_length)
    {
        free(*block_data);
        goto done;
    }

    got_block = true;

done:
    return got_block;
}

static bool process_pcapng_block(uint32_t const type, void * const block_data, uint32_t const length, void (* packet_cb)(pcapng_packet_t * packet, void * const user_context), void * const user_context)
{
    bool processed_ok;
    uint32_t const data_length = BLOCK_READ_SIZE(length);

    switch (type)
    {
        case PCAPNG_ENHANCED_PACKET_BLOCK:
            if (length < sizeof(block_hdr_t) + sizeof(enhanced_packet_block_t))
            {
                processed_ok = false;
                goto done;
            }
            if (!process_enhanced_packet_block((enhanced_packet_block_t *)block_data, data_length, packet_cb, user_context))
            {
                processed_ok = false;
                goto done;
            }
            break;

        case PCAPNG_INTERFACE_BLOCK:
            if (length < sizeof(block_hdr_t) + sizeof(interface_block_t))
            {
                processed_ok = false;
                goto done;
            }
            if (!process_interface_block((interface_block_t *)block_data, data_length))
            {
                processed_ok = false;
                goto done;
            }
            break;

        case PCAPNG_SECTION_BLOCK:
            process_section_block((section_block_t *)block_data, data_length);
            break;

        case PCAPNG_SIMPLE_PACKET_BLOCK:
        case PCAPNG_NAME_RESOLUTION_BLOCK:
        case PCAPNG_INTERFACE_STATS_BLOCK:
            /* no break */
        default:
            /* Ignore these block types */
            ERROR("Skipping unsupported block type: %x (%d bytes)\n", type, length);
            break;
    }

    processed_ok = true;

done:
    return processed_ok;
}

int pcapng_fp_read(FILE * const fp, void (* packet_cb)(pcapng_packet_t * packet, void * const user_context), void * const user_context)
{
    int retval;
    uint8_t * block_data;
    uint32_t type = 0;
    uint32_t block_data_length;
    bool had_error = false;

    clear_interfaces();

    while (!had_error && get_next_pcapng_block(fp, &type, &block_data, &block_data_length))
    {
        had_error = !process_pcapng_block(type, block_data, block_data_length, packet_cb, user_context);
        free(block_data);
    }

    clear_interfaces();

    retval = had_error ? -1 : 0;

    return retval;
}

int pcapng_file_read(char const * const filename, void (* packet_cb)(pcapng_packet_t * const packet, void * const user_context), void * const user_context)
{
    int retval;
    FILE * fp = NULL;

    if (filename == NULL)
    {
        retval = -1;
        goto done;
    }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        ERROR("Unable to open %s\n", filename);
        retval = -1;
        goto done;
    }

    retval = pcapng_fp_read(fp, packet_cb, user_context);

done:
    if (fp != NULL)
    {
        fclose(fp);
    }

    return retval;
}



