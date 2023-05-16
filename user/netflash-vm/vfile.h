#pragma once

/**
 * Abstract virtual file.
 *
 * netflash works with various images files, some streaming, some compressed,
 * some structured. This abstract vfile interface is how we compose file
 * translators.
 *
 * The operations on an abstract vfile are:
 *  - close()   - Release resources, free vfile. Called via #vfile_decref().
 *  - read()    - Reads data into memory. Returns 0 at EOF, -1 on error.
 *  - tell()    - Returns current read position.
 *  - getsize() - Returns total size in bytes (or -1, ENOTSUP)
 *  - seek()    - Changes read position (or returns -1, ENOTSUP)
 *
 * All vfile functions return -1 on error and set #errno.
 *
 * reference counting: Generally, passing a vfile to a construct
 * passes ownership. Initially a vfile's refcnt is 1; only #vfile_decref()
 * decrements it and only #vfile_incref() increments it.
 */
struct vfile;

struct vfileops {
	void (*close)(struct vfile *f);
	int (*read)(struct vfile *f, void *dst, int sz);
	int (*tell)(struct vfile *f);
	int (*getsize)(struct vfile *f);
	int (*seek)(struct vfile *f, int offset);
};

struct vfile {
	const struct vfileops *ops;
	unsigned int refcnt;
};

/** Potentially releases a vfile.
 *  @arg f  The f pointer to decrement the refcount and possibly invalidate.
 *          If @c NULL, then this function has no effect. */
static inline void
vfile_decref(struct vfile *f) {
	if (f && !--f->refcnt)
		f->ops->close(f);
}

/** Increments the reference count of a file */
static inline struct vfile *
vfile_incref(struct vfile *f) {
	if (f)
		++f->refcnt;
	return f;
}

/** Reads data from the vfile, and updates read position.
 *  As many bytes as possible are read, until sz bytes, an error or EOF.
 *  @returns number of bytes read on success
 *  @returns 0 if at end-of-file
 *  @returns -1 on error #errno */
static inline int vfile_read(struct vfile *f, void *dst, int sz) { return f->ops->read(f, dst, sz); }

/** Returns the current read offset within the file.
 *  @returns -1 on error #errno */
static inline int vfile_tell(struct vfile *f) { return f->ops->tell(f); }

/** Returns the size of the vfile in bytes.
 *  @returns -1 on error #errno e.g. #ENOTSUP. */
static inline int vfile_getsize(struct vfile *f) { return f->ops->getsize(f); }

/** Moves the read pointer in the file to the offset.
 *  @arg offset  positive offset from file start
 *  @returns new position on success
 *  @returns -1 on error #errno e.g. #ENOTSUP */
static inline int vfile_seek(struct vfile *f, int offset) { return f->ops->seek(f, offset); }

/** Initialises the base vfile structure.
 *  This macro should only and always be used by vfile implementations,
 *  directly allocating storage for a struct #vfile.
 *  @arg v    pointer to the #vfile being initialised to a refcnt of 1
 *  @arg ops_ pointer to a method dispatch table
 */
#define VFILE_INIT(v, ops_) do { \
		(v)->ops = (ops_); \
		(v)->refcnt = 1; \
	} while (0)


/* An in-memory buffering vfile adapter.
 * Use this if you must have a working seek() or getsize().
 * Special case: if src is already a membuf, behave as identity function. */
struct vfile *vfile_membuf(struct vfile *src);

/* A wrapper that presents a subrange as an independent file. */
struct vfile *vfile_range(struct vfile *src, int start, int len);

/* A regular unix file adapter. */
struct vfile *vfile_fd(int fd);

/* A progress adapter that calls cb as read/seek/getsize operations complete. */
struct vfile *vfile_progress(struct vfile *src,
	void (*cb)(void *d, int pos, int max), void *d);

/* A pipe/fork/execvp adapter that accesses the command's stdout. */
struct vfile *vfile_proc(const char *file, char *const argv[]);

/* Access the data within a qcow2 image file */
struct vfile *vfile_qcow2(struct vfile *src);

/* Access a DOS partition from a disk image */
struct vfile *vfile_dospart(struct vfile *src, unsigned int partno);
