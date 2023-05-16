#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * A vfile intended for mocking that presents a constant string or
 * byte array as a file. It can also "repeat" that string an arbitrary
 * number of times.
 */

#include "mock-vfile.h"

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }

/* vfile with constant data in it; useful for testing */
struct mock_vfile {
	struct vfile vfile;
	const char *src;
	int srcsz;
	int pos;
	int flags;
	int repeats;
};

static struct mock_vfile *
to_mock_vfile(struct vfile *f)
{
	return (struct mock_vfile *)f;
}

static void
mock_close(struct vfile *f)
{
	struct mock_vfile *mf = to_mock_vfile(f);
	free(mf);
}

static int
mock_read(struct vfile *f, void *dst, int sz)
{
	struct mock_vfile *mf = to_mock_vfile(f);
	int opos = mf->pos;
	char *d = dst;

	sz = min(sz, mf->srcsz * mf->repeats - mf->pos);
	while (sz) {
		int rpos = mf->pos % mf->srcsz;
		int n = min(sz, mf->srcsz - rpos);
		memcpy(d, mf->src + rpos, n);
		sz -= n;
		mf->pos += n;
		d += n;
	}
	return mf->pos - opos;
}

static int
mock_tell(struct vfile *f)
{
	struct mock_vfile *mf = to_mock_vfile(f);

	if (mf->flags & MOCK_NO_TELL) {
		errno = ENOTSUP;
		return -1;
	}
	return mf->pos;
}

static int
mock_getsize(struct vfile *f)
{
	struct mock_vfile *mf = to_mock_vfile(f);
	if (mf->flags & MOCK_NO_GETSIZE) {
		errno = ENOTSUP;
		return -1;
	}
	return mf->srcsz * mf->repeats;
}

static int
mock_seek(struct vfile *f, int pos)
{
	struct mock_vfile *mf = to_mock_vfile(f);

	if (mf->flags & MOCK_NO_SEEK) {
		errno = ENOTSUP;
		return -1;
	}
	if (pos < 0)
		return mf->pos;
	return mf->pos = min(pos, mf->srcsz * mf->repeats);
}

const struct vfileops mock_ops = {
	.close = mock_close,
	.read = mock_read,
	.tell = mock_tell,
	.getsize = mock_getsize,
	.seek = mock_seek,
};

struct vfile *
mock_vfile_new(const char *src, int srcsz, int repeats, int flags)
{
	struct mock_vfile *mf = calloc(1, sizeof *mf);

	if (!mf)
		return NULL;
	VFILE_INIT(&mf->vfile, &mock_ops);
	mf->src = src;
	mf->srcsz = srcsz;
	mf->repeats = repeats;
	mf->flags = flags;
	return &mf->vfile;
}

