#ifndef __FILE_H__
#define __FILE_H__

#include "pcapng.h" /* For file_st */
#include <stddef.h>

struct file_st
{
    FILE * fp;
    long file_size; /* Local storage for file size to avoid having to do ffllush()/fstat() for each packet. */
};

file_st * file_open(char const * const filename, char const * const mode);
long file_size_get(file_st const * const file);
size_t file_write(void const * const ptr,
                  size_t const size,
                  size_t const n, file_st * const file);

#endif /* __FILE_H__ */
