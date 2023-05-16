/*
 * Open and read from a qcow2 image file.
 * David Leonard, 2020. CC0
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>
#include <err.h>

#if defined(NO_ENDIAN_H)
# include "be.h"
#else
# include <endian.h>
#endif

#include "vfile.h"

/* Cluster kinds, used for the cache */
#define KIND_L2		0
#define KIND_DATA	1
#define KIND_MAX	2

/* Access the raw image data of a QCOW2 file */
struct vfile_qcow2 {
	struct vfile vfile;
	struct vfile *src;
	uint64_t pos;

	/* extracted from file header */
	uint64_t size;
	uint64_t *l1_table;
	size_t l1_table_size;
	size_t cluster_size;
	unsigned int l1_shift;
	unsigned int l2_amask;
	unsigned int l2_shift;

	/* cluster cache */
	struct cluster {
		void *base;
		uint64_t offset;	/* 0 when unused */
	} cluster[KIND_MAX];
};

/* Endian conversion with unaligned pointers */

static uint32_t
be32tohp(const void *p) {
	uint32_t v;
	memcpy(&v, p, sizeof v);
	return be32toh(v);
}

static uint64_t
be64tohp(const void *p) {
	uint64_t v;
	memcpy(&v, p, sizeof v);
	return be64toh(v);
}

static struct vfile_qcow2 *
to_qcow2(struct vfile *f)
{
	return (struct vfile_qcow2 *)f;
}

/* Loads a cluster, if it isn't already cached */
static const void *
load_cluster(struct vfile_qcow2 *q, uint64_t offset, unsigned kind)
{
	if (!offset) {
		errno = EINVAL; /* There is no cluster 0 */
		return NULL;
	}

	struct cluster *c = &q->cluster[kind];
	if (c->offset == offset)
		return c->base;	/* Cache hit */

	if (vfile_seek(q->src, offset) == -1)
		return NULL;
	c->offset = 0;
	if (vfile_read(q->src, c->base, q->cluster_size) != q->cluster_size)
		return NULL;
	c->offset = offset;
	return c->base;
}

static void
qcow2_close(struct vfile *f)
{
	struct vfile_qcow2 *q = to_qcow2(f);
	unsigned kind;

	if (!q)
		return;
	for (kind = 0; kind < KIND_MAX; kind++)
		free(q->cluster[kind].base);
	free(q->l1_table);
	vfile_decref(q->src);
	free(q);
}

static int
qcow2_getsize(struct vfile *f)
{
	struct vfile_qcow2 *q = to_qcow2(f);
	return q->size;
}

static int
qcow2_tell(struct vfile *f)
{
	struct vfile_qcow2 *q = to_qcow2(f);
	return q->pos;
}

static int
qcow2_seek(struct vfile *f, int pos)
{
	struct vfile_qcow2 *q = to_qcow2(f);
	if (pos < 0 || pos > q->size) {
		errno = EINVAL;
		return -1;
	}
	q->pos = pos;
	return pos;
}

static int
qcow2_read(struct vfile *f, void *dest, int len)
{
	struct vfile_qcow2 *q = to_qcow2(f);
	int ret = 0;
	uint64_t offset = q->pos;

	/* Ensure read within bounds */
	if (offset >= q->size)
		len = 0;
	else if (len > q->size - offset)
		len = q->size - offset;

	while (len) {
		unsigned int l2_index = (offset >> q->l2_shift) & q->l2_amask;
		unsigned int l1_index = (offset >> q->l1_shift);
		uint64_t l1_val = be64toh(q->l1_table[l1_index]);
		uint64_t l2_offset = l1_val & UINT64_C(0x00fffffffffffe00);
		uint64_t data_offset;

		if (!l2_offset) {
			data_offset = 0;
		} else {
			const uint64_t *l2_table = load_cluster(q, l2_offset,
			    KIND_L2);
			if (!l2_table)
				return -1;

			uint64_t l2_val = be64toh(l2_table[l2_index]);
			if ((l2_val >> 62) & 1) {
				/* Compressed cluster not supported */
				errno = ENOTSUP;
				return -1;
			}
			if ((l2_val & 1))
				data_offset = 0;
			else
				data_offset = l2_val &
				   UINT64_C(0x00fffffffffffe00);
		}

		/* Be careful not to read beyond the end of the cluster */
		size_t cluster_offset = offset % q->cluster_size;
		size_t rlen = len;
		if (rlen + cluster_offset > q->cluster_size)
			rlen = q->cluster_size - cluster_offset;

		if (!data_offset) {
			memset(dest, '\0', rlen);
		} else {
			if (vfile_seek(q->src, data_offset + cluster_offset)
			    == -1)
				return -1;
			int n = vfile_read(q->src, dest, rlen);
			if (n == -1)
				return -1;
			rlen = n;
		}

		if (rlen == 0)
			break; /* EOF */
		ret += rlen;
		len -= rlen;
		dest = (char *)dest + rlen;
		offset += rlen;
	}

	q->pos += ret;
	return ret;
}

