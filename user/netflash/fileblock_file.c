#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fileblock_file.h"

static int fd = -1;
static unsigned long file_length;
static unsigned long meta_length;
static unsigned long current_pos;

int fb_file_open(const char *filename)
{
	struct stat st;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return -1;

	if (fstat(fd, &st) < 0) {
		close(fd);
		return -1;
	}

	file_length = st.st_size;
	meta_length = 0;
	current_pos = 0;
	return 0;
}

unsigned long fb_file_len()
{
	return file_length;
}

int fb_file_seek_set(unsigned long pos)
{
	if (pos > file_length)
		return -1;

	if (lseek(fd, pos, SEEK_SET) != pos)
		return -1;

	current_pos = pos;
	return 0;
}

int fb_file_seek_end(unsigned long offset)
{
	if (offset > file_length)
		return -1;
	return fb_file_seek_set(file_length - offset);
}

int fb_file_seek_inc(unsigned long offset)
{
	return fb_file_seek_set(current_pos + offset);
}

int fb_file_seek_dec(unsigned long offset)
{
	if (offset > current_pos)
		return -1;
	return fb_file_seek_set(current_pos - offset);
}

unsigned long fb_file_tell(void)
{
	return current_pos;
}

void fb_file_throw(unsigned long maxlen, void (* f)(void *, unsigned long))
{
}

int fb_file_write(const void *data, unsigned long len)
{
	/* Not supported; we don't want to modify the input file. */
	return -1;
#if 0
	/* FIXME: handle short writes */
	if (write(fd, data, len) != len)
		return -1;

	current_pos += len;
	if (current_pos > file_length) {
		file_length = current_pos;
	}

	return 0;
#endif
}

int fb_file_peek(void *data, unsigned long len)
{
	unsigned long pos;
	int ret;

	pos = fb_file_tell();
	ret = fb_file_read(data, len);
	fb_file_seek_set(pos);
	return ret;
}

int fb_file_read(void *data, unsigned long len)
{
	ssize_t n;
	int total;

	if (current_pos + len > file_length)
		len = file_length - current_pos;

	total = 0;
	while (len) {
		do {
			n = read(fd, data, len);
		} while (n < 0 && errno == EINTR);
		if (n < 0) {
			if (total)
				return total;
			return n;
		}
		if (n == 0)
			break;
		current_pos += n;
		data += n;
		total += n;
		len -= n;
	}

	return total;
}

void *fb_file_read_block(unsigned long *len)
{
	static char buf[4096];

	*len = fb_file_read(buf, sizeof(buf));
	if (*len <= 0)
		return NULL;

	return buf;
}

int fb_file_trim(unsigned long len)
{
	if (len > file_length)
		return -1;

#if 0
	if (ftruncate(fd, file_length - len) != 0)
		return -1;
#endif

	file_length -= len;
	if (current_pos > file_length)
		return fb_file_seek_set(file_length);

	return 0;
}

unsigned long fb_file_meta_len(void)
{
	return meta_length;
}

void fb_file_meta_add(unsigned long len)
{
	meta_length += len;
}

void fb_file_meta_trim(void)
{
	if (meta_length) {
		fb_file_trim(meta_length);
		meta_length = 0;
	}
}
