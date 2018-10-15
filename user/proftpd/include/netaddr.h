/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Network address API
 * $Id: netaddr.h,v 1.18 2004/11/02 18:18:58 castaglia Exp $
 */

#ifndef PR_NETADDR_H
#define PR_NETADDR_H

#include "conf.h"

#ifndef HAVE_STRUCT_ADDRINFO
struct addrinfo {

  /* AI_PASSIVE, AI_CANONNAME */
  int ai_flags;

  /* AF/PF_xxx */
  int ai_family;

  /* SOCK_xxx */
  int ai_socktype;

  /* IPPROTO_xxx for IPv4/v6 */
  int ai_protocol;

  /* Length of ai_addr */
  int ai_addrlen;

  /* Canonical name for host */
  char *ai_canonname;

  /* Binary address */
  struct sockaddr *ai_addr;

  /* Next structure in the linked list */
  struct addrinfo *ai_next;
};
#endif /* HAVE_STRUCT_ADDRINFO */

#if defined(HAVE_GETADDRINFO) && !defined(PR_USE_GETADDRINFO)
/* Use the system getaddrinfo(2) and freeaddrinfo(2) by redefining the
 * 'pr_getaddrinfo' and 'pr_freeaddrinfo' symbols to be 'getaddrinfo' and
 * 'freeaddrinfo', respectively.
 */
# define pr_getaddrinfo         getaddrinfo
# define pr_freeaddrinfo        freeaddrinfo
#else
int pr_getaddrinfo(const char *, const char *, const struct addrinfo *,
  struct addrinfo **);
void pr_freeaddrinfo(struct addrinfo *);
#endif /* HAVE_GETNAMEINFO and !PR_USE_GETNAMEINFO */

/* These AI_ defines are for use by getaddrinfo(3). */

/* Indicates that the socket is intended for bind()+listen(). */
#ifndef AI_PASSIVE
# define AI_PASSIVE     1
#endif /* AI_PASSIVE */

/* Return the canonical name. */
#ifndef AI_CANONNAME
# define AI_CANONNAME   2
#endif /* AI_CANONNAME */

/* The following EAI_ defines are for errors. */

/* Host address family not supported. */
#ifndef EAI_ADDRFAMILY
# define EAI_ADDRFAMILY -1
#endif /* EAI_ADDRFAMILY */

/* Temporary failure in name resolution. */
#ifndef EAI_AGAIN
# define EAI_AGAIN      -2
#endif /* EAI_AGAIN */

/* Invalid value for ai_flags. */
#ifndef EAI_BADFLAGS
# define EAI_BADFLAGS   -3
#endif /* EAI_BADFLAGS */

/* Non-recoverable failure in name resolution. */
#ifndef EAI_FAIL
# define EAI_FAIL       -4
#endif /* EAI_FAIL */

/* ai_family not supported. */
#ifndef EAI_FAMILY
# define EAI_FAMILY     -5
#endif /* EAI_FAMILY */

/* Memory allocation failure. */
#ifndef EAI_MEMORY
# define EAI_MEMORY     -6
#endif /* EAI_MEMORY */

/* No address associated with host. */
#ifndef EAI_NODATA
# define EAI_NODATA     -7
#endif /* EAI_NODATA */

/* Host nor service not provided, or not known. */
#ifndef EAI_NONAME
# define EAI_NONAME     -8
#endif /* EAI_NONAME */

/* Service not supported for ai_socktype. */
#ifndef EAI_SERVICE
# define EAI_SERVICE    -9
#endif /* EAI_SERVICE */

/* ai_socktype not supported. */
#ifndef EAI_SOCKTYPE
# define EAI_SOCKTYPE   -10
#endif /* EAI_SOCKTYPE */

/* System error contained in errno. */
#ifndef EAI_SYSTEM
# define EAI_SYSTEM     -11
#endif /* EAI_SYSTEM */

#if defined(HAVE_GETNAMEINFO) && !defined(PR_USE_GETNAMEINFO)
/* Use the system getnameinfo(2) by redefining the 'pr_getnameinfo' symbol
 * to be simply 'getnameinfo'.
 */
# define pr_getnameinfo         getnameinfo
#else
int pr_getnameinfo(const struct sockaddr *, socklen_t, char *, size_t,
  char *, size_t, int);
