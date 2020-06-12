#include "file.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "file.h"


static long actual_file_size_get(FILE * const fp)
{
    long file_size;
    struct stat st;

    /* Flush out any capture data */
    fflush(fp);

    if (fstat(fileno(fp), &st) < 0)
    {
        /* Error checking the file size, leave errno as is */
        file_size = -1;
        goto done;
    }

    file_size = st.st_size;

done:
    return file_size;
}

long file_size_get(file_st const * const file)
{
    return file->file_size;
}

/**
 * Closes the pcapng file
 */
void pcapng_file_close(file_st * const file)
{
    if (file != NULL)
    {
        if (file->fp != NULL)
        {
            fclose(file->fp);
            file->fp = NULL;
        }
        free(file);
    }
}

file_st * file_open(char const * const filename, char const * const mode)
{
    file_st * file = NULL;
    bool had_error;

    if (filename == NULL)
    {
        had_error = true;
        goto done;
    }

    file = (file_st *)malloc(sizeof *file);
    if (file == NULL)
    {
        had_error = true;
        goto done;
    }

    file->fp = fopen(filename, mode);
    if (file->fp == NULL)
    {
        had_error = true;
        goto done;
    }

    file->file_size = actual_file_size_get(file->fp);
    if (file->file_size < 0)
    {
        had_error = true;
        goto done;
    }

    had_error = false;

done:
    if (had_error)
    {
        pcapng_file_close(file);
        file = NULL;
    }

    return file;
}

size_t file_write(void const * const ptr,
                  size_t const size,
                  size_t const n, file_st * const file)
{
    size_t result;

    if (file != NULL)
    {
        result = fwrite(ptr, size, n, file->fp);
        file->file_size += result * size;
    }
    else
    {
        result = 0;
    }

    return result;
}

int pcapng_file_truncate(file_st * const file, off_t length)
{
    return ftruncate(fileno(file->fp), length);
}
