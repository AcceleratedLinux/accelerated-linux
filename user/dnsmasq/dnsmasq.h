/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* Author's email: simon@thekelleys.org.uk */

/* get these before config.h  for IPv6 stuff... */
#include <sys/types.h>
#include <netinet/in.h>

/* get this before config.h too. */
#include <syslog.h>

#include "config.h"

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#if defined(__sun) || defined(__sun__)
#include <sys/sockio.h>
#endif
#include <sys/time.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#endif
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#define OPT_BOGUSPRIV      1
#define OPT_FILTER         2
#define OPT_LOG            4
#define OPT_SELFMX         8
#define OPT_NO_HOSTS       16
#define OPT_NO_POLL        32
#define OPT_DEBUG          64
#define OPT_ORDER          128
#define OPT_NO_RESOLV      256
#define OPT_EXPAND         512
#define OPT_LOCALMX        1024
#define OPT_NO_NEG         2048
#define OPT_NODOTS_LOCAL   4096

struct all_addr {
  union {
    struct in_addr addr4;
#ifdef HAVE_IPV6
    struct in6_addr addr6;
#endif
  } addr;
};

struct bogus_addr {
  struct in_addr addr;
  struct bogus_addr *next;
};

union bigname {
  char name[MAXDNAME];
  union bigname *next; /* freelist */
};

struct crec { 
  struct crec *next, *prev, *hash_next;
  time_t ttd; /* time to die */
  struct all_addr addr;
  unsigned short flags;
  union {
    char sname[SMALLDNAME];
    union bigname *bname;
  } name;
};

#define F_IMMORTAL  1
#define F_CONFIG    2
#define F_REVERSE   4
#define F_FORWARD   8
#define F_DHCP      16 
#define F_NEG       32       
#define F_HOSTS     64
#define F_IPV4      128
#define F_IPV6      256
#define F_BIGNAME   512
#define F_UPSTREAM  1024
#define F_SERVER    2048
#define F_NXDOMAIN  4096
#define F_QUERY     8192
#define F_ADDN      16384
#define F_NOERR     32768

/* struct sockaddr is not large enough to hold any address,
   and specifically not big enough to hold and IPv6 address.
   Blech. Roll our own. */
union mysockaddr {
  struct sockaddr sa;
  struct sockaddr_in in;
#ifdef HAVE_BROKEN_SOCKADDR_IN6
  /* early versions of glibc don't include sin6_scope_id in sockaddr_in6
     but latest kernels _require_ it to be set. The choice is to have
     dnsmasq fail to compile on back-level libc or fail to run
     on latest kernels with IPv6. Or to do this: sorry that it's so gross. */
  struct my_sockaddr_in6 {
    sa_family_t     sin6_family;    /* AF_INET6 */
    uint16_t        sin6_port;      /* transport layer port # */
    uint32_t        sin6_flowinfo;  /* IPv6 traffic class & flow info */
    struct in6_addr sin6_addr;      /* IPv6 address */
    uint32_t        sin6_scope_id;  /* set of interfaces for a scope */
  } in6;
#elif defined(HAVE_IPV6)
  struct sockaddr_in6 in6;
#endif
};

#define SERV_FROM_RESOLV     1  /* 1 for servers from resolv, 0 for command line. */
#define SERV_NO_ADDR         2  /* no server, this domain is local only */
#define SERV_LITERAL_ADDRESS 4  /* addr is the answer, not the server */ 
#define SERV_HAS_SOURCE      8  /* source address specified */
#define SERV_HAS_DOMAIN     16  /* server for one domain only */
#define SERV_FOR_NODOTS     32  /* server for names with no domain part only */
#define SERV_TYPE    (SERV_HAS_DOMAIN | SERV_FOR_NODOTS)

struct serverfd {
  int fd;
  union mysockaddr source_addr;
  struct serverfd *next;
};

struct server {
  union mysockaddr addr, source_addr;
  struct serverfd *sfd; /* non-NULL if this server has its own fd bound to
			   a source port */
  char *domain; /* set if this server only handles a domain. */ 
  int flags;
  struct server *next; 
};

/* linked list of all the interfaces in the system and 
   the sockets we have bound to each one. */
