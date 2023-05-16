#ifndef NETFLASH_PROGRAM_H
#define NETFLASH_PROGRAM_H

extern int sgsize;
extern long long devsize;

extern int offset;		/* Offset to start writing at */
extern int exitstatus;
extern int dolock, dounlock;	/* do we lock/unlock segments as we go */
extern int checkimage;		/* Compare with current flash contents */
extern int checkblank;		/* Do not erase if already blank */
extern unsigned char *check_buf;
extern int preserveconfig;	/* Preserve special bits of flash such as config fs */
extern int preserve;		/* Preserve and portions of flash not written to */
extern int stop_early;		/* stop at end of input data, do not write full dev. */
extern int dojffs2;		/* Write the jffs2 magic to unused segments */

void program_flash(int rd, unsigned long image_length, long long devsize, unsigned char *sgdata, int sgsize);
void check_flash(int rd, uint64_t devsize, unsigned char *sgdata, int sgsize);

int program_flash_open(const char *rdev);

void program_file_init(void);
char *program_file_autoname(const char *filename);
int program_file_open(const char *rdev, int dobackup);

#endif
