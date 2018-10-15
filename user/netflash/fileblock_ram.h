/****************************************************************************/

/*
 *	fileblock_ram.h -- ram implementation of fileblock
 */

/****************************************************************************/
#ifndef FILEBLOCK_RAM_H
#define FILEBLOCK_RAM_H 1
/****************************************************************************/

extern unsigned long fb_ram_len(void);
extern int fb_ram_seek_set(unsigned long offset);
extern int fb_ram_seek_end(unsigned long offset);
extern int fb_ram_seek_inc(unsigned long offset);
extern int fb_ram_seek_dec(unsigned long offset);
extern unsigned long fb_ram_tell(void);
extern void fb_ram_throw(unsigned long maxlen, void (* f)(void *, unsigned long));
extern int fb_ram_write(const void *data, unsigned long len);
extern int fb_ram_peek(void *data, unsigned long len);
extern int fb_ram_read(void *data, unsigned long len);
extern void *fb_ram_read_block(unsigned long *len);
extern int fb_ram_trim(unsigned long len);
extern unsigned long fb_ram_meta_len(void);
extern void fb_ram_meta_add(unsigned long len);
extern void fb_ram_meta_trim(void);

/****************************************************************************/
#endif /* FILEBLOCK_RAM_H */