struct irec {
  union mysockaddr addr;
  int fd;
  int valid;
  struct irec *next;
};

/* interface and address parms from command line. */
struct iname {
  char *name;
  union mysockaddr addr;
  struct iname *next;
  int found;
};

/* resolv-file parms from command-line */
struct resolvc {
  struct resolvc *next;
  int is_default;
  int logged;
  char *name;
};

struct frec {
  union mysockaddr source;
  struct server *sentto;
  unsigned short orig_id, new_id;
  int fd;
  time_t time;
  struct frec *next;
};

/* cache.c */
void cache_init(int cachesize, int log);
void log_query(unsigned short flags, char *name, struct all_addr *addr);
struct crec *cache_find_by_addr(struct crec *crecp,
				struct all_addr *addr, time_t now, 
				unsigned short prot);
struct crec *cache_find_by_name(struct crec *crecp, 
				char *name, time_t now, unsigned short  prot);
void cache_end_insert(void);
void cache_start_insert(void);
void cache_insert(char *name, struct all_addr *addr,
		  time_t now, unsigned long ttl, unsigned short flags);
void cache_reload(int opts, char *buff, char *domain_suffix, char *addn_hosts);
void cache_add_dhcp_entry(char *host_name, struct all_addr *host_address, 
			  time_t ttd, unsigned short flags);
void cache_unhash_dhcp(void);
void dump_cache(int debug, int size);
char *cache_get_name(struct crec *crecp);

/* rfc1035.c */
unsigned short extract_request(HEADER *header, unsigned int qlen, char *name);
int setup_reply(HEADER *header, unsigned int qlen,
		struct all_addr *addrp, unsigned short flags,
		unsigned long local_ttl);
void extract_addresses(HEADER *header, unsigned int qlen, char *namebuff, time_t now);
void extract_neg_addrs(HEADER *header, unsigned int qlen, char *namebuff, time_t now);
int answer_request(HEADER *header, char *limit, unsigned int qlen, char *mxname, 
		   char *mxtarget, unsigned int options, time_t now, unsigned long local_ttl,
		   char *namebuff);
int check_for_bogus_wildcard(HEADER *header, unsigned int qlen, char *name, 
			     struct bogus_addr *addr, time_t now);

/* dhcp.c */
void load_dhcp(char *file, char *suffix, time_t now, char *hostnamebuff);

/* util.c */
unsigned short rand16(void);
int legal_char(char c);
int canonicalise(char *s);
void die(char *message, char *arg1);
void *safe_malloc(int size);
char *safe_string_alloc(char *cp);
int sa_len(union mysockaddr *addr);
int sockaddr_isequal(union mysockaddr *s1, union mysockaddr *s2);
int hostname_isequal(unsigned char *a, unsigned char *b);

/* option.c */
unsigned int read_opts(int argc, char **argv, char *buff, struct resolvc **resolv_file, 
		       char **mxname, char **mxtarget, char **lease_file, 
		       char **username, char **groupname, 
		       char **domain_suffix, char **runfile, 
		       struct iname **if_names, struct iname **if_addrs, struct iname **if_except, 
		       struct bogus_addr **bogus_addr, struct server **serv_addrs, int *cachesize, 
		       int *port, int *query_port, unsigned long *local_ttl, char **addn_hosts);

/* forward.c */
void forward_init(int first);
void reap_forward(int fd);
struct server *forward_query(int udpfd, union mysockaddr *udpaddr, HEADER *header, 
			     int plen, unsigned int options, char *dnamebuff, 
			     struct server *servers, struct server *last_server,
			     time_t now, unsigned long local_ttl);
struct server *reply_query(int fd, int options, char *packet, time_t now,
			   char *dnamebuff, struct server *last_server, 
			   struct bogus_addr *bogus_nxdomain);

/* network.c */
struct server *reload_servers(char *fname, char *buff, struct server *servers, int query_port);
struct server *check_servers(struct server *new, struct irec *interfaces, struct serverfd **sfds);
char *enumerate_interfaces(struct irec **interfaces,
			   struct iname *names, 
			   struct iname *addrs, 
			   struct iname *except,
			   int port);
