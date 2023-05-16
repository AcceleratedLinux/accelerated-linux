#ifndef _OPENGEAR_ZIPSTREAM_H_
#define _OPENGEAR_ZIPSTREAM_H_

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stat;

/*!
 * The state of a streaming ZIP file creator.
 *
 * The memory used is proportional to the number of file entries
 * being streamed (because it will buffer the central directory), plus
 * the default memory requirements of zlib's deflate().
 *
 * To use a zipstream:
 *
 *     zipstream_new()           - allocates memory resources
 *     for each file {
 *         zipstream_entry()     - starts a new file entry
 *         for each data chunk {
 *	       zipstream_write() - append some data to the current file entry
 *         }
 *     }
 *     zipstream_close()         - write trailer and free resources
 */
struct zipstream;

/**
 * Signature of the writer function that zipstream calls to output
 * chunks of raw ZIP file.
 */
typedef ssize_t zipstream_write_fn(const void *data, size_t datalen, void *p);

/**
 * Allocates a new zipstream.
 * No invocations of the write() function will be made during this call.
 * @param write   a writer function that will be called as zip file content is
 *                available. It should return -1 on error.
 *                On completion, the write function will be called with
 *                datalen = 0)
 * @param p       a context passed to the write function each time
 * @returns a pointer that should be released with zipstream_close()
 */
struct zipstream *zipstream_new(zipstream_write_fn *write, void *p);

/**
 * Starts a new ZIP file entry in the stream.
 * It will always be compressed using DEFLATE.
 *
 * @param zs	  an open zipstream
 * @param path    NUL-terminated path name using / as a separator,
 *                or "" to indicate no name (eg stdin)
 *                Leading '/' chars are stripped.
 *                Directories will automatically have '/' appended.
 * @param stat    Optional stat structure that will be used to
 *                initialize the file entry. The following fields
 *                are used: st_mtime to record a timestamp,
 *                          st_mode to see if it's a directory
 *                                  and to store file permissions
 * @returns 0 on success or -1 on error
 */
int zipstream_entry(struct zipstream *zs, const char *path, const struct stat *stat);

/**
 * Adds data to the currently compressing file entry.
 * This should only be called after zipstream_entry(), and
 * can be called multiple times.
 * Might not support more than 4GB of data.
 * @returns number of (uncompressed) bytes compressed, or -1 on error.
 */
ssize_t zipstream_write(struct zipstream *zs, const void *data, size_t datalen);

/**
 * Flushes and releases a zipstream.
 * If no file entries have been added, an EOCD entry is still written.
 * @returns 0 on success, or -1 on error
 */
int zipstream_close(struct zipstream *zs);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_ZIPSTREAM_H_ */

/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */
