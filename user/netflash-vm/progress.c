#include <stddef.h>
#include <limits.h>
#include <stdlib.h>

#include "vfile.h"

/*
 * A pass-through filter that calls a callback function whenever
 * information about the read position or the file size changes.
 */
struct vfile_progress {
	struct vfile vfile;
	struct vfile *src;
	void (*cb)(void *d, int n, int max); /* callback function */
	void *d; /* callback function context */
	int max;
	int n;
};

static struct vfile_progress *
to_progress(struct vfile *f)
{
	/* assert(f->ops == &progress_ops); */
	return (struct vfile_progress *)f;
}

static void
progress_close(struct vfile *f)
{
	struct vfile_progress *p = to_progress(f);

	p->cb(p->d, -1, -1);
	vfile_decref(p->src);
	free(f);
}

static int
progress_read(struct vfile *f, void *dst, int sz)
{
	struct vfile_progress *p = to_progress(f);
	int n = vfile_read(p->src, dst, sz);
	if (n > 0) {
		/* Clamp to INT_MAX */
		if (INT_MAX - n < p->n)
			p->n = INT_MAX;
		else
			p->n += n;
		p->cb(p->d, p->n, p->max);
	} else if (n == 0 && p->max < 0) {
		p->max = p->n;
		p->cb(p->d, p->n, p->max);
	}
	return n;
}

static int
progress_tell(struct vfile *f)
{
	struct vfile_progress *p = to_progress(f);
	int n = vfile_tell(p->src);
	if (n >= 0 && n != p->n) {
		p->n = n;
		p->cb(p->d, p->n, p->max);
	}
	return n;
}

static int
progress_getsize(struct vfile *f)
{
	struct vfile_progress *p = to_progress(f);
	int max = vfile_getsize(p->src);
	if (max >= 0 && max != p->max) {
		p->max = max;
		p->cb(p->d, p->n, p->max);
	}
	return max;
}

static int
progress_seek(struct vfile *f, int offset)
{
	struct vfile_progress *p = to_progress(f);
	int n = vfile_seek(p->src, offset);
	if (n >= 0 && n != p->n) {
		p->n = n;
		p->cb(p->d, p->n, p->max);
	}
	return n;
}

const struct vfileops progress_ops = {
	.close = progress_close,
	.read = progress_read,
	.tell = progress_tell,
	.getsize = progress_getsize,
	.seek = progress_seek,
};

struct vfile *
vfile_progress(struct vfile *src, void (*cb)(void *d, int n, int max), void *d)
{
	struct vfile_progress *p;

	if (!src) {
		cb(d, -1, -1);
		return NULL;
	}

	p = calloc(1, sizeof *p);
	if (!p) {
		cb(d, -1, -1);
		vfile_decref(src);
		return NULL;
	}

	VFILE_INIT(&p->vfile, &progress_ops);
	p->src = src;
	p->cb = cb;
	p->d = d;
	p->n = 0;
	p->max = -1;
	p->cb(p->d, p->n, p->max);
	return &p->vfile;
}
