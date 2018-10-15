
/***************************************************************************
 * nbase.h -- The main include file exposing the external API for          *
 * libnbase, a library of base (often compatability) routines.  Programs   *
 * using libnbase can guarantee the availability of functions like         *
 * (v)snprintf and inet_pton.  This library also provides consistency and  *
 * extended features for some functions.  It was originally written for    *
 * use in the Nmap Security Scanner ( http://www.insecure.org/nmap/ ).     *
 *                                                                         *
 ***********************IMPORTANT NMAP LICENSE TERMS************************
 *                                                                         *
 * The Nmap Security Scanner is (C) 1996-2006 Insecure.Com LLC. Nmap is    *
 * also a registered trademark of Insecure.Com LLC.  This program is free  *
 * software; you may redistribute and/or modify it under the terms of the  *
 * GNU General Public License as published by the Free Software            *
 * Foundation; Version 2 with the clarifications and exceptions described  *
 * below.  This guarantees your right to use, modify, and redistribute     *
 * this software under certain conditions.  If you wish to embed Nmap      *
 * technology into proprietary software, we sell alternative licenses      *
 * (contact sales@insecure.com).  Dozens of software vendors already       *
 * license Nmap technology such as host discovery, port scanning, OS       *
 * detection, and version detection.                                       *
 *                                                                         *
 * Note that the GPL places important restrictions on "derived works", yet *
 * it does not provide a detailed definition of that term.  To avoid       *
 * misunderstandings, we consider an application to constitute a           *
 * "derivative work" for the purpose of this license if it does any of the *
 * following:                                                              *
 * o Integrates source code from Nmap                                      *
 * o Reads or includes Nmap copyrighted data files, such as                *
 *   nmap-os-fingerprints or nmap-service-probes.                          *
 * o Executes Nmap and parses the results (as opposed to typical shell or  *
 *   execution-menu apps, which simply display raw Nmap output and so are  *
 *   not derivative works.)                                                * 
 * o Integrates/includes/aggregates Nmap into a proprietary executable     *
 *   installer, such as those produced by InstallShield.                   *
 * o Links to a library or executes a program that does any of the above   *
 *                                                                         *
 * The term "Nmap" should be taken to also include any portions or derived *
 * works of Nmap.  This list is not exclusive, but is just meant to        *
 * clarify our interpretation of derived works with some common examples.  *
 * These restrictions only apply when you actually redistribute Nmap.  For *
 * example, nothing stops you from writing and selling a proprietary       *
 * front-end to Nmap.  Just distribute it by itself, and point people to   *
 * http://insecure.org/nmap/ to download Nmap.                             *
 *                                                                         *
 * We don't consider these to be added restrictions on top of the GPL, but *
 * just a clarification of how we interpret "derived works" as it applies  *
 * to our GPL-licensed Nmap product.  This is similar to the way Linus     *
 * Torvalds has announced his interpretation of how "derived works"        *
 * applies to Linux kernel modules.  Our interpretation refers only to     *
 * Nmap - we don't speak for any other GPL products.                       *
 *                                                                         *
 * If you have any questions about the GPL licensing restrictions on using *
 * Nmap in non-GPL works, we would be happy to help.  As mentioned above,  *
 * we also offer alternative license to integrate Nmap into proprietary    *
 * applications and appliances.  These contracts have been sold to dozens  *
 * of software vendors, and generally include a perpetual license as well  *
 * as providing for priority support and updates as well as helping to     *
 * fund the continued development of Nmap technology.  Please email        *
 * sales@insecure.com for further information.                             *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included Copying.OpenSSL file, and distribute linked      *
 * combinations including the two. You must obey the GNU GPL in all        *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         *
 * If you received these files with a written license agreement or         *
 * contract stating terms other than the terms above, then that            *
 * alternative license agreement takes precedence over these comments.     *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to fyodor@insecure.org for possible incorporation into the main         *
 * distribution.  By sending these changes to Fyodor or one the            *
 * Insecure.Org development mailing lists, it is assumed that you are      *
 * offering Fyodor and Insecure.Com LLC the unlimited, non-exclusive right *
 * to reuse, modify, and relicense the code.  Nmap will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).  We also occasionally relicense the    *
 * code to third parties as discussed above.  If you wish to specify       *
 * special license conditions of your contributions, just say so when you  *
 * send them.                                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details at                              *
 * http://www.gnu.org/copyleft/gpl.html , or in the COPYING file included  *
 * with Nmap.                                                              *
 *                                                                         *
 ***************************************************************************/

