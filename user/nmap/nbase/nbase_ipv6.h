
/***************************************************************************
 * nbase_ipv6.h -- IPv6 portability classes and structures These were      *
 * written by fyodor@insecure.org .                                        *
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

/* $Id: nbase_ipv6.h 3899 2006-08-29 05:42:35Z fyodor $ */

#ifndef NBASE_IPV6_H
#define NBASE_IPV6_H

#ifdef __amigaos__
#ifndef _NMAP_AMIGAOS_H_
#include "../nmap_amigaos.h"
#endif
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef HAVE_AF_INET6
#define AF_INET6 10
#define PF_INET6 10
#endif /* HAVE_AF_INET6 */
#ifndef HAVE_INET_PTON
/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
int inet_pton(int af, const char *src, void *dst);
#endif /* HAVE_INET_PTON */

#ifndef HAVE_INET_NTOP
/* char *
 * inet_ntop(af, src, dst, size)
 *	convert a network format address to presentation format.
 * return:
 *	pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *	Paul Vixie, 1996.
 */
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#endif /* HAVE_INET_NTOP */

#ifndef HAVE_SOCKADDR_STORAGE
     /* Just needs to be big enough to hold sockaddr_in or
	sockaddr_in6.  I should really align it at 64 bits, but 32 is
	probably fine as hosts that actually want to store a
	sockaddr_in6 in here should already have this defined (see
	RFC2355). */
struct sockaddr_storage {
	u32 padding[32];
}; 
#endif /* SOCKADDR_STORAGE */

/* This function is an easier version of inet_ntop because you don't
   need to pass a dest buffer.  Instead, it returns a static buffer that
   you can use until the function is called again (by the same or another
   thread in the process).  If there is a wierd error (like sslen being
   too short) then NULL will be returned. */
const char *inet_ntop_ez(struct sockaddr_storage *ss, size_t sslen);


#if !HAVE_GETNAMEINFO || !HAVE_GETADDRINFO
#if !defined(EAI_MEMORY)
#define EAI_ADDRFAMILY   1      /* address family for hostname not supported */
#define EAI_AGAIN        2      /* temporary failure in name resolution */
#define EAI_BADFLAGS     3      /* invalid value for ai_flags */
#define EAI_FAIL         4      /* non-recoverable failure in name resolution */
#define EAI_FAMILY       5      /* ai_family not supported */
#define EAI_MEMORY       6      /* memory allocation failure */
#define EAI_NODATA       7      /* no address associated with hostname */
#define EAI_NONAME       8      /* hostname nor servname provided, or not known */
#define EAI_SERVICE      9      /* servname not supported for ai_socktype */
#define EAI_SOCKTYPE    10      /* ai_socktype not supported */
#define EAI_SYSTEM      11      /* system error returned in errno */
#define EAI_BADHINTS    12
#define EAI_PROTOCOL    13
#define EAI_MAX         14
#endif /* EAI_MEMORY */
#endif /* !HAVE_GETNAMEINFO || !HAVE_GETADDRINFO */

#if !HAVE_GETNAMEINFO
/* This replacement version is *NOT* a full implementation by any
   stretch of the imagination */
/* getnameinfo flags */
#if !defined(NI_NAMEREQD)
#define NI_NOFQDN 8
#define NI_NUMERICHOST 16
#define NI_NAMEREQD 32
#define NI_NUMERICSERV 64
#define NI_DGRAM 128
#endif

struct sockaddr;
int getnameinfo(const struct sockaddr *sa, size_t salen,
		char *host, size_t hostlen,
		char *serv, size_t servlen, int flags);
#endif /* !HAVE_GETNAMEINFO */

#if !HAVE_GETADDRINFO
/* This replacement version is *NOT* a full implementation by any
   stretch of the imagination */
struct addrinfo {
  int ai_flags;      /*  AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
  int ai_family;    /* PF_xxx */
  int ai_socktype;  /* SOCK_xxx */
  int ai_protocol;  /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
  size_t ai_addrlen;   /* length of ai_addr */
  char *ai_canonname; /* canonical name for nodename */
  struct sockaddr  *ai_addr; /* binary address */
  struct  addrinfo  *ai_next; /* next structure in linked list */
};

/* getaddrinfo Flags */
#if !defined(AI_PASSIVE) || !defined(AI_CANONNAME) || !defined(AI_NUMERICHOST)
#define AI_PASSIVE 1
#define AI_CANONNAME 2
#define AI_NUMERICHOST 4
#endif

void freeaddrinfo(struct addrinfo *res);
int getaddrinfo(const char *node, const char *service, 
		const struct addrinfo *hints, struct addrinfo **res);

#endif /* !HAVE_GETADDRINFO */

#ifndef HAVE_GAI_STRERROR
const char *gai_strerror(int errcode);
#endif


#endif /* NBASE_IPV6_H */
