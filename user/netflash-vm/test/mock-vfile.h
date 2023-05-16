#pragma once

#include <string.h>
#include "../vfile.h"

#define MOCK_NO_GETSIZE	1
#define MOCK_NO_SEEK    2
#define MOCK_NO_TELL    4

/**
 * Creates a mock vfile that appears to contain repeated src[srcsz].
 * @arg src      Source data
 * @arg srcsz    Size of source data in bytes
 * @arg repeats  Number of times the src appears to be repeated.
 * @arg flasg    Bitwise union of flags: #MOCK_NO_GETSIZE, #MOCK_NO_SEEK
 * @returns vfile that should be released with #vfile_decref(),
 * @returns NULL on error.
 */
struct vfile *mock_vfile_new(const char *src, int srcsz,
    int repeats, int flags);

static inline struct vfile *mock_vfilen(const char *src,
    int repeats, int flags) {
	return mock_vfile_new(src, src ? strlen(src) : 0, repeats, flags);
}

static inline struct vfile *mock_vfile(const char *src) {
	return mock_vfilen(src, 1, 0);
}
