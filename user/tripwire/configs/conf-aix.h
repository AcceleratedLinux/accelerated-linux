/* $Id: conf-aix.h,v 1.2 1993/08/19 05:26:50 genek Exp $ */

/*
 * conf-aix.h
 *
 *	Tripwire configuration file
 *
 * Gene Kim
 * Purdue University
 */

/***
 *** Operating System specifics
 ***	
 ***	If the answer to a question in the comment is "Yes", then
 ***	change the corresponding "#undef" to a "#define"
 ***/

/*
 * is your OS a System V derivitive?  if so, what version?
 *			(e.g., define SYSV 4)
 */

#undef SYSV

/* 
 * does your system have a <malloc.h> like System V? 
 */

#undef MALLOCH 	

/* 
 * does your system have a <stdlib.h> like POSIX says you should? 
 */

#undef STDLIBH

/* 
 * does your system have gethostname(2) (instead of uname(2))?
 */

#define GETHOSTNAME

/*
 * does your system use readdir(3) that returns (struct dirent *)?
 */

#define DIRENT

/*
 * is #include <string.h> ok?  (as opposed to <strings.h>)
 */

#define STRINGH
 
/* 
 * miscellaneous stuff
 *
 *	AIX redefines much of <sys/types> typedefs as unsigned longs.
 */

#define AIX		