/* $Id: nbase.h 3899 2006-08-29 05:42:35Z fyodor $ */

#ifndef NBASE_H
#define NBASE_H

/* NOTE -- libnbase offers the following features that you should probably
 * be aware of:
 *
 * * 'inline' is defined to what is neccessary for the C compiler being
 *   used (which may be nothing)
 *
 * * snprintf, inet_pton, memcpy, and bzero are 
 *   provided if you don't have them (prototypes for these are 
 *   included either way).
 *
 * * WORDS_BIGENDIAN is defined if platform is big endian
 *
 * * Definitions included which give the operating system type.  They
 *   will generally be one of the following: LINUX, FREEBSD, NETBSD,
 *   OPENBSD, SOLARIS, SUNOS, BSDI, IRIX, NETBSD
 *
 * * Insures that GNU getopt_* functions exist (such as getopt_long_only
 *
 * * Various string functions such as Strncpy() and strcasestr() see protos 
 *   for more info.
 *
 * * IPv6 structures like 'sockaddr_storage' are provided if they do
 *   not already exist.
 *
 * * Various Windows -> UNIX compatability definitions are added (such as defining EMSGSIZE to WSAEMSGSIZE)
 */

#if HAVE_CONFIG_H
#include "nbase_config.h"
#else
#ifdef WIN32
#include "nbase_winconfig.h"
#endif /* WIN32 */
#endif /* HAVE_CONFIG_H */

#ifdef WIN32
#include "nbase_winunix.h"
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>  
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 2048
#endif

#ifndef HAVE___ATTRIBUTE__
#define __attribute__(args)
#endif

#include <stdarg.h>

/* Integer widths */
#if (SIZEOF_CHAR == 1)
typedef unsigned char u8;
#else
typedef u_int8_t u8;
#endif

#if (SIZEOF_SHORT == 2)
typedef unsigned short u16;
typedef short s16;
#elif (SIZEOF_CHAR == 2)
typedef unsigned char u16;
typedef char s16;
#else
typedef u_int16_t u16;
typedef int16_t s16;
#endif

#if (SIZEOF_SHORT == 4)
typedef unsigned short u32;
typedef short s32;
#elif (SIZEOF_INT == 4)
typedef unsigned int u32;
typedef int s32;
#elif (SIZEOF_LONG == 4)
typedef unsigned long u32;
typedef long s32;
#else
typedef u_int32_t u32;
typedef int32_t s32;
#endif

/* Mathematicial MIN/MAX/ABS (absolute value) macros */
#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef ABS
#define ABS(x) (((x) >= 0)?(x):(-x)) 
#endif


/* sprintf family */
#if !defined(HAVE_SNPRINTF) && defined(__cplusplus)
extern "C" int snprintf (char *str, size_t sz, const char *format, ...)
     __attribute__ ((format (printf, 3, 4)));
#endif

#if !defined(HAVE_VSNPRINTF) && defined(__cplusplus)
extern "C" int vsnprintf (char *str, size_t sz, const char *format,
     va_list ap)
     __attribute__((format (printf, 3, 0)));
#endif

#if !defined(HAVE_ASPRINTF) && defined(__cplusplus)
extern "C" int asprintf (char **ret, const char *format, ...)
     __attribute__ ((format (printf, 2, 3)));
#endif

#if !defined(HAVE_VASPRINTF) && defined(__cplusplus)
extern "C" int vasprintf (char **ret, const char *format, va_list ap)
     __attribute__((format (printf, 2, 0)));
#endif

#if !defined(HAVE_ASNPRINTF) && defined(__cplusplus)
extern "C" int asnprintf (char **ret, size_t max_sz, const char *format, ...)
     __attribute__ ((format (printf, 3, 4)));
#endif

