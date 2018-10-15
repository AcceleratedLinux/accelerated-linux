/***************************************************************************
 * getaddrinfo.c -- A **PARTIAL** implementation of the getaddrinfo(3)     *
 * hostname resolution call.  In particular, IPv6 is not supported and     *
 * neither are some of the flags.  Service "names" are always returned as  *
 * port numbers.                                                           *
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

/* $Id: getaddrinfo.c 3899 2006-08-29 05:42:35Z fyodor $ */

#include "nbase.h"

#include <stdio.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <assert.h>


#if !defined(HAVE_GAI_STRERROR) || defined(__MINGW32__)
#ifdef __MINGW32__
#undef gai_strerror
#endif

const char *gai_strerror(int errcode) {
  static char customerr[64];
  switch (errcode) {
  case EAI_FAMILY:
    return "ai_family not supported";
  case EAI_NODATA:
    return "no address associated with hostname";
  case EAI_NONAME:
    return "hostname nor servname provided, or not known";
  default:
    snprintf(customerr, sizeof(customerr), "unknown error (%d)", errcode);
    return "unknown error.";
  }
  return NULL; /* unreached */
}
#endif

#ifdef __MINGW32__
char* WSAAPI gai_strerrorA (int errcode)
{
  return gai_strerror(errcode);
}
#endif

#ifndef HAVE_GETADDRINFO
void freeaddrinfo(struct addrinfo *res) {
  struct addrinfo *next;
  
  do {
    next = res->ai_next;
    free(res);
  } while ((res = next) != NULL);
}

/* Allocates and initializes a new AI structure with the port and IPv4
   address specified in network byte order */
static struct addrinfo *new_ai(unsigned short portno, u32 addr)
{
	struct addrinfo *ai;

	ai = (struct addrinfo *) malloc(sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
	assert(ai);
	
	memset(ai, 0, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
	
	ai->ai_family = AF_INET;
	ai->ai_addrlen = sizeof(struct sockaddr_in);
	ai->ai_addr = (struct sockaddr *)(ai + 1);
	ai->ai_addr->sa_family = AF_INET;
#if HAVE_SOCKADDR_SA_LEN
	ai->ai_addr->sa_len = ai->ai_addrlen;
#endif
	((struct sockaddr_in *)(ai)->ai_addr)->sin_port = portno;
	((struct sockaddr_in *)(ai)->ai_addr)->sin_addr.s_addr = addr;
	
	return(ai);
}


int getaddrinfo(const char *node, const char *service, 
		const struct addrinfo *hints, struct addrinfo **res) {

  struct addrinfo *cur, *prev = NULL;
  struct hostent *he;
  struct in_addr ip;
  unsigned short portno;
  int i;
  
  if (service)
    portno = htons(atoi(service));
  else
    portno = 0;
  
  if (hints && hints->ai_flags & AI_PASSIVE) {
    *res = new_ai(portno, htonl(0x00000000));
    return 0;
  }
  
  if (!node) {
    *res = new_ai(portno, htonl(0x7f000001));
    return 0;
  }
  
  if (inet_pton(AF_INET, node, &ip)) {
    *res = new_ai(portno, ip.s_addr);
    return 0;
  }
  
  he = gethostbyname(node);
  if (he && he->h_addr_list[0]) {
    for (i = 0; he->h_addr_list[i]; i++) {
      cur = new_ai(portno, ((struct in_addr *)he->h_addr_list[i])->s_addr);

      if (prev)
	prev->ai_next = cur;
      else
	*res = cur;
      
      prev = cur;
    }
    return 0;
  }
  
  return EAI_NODATA;
}
#endif /* HAVE_GETADDRINFO */