static const struct vfileops qcow2_ops = {
	.close = qcow2_close,
	.read = qcow2_read,
	.tell = qcow2_tell,
	.getsize = qcow2_getsize,
	.seek = qcow2_seek,
};

struct vfile *
vfile_qcow2(struct vfile *src)
{
	struct vfile_qcow2 *q = NULL;
	uint64_t size;
	unsigned int i;

	if (!src)
		return NULL;

	/* Check the header */
	char hdr[104];
	if (vfile_seek(src, 0) == -1)
		goto fail;
	int n = vfile_read(src, hdr, sizeof hdr);
	if (n == -1)
		goto fail;
	if (n != sizeof hdr) {
		warnx("too short for qcow2");
		goto fail;
	}

	if (memcmp(hdr, "QFI\xfb", 4) != 0) {
		warnx("bad magic");
		goto fail;
	}

	q = calloc(1, sizeof *q);
	if (!q) {
		warn("calloc");
		goto fail;
	}
	VFILE_INIT(&q->vfile, &qcow2_ops);
	q->src = src;

	uint32_t version = be32tohp(&hdr[4]);
	if (version < 2) {
		warnx("qcow2 too old");
		goto fail;
	}

	uint32_t crypt_method = be32tohp(&hdr[32]);
	if (crypt_method != 0) {
		warnx("qcow2 encrypted"); /* not supported */
		goto fail;
	}

	if (version >= 3) {
		uint64_t incompatible_features = be64tohp(&hdr[72]);
		if ((incompatible_features >> 3) & 1) {
			warnx("qcow2 compressed"); /* not supported */
			goto fail;
		}
	}

	size = be64tohp(&hdr[24]);
	if (size > INT_MAX) {
		size = INT_MAX;
	}
	q->size = size;

	uint32_t cluster_bits = be32tohp(&hdr[20]);
	q->cluster_size = (size_t)1 << cluster_bits;
	if (!q->cluster_size) {
		warnx("clusters too big");
		goto fail;
	}

	/* Allocate a small cluster cache */
	for (i = 0; i < KIND_MAX; i++) {
		void *base = malloc(q->cluster_size);
		if (!base) {
			warn("malloc");
			goto fail;
		}
		q->cluster[i].base = base;
	}

	/* Precompute shifts and masks to extract the L1
	 * and L2 indicies from the offset. */
	q->l2_shift = cluster_bits;
	q->l2_amask = (1U << (cluster_bits - 3)) - 1;
	q->l1_shift = cluster_bits + cluster_bits - 3;

	uint64_t l1_table_offset = be64tohp(&hdr[40]);
	uint32_t l1_size = be32tohp(&hdr[36]);
	q->l1_table_size = l1_size * sizeof (uint64_t);

	if (l1_table_offset > INT_MAX) {
		warnx("image too big");
		goto fail;
	}

	/* Load the L1 table into memory */
	q->l1_table = malloc(q->l1_table_size);
	if (!q->l1_table) {
		warn("malloc L1");
		goto fail;
	}
	if (vfile_seek(q->src, l1_table_offset) == -1)
		goto fail;
	if (vfile_read(q->src, q->l1_table, q->l1_table_size) != q->l1_table_size)
		goto fail;

	return &q->vfile;

fail:;
	vfile_decref(src);
	if (q) {
		for (i = 0; i < KIND_MAX; i++)
			free(q->cluster[i].base);
		free(q->l1_table);
		free(q);
	}
	return NULL;
}
