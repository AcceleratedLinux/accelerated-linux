/****************************************************************************/

/*
 *	fileblock.h -- file implementation of fileblock
 */

/****************************************************************************/
#ifndef FILEBLOCK_FILE_H
#define FILEBLOCK_FILE_H 1
/****************************************************************************/

extern int fb_file_open(const char *filename);
extern unsigned long fb_file_len(void);
extern int fb_file_seek_set(unsigned long offset);
extern int fb_file_seek_end(unsigned long offset);
extern int fb_file_seek_inc(unsigned long offset);
extern int fb_file_seek_dec(unsigned long offset);
extern unsigned long fb_file_tell(void);
extern void fb_file_throw(unsigned long maxlen, void (* f)(void *, unsigned long));
extern int fb_file_write(const void *data, unsigned long len);
extern int fb_file_peek(void *data, unsigned long len);
extern int fb_file_read(void *data, unsigned long len);
extern void *fb_file_read_block(unsigned long *len);
extern int fb_file_trim(unsigned long len);
extern unsigned long fb_file_meta_len(void);
extern void fb_file_meta_add(unsigned long len);
extern void fb_file_meta_trim(void);

/****************************************************************************/
#endif /* FILEBLOCK_FILE_H */
