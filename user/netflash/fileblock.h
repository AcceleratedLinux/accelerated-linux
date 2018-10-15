/****************************************************************************/

/*
 *	fileblock.h -- common file buffer list
 */

/****************************************************************************/
#ifndef FILEBLOCK_H
#define FILEBLOCK_H 1
/****************************************************************************/

extern int fb_init_file(const char *filename);
extern unsigned long fb_len(void);
extern int fb_seek_set(unsigned long offset);
extern int fb_seek_end(unsigned long offset);
extern int fb_seek_inc(unsigned long offset);
extern int fb_seek_dec(unsigned long offset);
extern unsigned long fb_tell(void);
extern void fb_throw(unsigned long maxlen, void (* f)(void *, unsigned long));
extern int fb_write(const void *data, unsigned long len);
extern int fb_peek(void *data, unsigned long len);
extern int fb_read(void *data, unsigned long len);
extern void *fb_read_block(unsigned long *len);
extern int fb_trim(unsigned long len);
extern unsigned long fb_meta_len(void);
extern void fb_meta_add(unsigned long len);
extern void fb_meta_trim(void);

/****************************************************************************/
#endif /* FILEBLOCK_H */
