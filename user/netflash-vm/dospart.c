#include <assert.h>
#include <stdlib.h>

#include "vfile.h"

/* Decode the DOS partition table in the src image, and return a range vfile
 * that accesses the selected partition. */
struct vfile *
vfile_dospart(struct vfile *src, unsigned int partno)
{
	unsigned char mbr[512];
	unsigned char *part;
	unsigned int start, nsect;

	assert(partno >= 1 && partno <= 4);
	if (!src)
		return NULL;

	if (vfile_seek(src, 0) != 0)
		goto fail;
	if (vfile_read(src, mbr, sizeof mbr) != sizeof mbr)
		goto fail;
	if (mbr[0x1fe] != 0x55 || mbr[0x1ff] != 0xaa) /* MBR signature */
		goto fail;
	part = &mbr[0x1be + (partno - 1) * 16];
	if (!part[4])
		goto fail;	/* empty partition slot */
	start = (part[ 8] <<  0) |
		(part[ 9] <<  8) |
		(part[10] << 16) |
		(part[11] << 24);
	nsect = (part[12] <<  0) |
		(part[13] <<  8) |
		(part[14] << 16) |
		(part[15] << 24);

	return vfile_range(src, start * 512, nsect * 512);
fail:
	vfile_decref(src);
	return NULL;
}
