#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vfile.h"

/* A chunk is the unit of file data that we read into an in-memory list */
#define CHUNK_SIZE (65536 - 256)
#define UNKNOWN_SIZE (-1)

/* Buffer a source vfile into memory, making it seekable.
 * We will be careful never to call the source's seek() or tell() methods */
struct vfile_membuf {
	struct vfile vfile;
	struct vfile *src;	/* the unbuffered file we'll be reading from */
	int src_size;		/* when UNKNOWN_SIZE, getsize queries fail */
	int src_at_eof : 1;
	struct chunk {
		struct chunk *next;
		char data[CHUNK_SIZE];
	} *chunks;		/* Singly-linked list of same-sized chunks */

	struct chunk *last_chunk; /* Last chunk, and it may be short */
	int last_chunk_off;	/* offset of last_chunk's data */
	int last_chunk_len;	/* number of valid bytes in last chunk */
	int pos;		/* membuf read position; can be at end of pos_chunk */
	struct chunk *pos_chunk;
	int pos_chunk_off;	/* offset of pos_chunk's data */
	/* assert (pos_chunk_off <= pos) */
	/* assert (pos <= pos_chunk_off + CHUNK_SIZE) */
};

static int membuf_seek(struct vfile *f, int pos);

static struct vfile_membuf *
to_membuf(struct vfile *f)
{
	return (struct vfile_membuf *)f;
}

/* Read some chunk data. Returns number of bytes read, or -1 on error */
static int
load_chunk(struct vfile_membuf *mf)
{
	int n;
	struct chunk *chunk;

	if (mf->last_chunk && mf->last_chunk_len < CHUNK_SIZE) {
		/* Top up the last chunk */
		n = vfile_read(mf->src,
		        &mf->last_chunk->data[mf->last_chunk_len],
			CHUNK_SIZE - mf->last_chunk_len);
		if (n == -1)
			return -1;
		if (n == 0)
			mf->src_at_eof = 1;
		mf->last_chunk_len += n;
		return n;
	}

	chunk = malloc(sizeof *chunk);
	if (!chunk)
		return -1;

	n = vfile_read(mf->src, chunk->data, CHUNK_SIZE);
	if (n == -1) {
		free(chunk);
		return -1;
	}
	if (n == 0) {
		free(chunk);
		mf->src_at_eof = 1;
		return 0;
	}

	chunk->next = NULL;
	if (mf->last_chunk) {
		assert(INT_MAX - CHUNK_SIZE > mf->last_chunk_off);
		mf->last_chunk_off += CHUNK_SIZE;
		mf->last_chunk->next = chunk;
	} else
		mf->chunks = chunk;
	mf->last_chunk = chunk;
	mf->last_chunk_len = n;
	return n;
}

static void
membuf_close(struct vfile *f)
{
	struct vfile_membuf *mf = to_membuf(f);
	struct chunk *chunk, *next_chunk;

	vfile_decref(mf->src);

	next_chunk = mf->chunks;
	while ((chunk = next_chunk)) {
		next_chunk = chunk->next;
		free(chunk);
	}
	free(mf);
}

