#include <stdlib.h>
#include "fileblock.h"
#include "fileblock_ram.h"
#include "fileblock_file.h"

enum fb_mode {
	FB_MODE_RAM,
	FB_MODE_FILE,
};

static enum fb_mode fb_mode = FB_MODE_RAM;

int fb_init_file(const char *filename)
{
	int ret;

	ret = fb_file_open(filename);
	if (ret < 0)
		return ret;

	fb_mode = FB_MODE_FILE;
	return 0;
}

unsigned long fb_len(void)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_len();
	case FB_MODE_FILE: return fb_file_len();
	default: return 0;
	}
}

int fb_seek_set(unsigned long offset)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_seek_set(offset);
	case FB_MODE_FILE: return fb_file_seek_set(offset);
	default: return 0;
	}
}

int fb_seek_end(unsigned long offset)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_seek_end(offset);
	case FB_MODE_FILE: return fb_file_seek_end(offset);
	default: return -1;
	}
}

int fb_seek_inc(unsigned long offset)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_seek_inc(offset);
	case FB_MODE_FILE: return fb_file_seek_inc(offset);
	default: return -1;
	}
}

int fb_seek_dec(unsigned long offset)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_seek_dec(offset);
	case FB_MODE_FILE: return fb_file_seek_dec(offset);
	default: return -1;
	}
}

unsigned long fb_tell(void)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_tell();
	case FB_MODE_FILE: return fb_file_tell();
	default: return 0;
	}
}

void fb_throw(unsigned long maxlen, void (* f)(void *, unsigned long))
{
	switch (fb_mode) {
	case FB_MODE_RAM: fb_ram_throw(maxlen, f);
	case FB_MODE_FILE: fb_file_throw(maxlen, f);
	default: break;
	}
}

int fb_write(const void *data, unsigned long len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_write(data, len);
	case FB_MODE_FILE: return fb_file_write(data, len);
	default: return -1;
	}
}

int fb_peek(void *data, unsigned long len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_peek(data, len);
	case FB_MODE_FILE: return fb_file_peek(data, len);
	default: return -1;
	}
}

int fb_read(void *data, unsigned long len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_read(data, len);
	case FB_MODE_FILE: return fb_file_read(data, len);
	default: return -1;
	}
}

void *fb_read_block(unsigned long *len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_read_block(len);
	case FB_MODE_FILE: return fb_file_read_block(len);
	default: return NULL;
	}
}

int fb_trim(unsigned long len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_trim(len);
	case FB_MODE_FILE: return fb_file_trim(len);
	default: return -1;
	}
}

unsigned long fb_meta_len(void)
{
	switch (fb_mode) {
	case FB_MODE_RAM: return fb_ram_meta_len();
	case FB_MODE_FILE: return fb_file_meta_len();
	default: return 0;
	}
}

void fb_meta_add(unsigned long len)
{
	switch (fb_mode) {
	case FB_MODE_RAM: fb_ram_meta_add(len);
	case FB_MODE_FILE: fb_file_meta_add(len);
	default: break;
	}
}

void fb_meta_trim(void)
{
	switch (fb_mode) {
	case FB_MODE_RAM: fb_ram_meta_trim();
	case FB_MODE_FILE: fb_file_meta_trim();
	default: break;
	}
}
