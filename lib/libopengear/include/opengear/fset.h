#ifndef _OPENGEAR_FSET_H_
#define _OPENGEAR_FSET_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEFAULTOFFS
#define DEFAULTOFFS 0x000
#endif

#define FLASHLEN	(1024 * 128)

#define	DEFAULTFLASH	"/dev/flash/bootarg"

typedef struct V_SET {
	char *name;
	char *value;
	struct V_SET *next;
} vset;

/**
 * @return 1 for error and 2 to display usage()
*/
size_t readfsetflash(const char *flash, int offset);

/**
 * The calling program is responsible for calling freeVars()
 * in order to free the global variable called "head".
 * "head" is a ponter to the start of the flash memory
 * where the fset strings are stored.
*/
void freeVars(void);

vset * findValue(const char *value);
vset * firstValue(void);
void saveFlash(const char * flash, off_t offset);
int updateflash(const char * flash, off_t offset, const char * write);
void deleteflash(const char * flash, off_t offset, const char * name);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_FSET_H_ */