static int
membuf_read(struct vfile *f, void *dst, int sz)
{
	struct vfile_membuf *mf = to_membuf(f);
	int nread = 0;
	char *out = dst;
	int pos = mf->pos;
	struct chunk *chunk;
	int chunk_off;
	int n;

	if (sz < 0) {
		errno = EINVAL;
		return -1;
	}

	/* Remember current pos and seek forward,
	 * which will fill in extra chunks as needed. */
	chunk = mf->pos_chunk;
	chunk_off = mf->pos_chunk_off;
	if (membuf_seek(f, mf->pos + sz) == -1)
		return -1;
	sz = mf->pos - pos;
	if (!sz)
		return 0;

	if (!chunk)
		chunk = mf->chunks;
	else if (pos == chunk_off + CHUNK_SIZE) {
		chunk = chunk->next;
		chunk_off += CHUNK_SIZE;
		/* assert(chunk_off == pos); */
	}
	/* assert(pos < chunk_off + CHUNK_SIZE) */

	n = chunk_off + CHUNK_SIZE - pos;
	if (n > sz)
		n = sz;
	memcpy(out, &chunk->data[pos - chunk_off], n);
	nread += n;
	out += n;
	sz -= n;
	pos += n;
	if (!sz)
		return nread;
	/* assert(pos == chunk_off + CHUNK_SIZE); */

	while (sz >= CHUNK_SIZE) {
		chunk = chunk->next;
		chunk_off += CHUNK_SIZE;
		memcpy(out, chunk->data, CHUNK_SIZE);
		nread += CHUNK_SIZE;
		out += CHUNK_SIZE;
		sz -= CHUNK_SIZE;
		pos += CHUNK_SIZE;
		/* assert(pos == chunk_off + CHUNK_SIZE); */
	}
	/* assert(sz < CHUNK_SIZE) */

	chunk = chunk->next;
	chunk_off += CHUNK_SIZE;
	memcpy(out, chunk->data, sz);
	nread += sz;
	return nread;
}

static int
membuf_tell(struct vfile *f)
{
	struct vfile_membuf *mf = to_membuf(f);

	return mf->pos;
}


static int
membuf_getsize(struct vfile *f)
{
	struct vfile_membuf *mf = to_membuf(f);

	if (mf->src_size == UNKNOWN_SIZE) {
		int src_size = -1;
		/* src_size = vfile_getsize(mf->src); */
		if (src_size == -1) {
			while (!mf->src_at_eof) {
				if (load_chunk(mf) == -1)
					return -1;
			}
			src_size = mf->last_chunk_off + mf->last_chunk_len;
		}
		mf->src_size = src_size;
	}
	return mf->src_size;
}

static int
membuf_seek(struct vfile *f, int pos)
{
	struct vfile_membuf *mf = to_membuf(f);

	if (pos == 0 || pos < mf->pos_chunk_off) {
		/* rewind */
		mf->pos_chunk_off = 0;
		mf->pos_chunk = NULL;
		mf->pos = 0;
		if (pos == 0)
			return pos;
	}
	/* assert(pos > 0); */
	/* assert(pos >= mf->pos_chunk_off) */
	while (!mf->last_chunk || pos > mf->last_chunk_off + mf->last_chunk_len) {
		if (mf->src_at_eof) {
			mf->pos = mf->last_chunk_off + mf->last_chunk_len;
			return mf->pos;
		}
		if (load_chunk(mf) == -1)
			return -1;
	}
	/* assert(mf->last_chunk); */
	/* assert(pos <= mf->last_chunk_off + mf->last_chunk_len); */
	if (pos >= mf->last_chunk_off) {
		mf->pos_chunk = mf->last_chunk;
		mf->pos_chunk_off = mf->last_chunk_off;
		mf->pos = pos;
		return pos;
	}

	/* assert(pos < mf->pos_chunk_off); */
	/* assert(mf->pos_chunk != mf->last_chunk); */
	if (!mf->pos_chunk) {
		mf->pos_chunk_off = 0;
		mf->pos_chunk = mf->chunks;
	}
	while (pos > mf->pos_chunk_off + CHUNK_SIZE) {
		mf->pos_chunk_off += CHUNK_SIZE;
		mf->pos_chunk = mf->pos_chunk->next;
	}
	mf->pos = pos;
	return pos;
}

const struct vfileops membuf_ops = {
	.close = membuf_close,
	.read = membuf_read,
	.tell = membuf_tell,
	.getsize = membuf_getsize,
	.seek = membuf_seek,
};

struct vfile *
vfile_membuf(struct vfile *src)
{
	struct vfile_membuf *mf;

	if (!src)
		return NULL;

	if (src->ops == &membuf_ops) /* don't stack */
		return src;

	mf = calloc(1, sizeof *mf);
	if (!mf) {
		free(mf);
		vfile_decref(src);
		return NULL;
	}

	VFILE_INIT(&mf->vfile, &membuf_ops);
	mf->src = src;
	mf->src_size = UNKNOWN_SIZE;

	return &mf->vfile;
}

