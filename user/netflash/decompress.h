#ifndef NETFLASH_DECOMPRESS_H
#define NETFLASH_DECOMPRESS_H

extern int doinflate;

unsigned long check_decompression(void);
int decompress(void *data, int length);

#endif
