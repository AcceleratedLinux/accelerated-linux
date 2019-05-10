/* TFTP prototypes */

#include <stdio.h>
#include <arpa/tftp.h>

#define OACK 06				/* option acknowledgment */
#define PKTSIZE    SEGSIZE+4
#define BLOCKSIZE_DEFAULT 8192
#define BLOCKSIZE_MIN 8
#define BLOCKSIZE_MAX 65464

int tftpsynchnet(int f);
int tftpwriteit(FILE *file, struct tftphdr **dpp, int ct, int convert);
int tftpwrite_behind(FILE *file, int convert);
void tftprecvfile(int fd, char *name, char *mode, int blksize);

void tftpget(int argc, char *argv[]);
void tftpmain(int argc, char *argv[]);
void tftpmodecmd(int argc, char *argv[]);
void tftpsetascii(int argc, char *argv[]);
void tftpsetbinary(int argc, char *argv[]);
void tftpsetpeer(int argc, char *argv[]);
void tftprw_free(void);
