#ifndef NETFLASH_UTIL_H
#define NETFLASH_UTIL_H

#include <stdio.h>

#define notice(a...) do { fprintf(stdout, "netflash: " a); fprintf(stdout, "\n"); fflush(stdout); } while (0)
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
#define error(a...) do { fprintf(stdout, "netflash: " a); fprintf(stdout, "\n"); fflush(stdout); } while (0)
#else
#define error(a...) do { fprintf(stderr, "netflash: " a); fprintf(stderr, "\n"); fflush(stderr); } while (0)
#endif

extern int time_rate_100;
extern void time_start(void);
extern unsigned long time_elapsed(void);

#endif
