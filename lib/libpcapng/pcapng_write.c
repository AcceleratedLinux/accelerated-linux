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

#include "pcapng.h"
#include "section_block.h"
#include "file.h"

#include <stdio.h>

/*
 *
 * Returns
 *   true if the maximum file size is reached.
 *   false otherwise
 */
bool pcapng_file_size_exceeds_limit(file_st const * const file, long const limit)
{
    int size_limit_reached;

    if (file == NULL)
    {
        size_limit_reached = true;
        goto done;
    }

    if (file_size_get(file) >= limit)
    {
        errno = EFBIG;
        size_limit_reached = true;
        goto done;
    }

    size_limit_reached = false;

done:
    return size_limit_reached;
}

/**
 * Opens a file, writes in the section block header and return the
 * file descriptor.
 */
file_st * pcapng_file_open(char const * const filename, bool const append_to_file)
{
    file_st * file;
    char const * file_mode = append_to_file ? "a" : "w";

    file = file_open(filename, file_mode);

    return file;
}