#endif /* HAVE_GETNAMEINFO and !PR_USE_GETNAMEINFO */

/* These NI_ defines are for use by getnameinfo(3). */

/* Max hostname length returned. */
#ifndef NI_MAXHOST
# define NI_MAXHOST     1025
#endif /* NI_MAXHOST */

/* Max service name length returned. */
#ifndef NI_MAXSERV
# define NI_MAXSERV     32
#endif /* NI_MAXSERV */

/* Do not return FQDNs. */
#ifndef NI_NOFQDN
# define NI_NOFQDN      1
#endif /* NI_NOFQDN */

/* Return the numeric form of the hostname. */
#ifndef NI_NUMERICHOST
# define NI_NUMERICHOST 2
#endif /* NI_NUMERICHOST */

/* Return an error if hostname is not found. */
#ifndef NI_NAMEREQD
# define NI_NAMEREQD    4
#endif /* NI_NAMEREQD */

/* Return the numeric form of the service name. */
#ifndef NI_NUMERICSERV
# define NI_NUMERICSERV 8
#endif /* NI_NUMERICSERV */

/* Datagram service for getservbyname(). */
#ifndef NI_DGRAM
# define NI_DGRAM       16
#endif /* NI_DGRAM */


#if defined(HAVE_INET_NTOP)
/* Use the system inet_ntop(3) by redefining the 'pr_inet_ntop' symbol to be
 * 'inet_ntop'.
 */
# define pr_inet_ntop           inet_ntop
#else
const char *pr_inet_ntop(int, const void *, char *, size_t);
#endif

#if defined(HAVE_INET_PTON)
/* Use the system inet_pton(3) by redefining the 'pr_inet_pton' symbol to be
 * 'inet_pton'.
 */
# define pr_inet_pton           inet_pton
#else
int pr_inet_pton(int, const char *, void *);
#endif

/* Network Address API
 */

/* Allocate an initialized netaddr from the given pool. */
pr_netaddr_t *pr_netaddr_alloc(pool *);

/* Duplicate a netaddr using the given pool. */
pr_netaddr_t *pr_netaddr_dup(pool *, pr_netaddr_t *);

/* Initialize the given netaddr. */
void pr_netaddr_clear(pr_netaddr_t *);

/* Given a name (either an IP address string or a DNS name), return a
 * pr_netaddr_t * for that name.  In the case of DNS names, multiple
 * addresses might be associated with given name; callers that are interested
 * in these additional addresses should provide a pointer to an array_header *,
 * which will be filled with an array_header (allocated from the given pool)
 * that contains a list of additional pr_netaddr_t *'s.
 *
 * If there is a failure in resolving the given name to its address(es),
 * NULL will be return, and an error logged.
 */
pr_netaddr_t *pr_netaddr_get_addr(pool *, const char *, array_header **);

/* Compare the two given pr_netaddr_ts.  In order for the comparison to
 * be accurate, the pr_netaddr_ts must be of the same family (AF_INET or
 * AF_INET6).  In the case where the pr_netaddr_ts are from different
 * families, -1 will be returned, with errno set to EINVAL. Otherwise,
 * the comparison is a fancy memcmp().
 */
int pr_netaddr_cmp(const pr_netaddr_t *, const pr_netaddr_t *);

/* Compare the first N bits of the two given pr_netaddr_ts.  In order for
 * the comparison to be accurate, the pr_netaddr_ts must be of the same family
 * (AF_INET or AF_INET6).  In the case where the pr_netaddr_ts are from
 * different families, -1 will be returned, with errno set to EINVAL.
 * Otherwise, the comparison is a fancy memcmp().
 */
int pr_netaddr_ncmp(const pr_netaddr_t *, const pr_netaddr_t *, unsigned int);

/* Compare the given pr_netaddr_t against a glob pattern, as intended for
 * fnmatch(3).  The flags parameter is an OR of the following values:
 * PR_NETADDR_MATCH_DNS and PR_NETADDR_MATCH_IP.  If the PR_NETADDR_MATCH_DNS
 * flag is used, the given pattern will be matched against the DNS string of
 * the netaddr, if present.  If that doesn't match, and if the
 * PR_NETADDR_MATCH_IP flag is used, a comparison against the IP address string
 * will be tried.  A return value of -1, with errno set to EINVAL, occurs if
 * the netaddr or pattern are NULL.  Otherwise, TRUE is returned if the address
 * is matched by the pattern, or FALSE if is not matched.
 */
