#define _DEFAULT_SOURCE /* for S_IFREG */
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <opengear/og_config.h>
#include <opengear/zipstream.h>
#include "zlib.h"

/*
 * ZIP output stream
 *
 * References:
 *   http://www.pkware.com/documents/casestudies/APPNOTE.TXT
 *   http://fossies.org/linux/zip/proginfo/extrafld.txt
 */

static int zs_unix_to_dosdate(const time_t *unix_time, uint32_t *dos_time);

/* general purpose flags field in zip chunk header */
#define ZS_FLAG_DATADESC	8	/* Data descriptor follows compressed file data */

/* comp_method field in zip chunk headers */
#define ZS_METHOD_RAW		0	/* No compression */
#define ZS_METHOD_DEFLATE	8	/* Deflate compression method */

/* various version fields in zip chunk headers */
#define ZS_MADEBY_UNIX		3
#define ZS_ZIP_VERSION		63	/* 6.3.3 */

struct zipstream {
	zipstream_write_fn *write;
	void *p;
	off_t offset;			/* total written through write() */
	off_t cd_offset;		/* central directory offset */
	z_stream z_stream;
#define BUFFER_SIZE		32768
	char outbuf[BUFFER_SIZE];
	struct entry {
		/* Fields valid before local directory header */
		struct entry *next;
		size_t pathlen;
		uint16_t comp_method;
		uint16_t flags;
		uint32_t dos_mtime;
		uint32_t extattr;
		/* Fields valid after LDH */
		off_t offset;		/* offset of local fileheader */
		off_t data_offset;	/* offset of data section */
		/* Fields updated during zs_write() */
		uint32_t crc32;
		off_t uncomp_size;
		/* Fields valid after zs_end_entry() */
		off_t comp_size;
		char path[1];		/* allocated as path[1 + pathlen] */
	} *first_entry,
	  *last_entry;			/* current entry */
	uint16_t num_entries;
};

/**
 * Converts a unix time into a DOS date:time (UTC).
 *
 * DOS encoded date/times in FAT as:
 *    Date[0:4]   day of month, origin 1
 *    Date[5:8]   month of year, origin 1
 *    Date[9:15]  years since 1980
 *    Time[0:4]   half the number of seconds
 *    Time[5:10]  minutes
 *    Time[11:15] hours since midnight
 */
