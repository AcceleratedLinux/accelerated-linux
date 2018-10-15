#ifndef __FLASH_API
#define __FLASH_API

int flash_read(char *buf, off_t from, size_t len);
int flash_write(char *buf, off_t to, size_t len);

#endif