int pr_netaddr_fnmatch(pr_netaddr_t *, const char *, int);
#define PR_NETADDR_MATCH_DNS		0x001
#define PR_NETADDR_MATCH_IP		0x002

/* Returns the size of the contained address (or -1, with errno set to EINVAL,
 * if NULL is used as the argument).  If the pr_netaddr_t is of the AF_INET
 * family, the size of struct sockaddr_in is returned; if of the AF_INET6
 * family, the size of struct sockaddr_in6 is returned.
 */
size_t pr_netaddr_get_sockaddr_len(const pr_netaddr_t *);

/* Returns the size of the contained address (or -1, with errno set to EINVAL,
 * if NULL is used as the argument).  If the pr_netaddr_t is of the AF_INET
 * family, the size of struct in_addr is returned; if of the AF_INET6
 * family, the size of struct in6_addr is returned.
 */
size_t pr_netaddr_get_inaddr_len(const pr_netaddr_t *);

/* Returns the family of the given pr_netaddr_t, either AF_INET or AF_INET6.
 * A NULL pr_netaddr_t will result in -1 being returned, and errno set to
 * EINVAL.
 */
int pr_netaddr_get_family(const pr_netaddr_t *);

/* Sets the family on the given pr_netaddr_t.  Returns 0 on success, or
 * -1 on error (as when NULL is used as the argument).
 */
int pr_netaddr_set_family(pr_netaddr_t *, int);

/* Returns a void * pointing to either a struct in_addr (if the family of the
 * given pr_netaddr_t is AF_INET) or a struct in6_addr (if the family of the
 * given pr_netaddr_t is AF_INET6).  Returns NULL on error.
 */
void *pr_netaddr_get_inaddr(const pr_netaddr_t *);

/* Returns a struct sockaddr * (pointing to either a struct sockaddr_in or
 * a struct sockaddr_in6, depending on the family), or NULL if there was an
 * error.
 */
struct sockaddr *pr_netaddr_get_sockaddr(const pr_netaddr_t *);

/* Set the contained sockaddr * in the given pr_netaddr_t to be the
 * sockaddr given.  The family of the pr_netaddr_t must have been set
 * first.  Returns 0 on success, and -1 on error.
 */
int pr_netaddr_set_sockaddr(pr_netaddr_t *, struct sockaddr *);

/* Sets the address of the contained sockaddr to be the wildcard address.
 * Returns 0 on success, and -1 on error.
 */
int pr_netaddr_set_sockaddr_any(pr_netaddr_t *);

/* Returns the port of the contained struct sockaddr *. */
unsigned int pr_netaddr_get_port(const pr_netaddr_t *);

/* Sets the port on the contained struct sockaddr *.  Returns 0 on success,
 * or -1 on error (as when NULL is given as the argument).
 */
int pr_netaddr_set_port(pr_netaddr_t *, unsigned int);

/* Enables or disable use of reverse DNS lookups.  Returns the previous
 * setting.
 */
int pr_netaddr_set_reverse_dns(int);

/* Returns the DNS name associated with the given pr_netaddr_t.  If DNS
 * lookups have been disabled, the returned string will be the IP address.
 * Returns NULL if there was an error.
 */
const char *pr_netaddr_get_dnsstr(pr_netaddr_t *);

/* Returns the IP address associated with the given pr_netaddr_t.  Returns
 * NULL if there was an error.
 */
const char *pr_netaddr_get_ipstr(pr_netaddr_t *);

/* Returns the name of the local host, as returned by gethostname(2).  The
 * returned string will be dup'd from the given pool, if any.
 */
const char *pr_netaddr_get_localaddr_str(pool *);

unsigned int pr_netaddr_get_addrno(const pr_netaddr_t *);

/* Returns TRUE if the given pr_netaddr_t contains a loopback address,
 * FALSE otherwise.
 */
int pr_netaddr_loopback(const pr_netaddr_t *);

/* Returns TRUE if the given pr_netaddr_t is of the AF_INET6 family and
 * contains an IPv4-mapped IPv6 address; otherwise FALSE is returned.  A
 * return value of -1 is used to indicate an error.
 */
int pr_netaddr_is_v4mappedv6(const pr_netaddr_t *);

#endif /* PR_NETADDR_H */
