/* $Id: conf-convex.h,v 1.2 1993/08/19 05:26:53 genek Exp $ */

/*
 * conf-convex.h
 *
 *	Tripwire configuration file
 *
 * Gene Kim
 * Purdue University
 * 
 * Ported to COnvexos 9.1 
 *	by Adrian P van Bloois at ACCU <adrian@cc.ruu.nl>
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

#define STDLIBH

/*
 * does your system use readdir(3) that returns (struct dirent *)?
 */

#define DIRENT

/*
 * is #include <string.h> ok?  (as opposed to <strings.h>)
 */

#define STRINGH
 
/* 
 * does your system have gethostname(2) (instead of uname(2))?
 */

#define GETHOSTNAME
