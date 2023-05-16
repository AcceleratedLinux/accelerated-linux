#include <assert.h>
#include <stdlib.h>

#include "vfile.h"

/* Adapt a source vfile to only "see" a selected range within it.
 * Reads to the range vfile perform reads on the source file. */
struct vfile_range {
	struct vfile vfile;
	struct vfile *src;
	int start;	/* offset of the range view within the src */
	int pos;	/* estimated byte position within src */
	int end;	/* offset of end of the range view within the src */
};

static struct vfile_range *
to_range(struct vfile *f)
{
	/* assert(f->ops = &range_ops); */
        return (struct vfile_range *)f;
}

static void
range_close(struct vfile *f)
{
	struct vfile_range *r = to_range(f);

	vfile_decref(r->src);
	free(r);
}

static int
range_read(struct vfile *f, void *dst, int sz)
{
	struct vfile_range *r = to_range(f);
	int n;

	if (r->pos + sz > r->end)
		sz = r->end - r->pos;
	n = vfile_read(r->src, dst, sz);
	if (n != -1)
		r->pos += n;
	return n;
}

static int
range_tell(struct vfile *f)
{
	struct vfile_range *r = to_range(f);

	return r->pos - r->start;
}

static int
range_getsize(struct vfile *f)
{
	struct vfile_range *r = to_range(f);
	return r->end - r->start;
}

static int
range_seek(struct vfile *f, int offset)
{
	struct vfile_range *r = to_range(f);
	int pos;

	if (offset > r->end - r->start)
		return -1;
	pos = vfile_seek(r->src, offset + r->start);
	if (pos == -1)
		return -1;
	r->pos = pos;
	return pos - r->start;
}

static const struct vfileops range_ops = {
	.close = range_close,
	.read = range_read,
	.tell = range_tell,
	.getsize = range_getsize,
	.seek = range_seek,
};

struct vfile *
vfile_range(struct vfile *src, int start, int len)
{
	struct vfile_range *r;
	if (!src)
		return NULL;

	r = calloc(1, sizeof *r);
	if (!r)
		goto fail;
	VFILE_INIT(&r->vfile, &range_ops);
	r->src = src;
	r->start = start;
	r->end = start + len;

	if (vfile_seek(src, r->start) != r->start)
		goto fail;
	r->pos = r->start;
	return &r->vfile;
fail:
	vfile_decref(src);
	free(r);
	return NULL;
}