static int
zs_unix_to_dosdate(const time_t *unix_time, uint32_t *dos_time)
{
	struct tm tm_res, *tm;

	tm = localtime_r(unix_time, &tm_res);
	if (tm == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (tm->tm_year + 1900 < 1980) {
		errno = ERANGE;
		return -1;
	}
	*dos_time = tm->tm_sec >> 1
		  | tm->tm_min << 5
		  | tm->tm_hour << 11
		  | tm->tm_mday << 16
		  | (tm->tm_mon + 1) << (16 + 5)
		  | (tm->tm_year + 1900 - 1980) << (16 + 9)
		  ;
	return 0;
}

/**
 * Write data through the user's supplied write() function,
 * but protects against it from being called with a 0 length.
 * Updates the zs->offset pointer.
 */
static ssize_t
zs_write(struct zipstream *zs, const void *data, size_t datalen)
{
	ssize_t ret = 0;

	if (datalen) {
		ret = zs->write(data, datalen, zs->p);
		if (ret > 0) {
			zs->offset += ret;
		}
	}
	return ret;
}


/* Big-endian header construction functions */

static void
append8(char **p, uint8_t v)
{
	*(*p)++ = v;
}

static void
append16(char **p, uint16_t v)
{
	append8(p, (v >> 0) & 0xff);
	append8(p, (v >> 8) & 0xff);
}

static void
append32(char **p, uint32_t v)
{
	append8(p, (v >>  0) & 0xff);
	append8(p, (v >>  8) & 0xff);
	append8(p, (v >> 16) & 0xff);
	append8(p, (v >> 24) & 0xff);
}

/*
 * Files exceeding 4GB cannot be represented
 * with the 32-bit fields ZIP format.
 */
static int is_too_big_for_zip32(off_t off) {
#if (_FILE_OFFSET_BITS == 64)
	return off >= (off_t)0xffffffff;
#else
	return off == (off_t)0xffffffff;
#endif
}

/*
 * Writes a local file header.
 * This header precedes every data chunk in a ZIP file.
 * Entries in the central directory point back at these LFHs.
 * Because we don't know the size of the data ahead of time,
 * we rely on setting bit 3 of the 'general purpose flag',
 * which allows us to specify that info in a "data descriptor"
 * chunk that is to follow the data chunk.
 *
 * Sets:
 *    entry->offset
 *    entry->data_offset
 *
 * @returns -1 on error
 */
static int
zs_write_local_file_header(struct zipstream *zs, struct entry *entry)
{
	char buf[30], *b;

	if (is_too_big_for_zip32(entry->comp_size) ||
	    is_too_big_for_zip32(entry->uncomp_size) ||
	    entry->pathlen >= 0xffff)
	{
		errno = EINVAL;
		return -1;
	}

	if (entry->flags & ZS_FLAG_DATADESC) {
		entry->crc32 = 0;
		entry->comp_size = 0;
		entry->uncomp_size = 0;
	}

	b = buf;
	append32(&b, 0x04034b50);		/* local file header sig */
	append16(&b, 20);			/* deflated file or folder */
	append16(&b, entry->flags);		/* general purpose bit flag */
	append16(&b, entry->comp_method);	/* compression method */
	append32(&b, entry->dos_mtime);		/* last modified date/time */
	append32(&b, entry->crc32);		/* crc32 */
	append32(&b, entry->comp_size);		/* compressed size */
	append32(&b, entry->uncomp_size);	/* uncompressed size */
	append16(&b, entry->pathlen);		/* filename length */
	append16(&b, 0);			/* extra length */
	assert(b - buf == sizeof buf);

	/* Record local file header offset for later */
	entry->offset = zs->offset;

	/* Write out the header then filename */
	if (zs_write(zs, buf, b - buf) < 0)
	    return -1;
	if (zs_write(zs, entry->path, entry->pathlen) < 0)
	    return -1;

	/* Record data offset for later */
	entry->data_offset = zs->offset;

	return 0;
}

/**
 * Writes the "data descriptor" chunk signaled by ZS_FLAG_DATADESC.
 */
static int
zs_write_data_descriptor(struct zipstream *zs, struct entry *entry)
{
	char buf[16], *b;

	/* Check that the sizes will fit in a 32-bit ZIP format */
	if (((entry->flags & ZS_FLAG_DATADESC) == 0) ||
	    is_too_big_for_zip32(entry->comp_size) ||
	    is_too_big_for_zip32(entry->uncomp_size))
	{
		errno = EINVAL;
		return -1;
	}

	b = buf;
	append32(&b, 0x08074b50);		/* data desciptor signature */
	append32(&b, entry->crc32);		/* CRC-32 */
	append32(&b, entry->comp_size);		/* compressed size */
	append32(&b, entry->uncomp_size);	/* uncompressed size */
	assert(b - buf == sizeof buf);

	if (zs_write(zs, buf, b - buf) < 0)
	    return -1;

	return 0;
}

/**
 * Writes a "Central directory structure" for a given entry
 */
static int
zs_write_central_directory_structure(struct zipstream *zs, struct entry *entry)
{
	char buf[46], *b;

	if (is_too_big_for_zip32(entry->offset)) {
	    errno = EINVAL; /* Should be using ZIP64 */
	    return -1;
	}

	b = buf;
	append32(&b, 0x02014b50);		/* record signature */
	append16(&b, ZS_MADEBY_UNIX << 8 |	/* version made by */
		     ZS_ZIP_VERSION);
	append16(&b, 20);			/* version for extract */
	append16(&b, entry->flags);		/* general purpose bit flag */
	append16(&b, entry->comp_method);	/* compression method */
	append32(&b, entry->dos_mtime);		/* mod file time */
	append32(&b, entry->crc32);		/* CRC-32 */
	append32(&b, entry->comp_size);		/* compressed size */
	append32(&b, entry->uncomp_size);	/* uncompressed size */
	append16(&b, entry->pathlen);		/* filename length */
	append16(&b, 0);			/* extra length */
	append16(&b, 0);			/* comment length */
	append16(&b, 0);			/* disk number start */
	append16(&b, 0);			/* internal file attributes */
	append32(&b, entry->extattr);		/* external file attributes */
	append32(&b, entry->offset);		/* offset to local header */
	assert(b - buf == sizeof buf);

	if (zs_write(zs, buf, b - buf) < 0)
	    return -1;

	/* file name: */
	if (zs_write(zs, entry->path, entry->pathlen) < 0)
	    return -1;

	/* extra field: */
	/* file comment: */

	return 0;
}

static int
zs_write_end_of_central_directory_record(struct zipstream *zs)
{
	char buf[22], *b;
	uint32_t cd_size;

	if (is_too_big_for_zip32(zs->cd_offset)) {
		errno = EINVAL;
		return -1;
	}
	cd_size = zs->offset - zs->cd_offset;

	b = buf;
	append32(&b, 0x06054b50);		/* record signature */
	append16(&b, 0);			/* number of this disk */
	append16(&b, 0);			/* number of the first disk */
	append16(&b, zs->num_entries);		/* #entries in this disk */
	append16(&b, zs->num_entries);		/* total #entries all disks */
	append32(&b, cd_size);			/* size of central dir */
	append32(&b, zs->cd_offset);		/* offset of cde on disk 0 */
	append16(&b, 0);			/* length of comment */
	assert(b - buf == sizeof buf);

	if (zs_write(zs, buf, b - buf) < 0)
	    return -1;

	/* archive comment: */

	return 0;
}

struct zipstream *
zipstream_new(zipstream_write_fn *write, void *p)
{
	struct zipstream *zs;

	zs = malloc(sizeof *zs);
	if (!zs) {
		return NULL;
	}

	zs->write = write;
	zs->p = p;
	zs->first_entry = NULL;
	zs->last_entry = NULL;
	zs->num_entries = 0;
	zs->offset = 0;

	memset(&zs->z_stream, 0, sizeof zs->z_stream);

	return zs;
}

/**
 * Flushes any pending, deflated data from zs->outbuf[] through zs->write().
 * Resets zs->z_stream.avail_in/next_in for immediate use.
 */
static int
zs_flush_deflate_outbuf(struct zipstream *zs)
{
	ssize_t wlen;
	char *data = zs->outbuf;
	size_t datalen = (char *)zs->z_stream.next_out - zs->outbuf;

	while (datalen) {
		wlen = zs_write(zs, data, datalen);
		if (wlen < 0) {
			return -1;
		}
		datalen -= wlen;
		data += wlen;
	}
	zs->z_stream.next_out = (void *)zs->outbuf;
	zs->z_stream.avail_out = sizeof zs->outbuf;

	return 0;
}

/**
 * Finishes up writing the currently compressing file entry.
 */
static int
zs_end_entry(struct zipstream *zs)
{
	struct entry *entry = zs->last_entry;
	int err;

	if (entry->comp_method == ZS_METHOD_DEFLATE) {
		/* Flush the compressor output */
		zs->z_stream.avail_in = 0;
		do {
			err = deflate(&zs->z_stream, Z_FINISH);
			if (zs_flush_deflate_outbuf(zs) == -1)
				return -1;
		} while (err == Z_OK);
		if (err != Z_STREAM_END) {
			return -1;
		}
		if (deflateEnd(&zs->z_stream) != Z_OK) {
			return -1;
		}
	}

	/* Record the size of the data chunk */
	entry->comp_size = zs->offset - entry->data_offset;

	if (entry->flags & ZS_FLAG_DATADESC) {
		if (zs_write_data_descriptor(zs, entry) < 0)
			return -1;;
	}

	return 0;
}

/**
 * Allocate a new in-memory file record.
 * One is created for each file added to the zip stream.
 */
static struct entry *
zs_new_entry(const char *path, const struct stat *stat, int isdir)
{
	struct entry *entry;
	int pathlen;
	time_t mtime;
	size_t entry_size;

	/* Convert NULL paths to "" */
	if (!path) {
		path = "";
	}

	/* Fix paths that begin with '/' */
	while (*path == '/') {
		path++;
	}

	pathlen = strlen(path);

	/* Remove trailing slashes */
	while (pathlen > 0 && path[pathlen - 1] == '/') {
		pathlen--;
	}

	/* Allocate storage for the new file entry */
	entry_size = sizeof *entry +
	             sizeof entry->path[0] * (pathlen + isdir);
	entry = calloc(1, entry_size);
	if (!entry) {
		return NULL;
	}

	memcpy(entry->path, path, pathlen);
	if (isdir) {
		entry->path[pathlen++] = '/';
	}
	entry->pathlen = pathlen;
	/* fwiw, entry->path[] will still be NUL-terminated */

	if (stat) {
		mtime = stat->st_mtime;
	} else {
		mtime = time(0);
	}
	if (zs_unix_to_dosdate(&mtime, &entry->dos_mtime) == -1) {
		// use all zeros on unrepresentable date
		memset(&entry->dos_mtime, 0, sizeof entry->dos_mtime);
	}

	return entry;
}

int
zipstream_entry(struct zipstream *zs, const char *path, const struct stat *stat)
{
	struct entry *entry;
	int ret;
	int isdir = stat && S_ISDIR(stat->st_mode);

	entry = zs_new_entry(path, stat, isdir);
	if (!entry) {
		return -1;
	}

	/* End any current file */
	if (zs->last_entry) {
		ret = zs_end_entry(zs);
		if (ret == -1) {
			free(entry);
			return -1;
		}
	}

	/*
	 * Figure out the file attributes.
	 * The external file attributes for ZS_MADEBY_UNIX
	 * have the two high order bytes containing unix
	 * permissions and file type. Low order bytes contain
	 * MS-DOS FAT file attributes (bit 4 marks a directory)
	 * The top two bits should be zero as they indicate
	 * variant meanings for other fieldsa.
	 */
	entry->extattr = (stat ? stat->st_mode : 0644 | S_IFREG) << 16;
	entry->extattr &= ~(3<<30);

	/* Determine what kind of compression from the stat */
	if (isdir) {
		entry->comp_method = ZS_METHOD_RAW;
		entry->extattr |= 1 << 4;	/* DOS directory flag */
	} else {
		entry->comp_method = ZS_METHOD_DEFLATE;
		entry->flags |= ZS_FLAG_DATADESC;
	}

	if (entry->comp_method == ZS_METHOD_DEFLATE) {
		memset(&zs->z_stream, 0, sizeof zs->z_stream);
		zs->z_stream.next_out = (void *)zs->outbuf;
		zs->z_stream.avail_out = sizeof zs->outbuf;

		ret = deflateInit2(&zs->z_stream,
			Z_DEFAULT_COMPRESSION,	/* level */
			Z_DEFLATED,		/* method */
			-15,			/* windowBits (raw DEFLATE) */
			8,			/* memLevel */
			Z_DEFAULT_STRATEGY);	/* strategy */
		if (ret != Z_OK) {
			errno = (ret == Z_STREAM_ERROR)  ? EINVAL
			      : (ret == Z_MEM_ERROR)     ? ENOMEM
			      :	(ret == Z_VERSION_ERROR) ? ENOTSUP
			      :	                           EIO;
			free(entry);
			return -1;
		}
	}

	/* Append this new entry to the end of the entry list */
	if (!zs->first_entry) {
		zs->first_entry = entry;
	} else {
		zs->last_entry->next = entry;
	}
	entry->next = NULL;
	zs->last_entry = entry;
	zs->num_entries++;

	return zs_write_local_file_header(zs, entry);
}

ssize_t
zipstream_write(struct zipstream *zs, const void *data, size_t datalen)
{
	struct entry *entry = zs->last_entry;
	ssize_t ret = 0;
	int err;

	switch (entry->comp_method) {
	case ZS_METHOD_RAW:
		ret = zs_write(zs, data, datalen);
		break;
	case ZS_METHOD_DEFLATE:
		zs->z_stream.next_in = (void *)data;
		zs->z_stream.avail_in = datalen;
		if (zs->z_stream.avail_in) {
		    do {
			if (zs->z_stream.avail_out == 0 &&
			    zs_flush_deflate_outbuf(zs) == -1)
			{
				return -1;
			}
			err = deflate(&zs->z_stream, Z_NO_FLUSH);
			if (err != Z_OK && err != Z_BUF_ERROR) {
				errno = EIO;
				return -1;
			}
		    } while (zs->z_stream.avail_out == 0);
		}
		break;
	}

	entry->crc32 = crc32(entry->crc32, data, datalen);
	entry->uncomp_size += datalen;

	return ret;
}

/**
 * Writes out the central directory.
 */
static int
zs_write_central_directory(struct zipstream *zs)
{
	struct entry *entry;

	zs->cd_offset = zs->offset;
	for (entry = zs->first_entry; entry; entry = entry->next) {
		if (zs_write_central_directory_structure(zs, entry) < 0)
		    return -1;
	}
	if (zs_write_end_of_central_directory_record(zs) < 0)
		return -1;
	return 0;
}

/**
 * Deallocate the zipstream.
 */
static void
zs_free(struct zipstream *zs)
{
	struct entry *entry;

	(void) deflateEnd(&zs->z_stream);

	while ((entry = zs->first_entry)) {
		zs->first_entry = entry->next;
		free(entry);
	}
	free(zs);
}

int
zipstream_close(struct zipstream *zs)
{
	int ret = -1;

	/* Finish off the last file entry */
	if (zs->last_entry) {
		if (zs_end_entry(zs) < 0)
		    goto done;
	}

	/* Write the central directory structure */
	if (zs_write_central_directory(zs) < 0)
	    goto done;

	/* Signal to write() that we are now done */
	if (zs->write(NULL, 0, zs->p) < 0)
	    goto done;

	ret = 0;

done:
	zs_free(zs);
	return ret;
}

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
