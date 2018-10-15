/*	$OpenBSD: lp.h,v 1.14 2003/06/02 23:36:53 millert Exp $	*/
/*	$NetBSD: lp.h,v 1.14 2000/04/16 14:43:58 mrg Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * 	@(#)lp.h	8.2 (Berkeley) 4/28/95
 */

#include <dirent.h>
#include <sys/param.h>
#include <signal.h>
#include <grp.h>

/*
 * Global definitions for the line printer system.
 */

extern char	*AF;		/* accounting file */
extern long	 BR;		/* baud rate if lp is a tty */
extern char	*CF;		/* name of cifplot filter (per job) */
extern char	*DF;		/* name of tex filter (per job) */
extern long	 DU;		/* daemon user-id */
extern long	 FC;		/* flags to clear if lp is a tty */
extern char	*FF;		/* form feed string */
extern long	 FS;		/* flags to set if lp is a tty */
extern char	*GF;		/* name of graph(1G) filter (per job) */
extern long	 HL;		/* print header last */
extern char	*IF;		/* name of input filter (created per job) */
extern char	*LF;		/* log file for error messages */
extern char	*LO;		/* lock file name */
extern char	*LP;		/* line printer device name */
extern long	 MC;		/* maximum number of copies allowed */
extern char	*MS;		/* stty flags to set if lp is a tty */
extern long	 MX;		/* maximum number of blocks to copy */
extern char	*NF;		/* name of ditroff(1) filter (per job) */
extern char	*OF;		/* name of output filter (created once) */
extern long	 PL;		/* page length */
extern long	 PW;		/* page width */
extern long	 PX;		/* page width in pixels */
extern long	 PY;		/* page length in pixels */
extern char	*RF;		/* name of fortran text filter (per job) */
extern char	*RG;		/* restricted group */
extern char	*RM;		/* remote machine name */
extern char	*RP;		/* remote printer name */
extern long	 RS;		/* restricted to those with local accounts */
extern long	 RW;		/* open LP for reading and writing */
extern long	 SB;		/* short banner instead of normal header */
extern long	 SC;		/* suppress multiple copies */
extern char	*SD;		/* spool directory */
extern long	 SF;		/* suppress FF on each print job */
extern long	 SH;		/* suppress header page */
extern char	*ST;		/* status file name */
extern char	*TF;		/* name of troff(1) filter (per job) */
extern char	*TR;		/* trailer string to be output when Q empties */
extern char	*VF;		/* name of raster filter (per job) */
extern long	 XC;		/* flags to clear for local mode */
extern long	 XS;		/* flags to set for local mode */

extern char	line[BUFSIZ];
extern char	*bp;		/* pointer into printcap buffer */
extern char	*printer;	/* printer name */
				/* host machine name */
extern char	host[MAXHOSTNAMELEN];
extern char	*from;		/* client's machine name */
extern int	remote;		/* true if sending files to a remote host */
extern char	*printcapdb[];	/* printcap database array */
extern u_int	wait_time;	/* time to wait for remote responses */

extern uid_t	real_uid, effective_uid;
extern gid_t	real_gid, effective_gid;

extern volatile sig_atomic_t	gotintr;

/*
 * Structure used for building a sorted list of control files.
 */
struct queue {
	time_t	q_time;			/* modification time */
	char	q_name[MAXNAMLEN+1];	/* control file name */
};

/*
 * Macros to raise/lower permissions.
 */
#define PRIV_START do {				\
	int save_errno = errno;			\
	(void)seteuid(effective_uid);		\
	(void)setegid(effective_gid);		\
	errno = save_errno;			\
} while (0)

#define PRIV_END do {				\
	int save_errno = errno;			\
	(void)setegid(real_gid);		\
	(void)seteuid(real_uid);		\
	errno = save_errno;			\
} while (0)

#include <sys/cdefs.h>

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX	64
#endif

__BEGIN_DECLS
struct dirent;

void     blankfill(int);
char	*checkremote(void);
int      chk(char *);
void     displayq(int);
void     dump(char *, char *, int);
void fatal(const char *, ...)
	__attribute__((__format__(__printf__, 1, 2)));
int	 getcfline(FILE *);
int	 getport(char *, int);
int	 getq(struct queue ***);
void     header(void);
int      inlist(char *, char *);
int      iscf(const struct dirent *);
int      isowner(char *, char *);
void     ldump(char *, char *, int);
void     prank(int);
void     process(char *);
void     rmjob(void);
void     rmremote(void);
void     show(char *, char *, int);
int      startdaemon(char *);
void     nodaemon(void);
void     delay(int);
int	 safe_open(const char *, int, mode_t);

char    *cgetcap (char *, const char *, int);
int      cgetent (char **, char **, const char *);
int      cgetmatch (char *, const char *);
int      cgetnext (char **, char **);
int      cgetstr (char *, const char *, char **);
int      cgetunum (char *, const char *, long *);
int	cgetnum(char *, const char *, long *);

size_t	strlcpy(char *dst, const char *src, size_t siz);
size_t	strlcat(char *dst, const char *src, size_t siz);

int	cgetclose();
int	lprGetUID(void);

__END_DECLS
