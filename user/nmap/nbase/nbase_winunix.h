/***************************************************************************
 * nbase_winunix.h -- Misc. compatability routines that generally try to   *
 * reproduce UNIX-centric concepts on Windows.                             *
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

/* $Id: nbase_winunix.h 3899 2006-08-29 05:42:35Z fyodor $ */

#ifndef NBASE_WINUNIX_H
#define NBASE_WINUNIX_H

#define _INC_ERRNO  /* supress errno.h */
#define _ERRNO_H_ /* Also for errno.h suppresion */

/* Supress winsock.h */
#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h> /* IPv6 stuff */
#include <time.h>
#include <iptypes.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <process.h>
#include <limits.h>
#include <WINCRYPT.H>
#include <math.h>

#ifndef HAVE_U_INT8_T
#define HAVE_U_INT8_T
typedef unsigned char u_int8_t;
#endif

#ifndef HAVE_U_INT16_T
#define HAVE_U_INT16_T
typedef unsigned short u_int16_t;
#endif

#ifndef HAVE_U_INT32_T
#define HAVE_U_INT32_T
typedef unsigned long u_int32_t;
#endif

#ifndef _SSIZE_T_
typedef unsigned int ssize_t;
#endif

#define SIOCGIFCONF     0x8912          /* get iface list */

#ifndef GLOBALS
#define GLOBALS 1

#endif

/* Disables VC++ warning:
  "integral size mismatch in argument; conversion supplied".  Perhaps
  I should try to fix this with casts at some point */
/* #pragma warning(disable: 4761) */

#define munmap(ptr, len) win32_munmap(ptr, len)
int nmapwin_isroot();

/* Windows error message names */
#define ECONNABORTED    WSAECONNABORTED
#define ECONNRESET      WSAECONNRESET
#define ECONNREFUSED    WSAECONNREFUSED
#undef  EAGAIN
#define EAGAIN		WSAEWOULDBLOCK
#define EHOSTUNREACH	WSAEHOSTUNREACH
#define ENETDOWN	WSAENETDOWN
#define ENETUNREACH	WSAENETUNREACH
#define ENETRESET	WSAENETRESET
#define ETIMEDOUT	WSAETIMEDOUT
#define EHOSTDOWN	WSAEHOSTDOWN
#define EINPROGRESS	WSAEINPROGRESS
#undef  EINVAL
#define EINVAL          WSAEINVAL      /* Invalid argument */
#undef  EPERM
#define EPERM           WSAEACCES      /* Operation not permitted */
#undef  EACCES
#define EACCES          WSAEACCES     /* Operation not permitted */
#undef  EINTR
#define EINTR           WSAEINTR      /* Interrupted system call */
#define ENOBUFS         WSAENOBUFS     /* No buffer space available */
#undef  ENOENT
#define ENOENT          WSAENOENT      /* No such file or directory */
#define EMSGSIZE        WSAEMSGSIZE    /* Message too long */
#undef  ENOMEM
#define ENOMEM          WSAENOBUFS
#undef  EIO
#define EIO             WSASYSCALLFAILURE

#define close(x) my_close(x)
/* #define read(x,y,z) recv(x,(char*)(y),z,0) */

#ifdef __cplusplus
  extern "C" int my_close(int sd);
#else
  int my_close(int sd);
#endif

typedef unsigned short u_short_t;

#endif /* NBASE_WINUNIX_H */
