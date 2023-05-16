#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "vfile.h"

/* Simple adapter for a Unix file descriptor as vfile */
struct vfile_fd {
	struct vfile vfile;
	int fd;
};

static struct vfile_fd *
to_fd(struct vfile *f)
{
	/* assert(f->ops = &fd_ops); */
        return (struct vfile_fd *)f;
}

static void
fd_close(struct vfile *f)
{
	struct vfile_fd *fd = to_fd(f);

	close(fd->fd);
	free(fd);
}

static int
fd_read(struct vfile *f, void *dst, int sz)
{
	struct vfile_fd *fd = to_fd(f);
	int ret = 0;
	char *d = dst;

	/* Unix reads can be short, but vfile reads never are. */
	while (sz) {
		ssize_t n = read(fd->fd, d, sz);
		if (n == -1)
			return -1;
		if (n == 0)
			break;
		sz -= n;
		d += n;
		ret += n;
	}
	return ret;
}

static int
fd_tell(struct vfile *f)
{
	struct vfile_fd *fd = to_fd(f);

	return lseek(fd->fd, 0, SEEK_CUR);
}

static int
fd_getsize(struct vfile *f)
{
	struct vfile_fd *fd = to_fd(f);
	struct stat sb;

	if (fstat(fd->fd, &sb) == -1)
		return -1;
	return sb.st_size;
}

static int
fd_seek(struct vfile *f, int offset)
{
	struct vfile_fd *fd = to_fd(f);

	return lseek(fd->fd, offset, SEEK_SET);
}

static const struct vfileops fd_ops = {
	.close = fd_close,
	.read = fd_read,
	.tell = fd_tell,
	.getsize = fd_getsize,
	.seek = fd_seek,
};

struct vfile *
vfile_fd(int fd_)
{
	struct vfile_fd *fd = calloc(1, sizeof *fd);
	if (!fd) {
		close(fd_);
		return NULL;
	}
	VFILE_INIT(&fd->vfile, &fd_ops);
	fd->fd = fd_;
	return &fd->vfile;
}