#if !defined(HAVE_VASNPRINTF) && defined(__cplusplus)
extern "C" int vasnprintf (char **ret, size_t max_sz, const char *format,
     va_list ap)
     __attribute__((format (printf, 3, 0)));
#endif

#if defined(NEED_SNPRINTF_PROTO) && defined(__cplusplus)
extern "C" int snprintf (char *, size_t, const char *, ...);
#endif

#if defined(NEED_VSNPRINTF_PROTO) && defined(__cplusplus)
extern "C" int vsnprintf (char *, size_t, const char *, va_list);
#endif

/* GNU getopt replacements ... Anyone have a BSD licensed version of these? */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
/* The next half-dozen lines are from gcc-2.95 ... -Fyodor */
/* Include getopt.h for the sake of getopt_long.
   We don't need the declaration of getopt, and it could conflict
   with something from a system header file, so effectively nullify that.  */
#define getopt getopt_loser
#include "getopt.h"
#undef getopt
#endif /* HAVE_GETOPT_H */

/* More Windows-specific stuff */
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN /* Whatever this means! From winclude.h*/

/* Apparently Windows doesn't have S_ISDIR */
#ifndef S_ISDIR
#define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
#endif

#define stat _stat /* wtf was ms thinking? */
#define execve _execve
#define getpid _getpid
#define dup _dup
#define dup2 _dup2
#define strdup _strdup
#define write _write
#define open _open
#define stricmp _stricmp
#define putenv _putenv

#if !defined(__GNUC__)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define execv _execv

#endif /* WIN32 */

/* Apparently Windows doesn't like /dev/null */
#ifdef WIN32
#define DEVNULL "NUL"
#else
#define DEVNULL "/dev/null"
#endif



#ifdef __cplusplus
extern "C" {
#endif

  /* Returns the UNIX/Windows errno-equivalent.  Note that the Windows
     call is socket/networking specific.  Also, WINDOWS TENDS TO RESET
     THE ERROR, so it will return success the next time.  So SAVE THE
     RESULTS and re-use them, don't keep calling socket_errno().  The
     windows error number returned is like WSAMSGSIZE, but nbase.h
     includes #defines to correlate many of the common UNIX errors
     with their closest Windows equivalents.  So you can use EMSGSIZE
     or EINTR. */
int socket_errno();

/* The usleep() function is important as well */
#ifndef HAVE_USLEEP
#if defined( HAVE_NANOSLEEP) || defined(WIN32)
void usleep(unsigned long usec);
#endif
#endif

/***************** String functions -- See nbase_str.c ******************/
/* I modified this conditional because !@# Redhat does not easily provide
   the prototype even though the function exists */
#if !defined(HAVE_STRCASESTR) || (defined(LINUX) && !defined(__USE_GNU) && !defined(_GNU_SOURCE))
/* strcasestr is like strstr() except case insensitive */
char *strcasestr(const char *haystack, const char *pneedle);
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, struct timeval *tz);
#endif

#ifndef HAVE_SLEEP
unsigned int sleep(unsigned int seconds);
#endif

/* Strncpy is like strcpy() except it ALWAYS zero-terminates, even if
   it must truncate */
int Strncpy(char *dest, const char *src, size_t n);


/* Trivial function that returns nonzero if all characters in str of
   length strlength are printable (as defined by isprint()) */
int stringisprintable(const char *str, int strlength);

/* Convert non-printable characters to replchar in the string */
void replacenonprintable(char *str, int strlength, char replchar);

/* A few simple wrappers for the most common memory allocation routines which will exit() if the
	allocation fails, so you don't always have to check -- see nbase_memalloc.c */
void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);
/* Zero-initializing version of safe_malloc */
void *safe_zalloc(size_t size);

  /* Some routines for obtaining simple (not secure on systems that
     lack /dev/random and friends' "random" numbers */
int get_random_bytes(void *buf, int numbytes);
int get_random_int();
unsigned short get_random_ushort();
unsigned int get_random_uint();
u32 get_random_u32();
u16 get_random_u16();
u8 get_random_u8();

/* CRC16 Cyclic Redundancy Check  */
unsigned long crc16(unsigned char *buf, int len);

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#include "nbase_ipv6.h"

#ifdef __cplusplus
}
#endif

#endif /* NBASE_H */
