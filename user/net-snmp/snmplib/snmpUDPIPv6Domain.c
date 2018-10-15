#include <net-snmp/net-snmp-config.h>

#ifdef NETSNMP_TRANSPORT_UDPIPV6_DOMAIN

/*
 * hack-o-matic for Cygwin to use winsock2
*/
#if defined(cygwin)
#undef HAVE_UNISTD_H
#undef HAVE_NETINET_IN_H
#undef HAVE_ARPA_INET_H
#undef HAVE_NET_IF_H
#undef HAVE_NETDB_H
#undef HAVE_SYS_PARAM_H
#undef HAVE_SYS_SELECT_H
#undef HAVE_SYS_SOCKET_H
#undef HAVE_IN_ADDR_T
#endif

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if defined(HAVE_WINSOCK_H) || defined(cygwin)
    /*
     *  Windows IPv6 support is part of WinSock2 only
     */
#include <winsock2.h>
#include <ws2tcpip.h>
#undef  HAVE_IF_NAMETOINDEX

#ifndef HAVE_INET_PTON
extern int         inet_pton(int, const char*, void*);
#endif
#ifndef HAVE_INET_NTOP
extern const char *inet_ntop(int, const void*, char*, size_t);
#endif
const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
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
#if HAVE_NET_IF_H
#include <net/if.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#if HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY
#define SS_FAMILY ss_family
#elif HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY
#define SS_FAMILY __ss_family
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPIPv6Domain.h>

oid netsnmp_UDPIPv6Domain[] = { TRANSPORT_DOMAIN_UDP_IPV6 };
static netsnmp_tdomain udp6Domain;

/*
 * from snmpUDPDomain. not static, but not public, either.
 * (ie don't put it in a public header.)
 */
extern void _netsnmp_udp_sockopt_set(int fd, int server);

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.  
 */

static char *
netsnmp_udp6_fmtaddr(netsnmp_transport *t, void *data, int len)
{
    struct sockaddr_in6 *to = NULL;

    DEBUGMSGTL(("netsnmp_udp6", "fmtaddr: t = %p, data = %p, len = %d\n", t,
                data, len));
    if (data != NULL && len == sizeof(struct sockaddr_in6)) {
        to = (struct sockaddr_in6 *) data;
    } else if (t != NULL && t->data != NULL) {
        to = (struct sockaddr_in6 *) t->data;
    }
    if (to == NULL) {
        return strdup("UDP/IPv6: unknown");
    } else {
        char addr[INET6_ADDRSTRLEN];
        char tmp[INET6_ADDRSTRLEN + 18];

        sprintf(tmp, "UDP/IPv6: [%s]:%hu",
                inet_ntop(AF_INET6, (void *) &(to->sin6_addr), addr,
                          INET6_ADDRSTRLEN), ntohs(to->sin6_port));
        return strdup(tmp);
    }
}



/*
 * You can write something into opaque that will subsequently get passed back 
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...  
 */

static int
netsnmp_udp6_recv(netsnmp_transport *t, void *buf, int size,
		  void **opaque, int *olength)
{
    int             rc = -1;
    socklen_t       fromlen = sizeof(struct sockaddr_in6);
    struct sockaddr *from;

    if (t != NULL && t->sock >= 0) {
        from = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
        if (from == NULL) {
            *opaque = NULL;
            *olength = 0;
            return -1;
        } else {
            memset(from, 0, fromlen);
        }

	while (rc < 0) {
	  rc = recvfrom(t->sock, buf, size, 0, from, &fromlen);
	  if (rc < 0 && errno != EINTR) {
	    break;
	  }
	}

        if (rc >= 0) {
	    char *str = netsnmp_udp6_fmtaddr(NULL, from, fromlen);
            DEBUGMSGTL(("netsnmp_udp6",
			"recvfrom fd %d got %d bytes (from %s)\n", t->sock,
                        rc, str));
            free(str);
        } else {
            DEBUGMSGTL(("netsnmp_udp6", "recvfrom fd %d err %d (\"%s\")\n",
			t->sock, errno, strerror(errno)));
        }
        *opaque = (void *) from;
        *olength = sizeof(struct sockaddr_in6);
    }
    return rc;
}



static int
netsnmp_udp6_send(netsnmp_transport *t, void *buf, int size,
		  void **opaque, int *olength)
{
    int rc = -1;
    struct sockaddr *to = NULL;

    if (opaque != NULL && *opaque != NULL &&
        *olength == sizeof(struct sockaddr_in6)) {
        to = (struct sockaddr *) (*opaque);
    } else if (t != NULL && t->data != NULL &&
               t->data_length == sizeof(struct sockaddr_in6)) {
        to = (struct sockaddr *) (t->data);
    }

    if (to != NULL && t != NULL && t->sock >= 0) {
        char *str = netsnmp_udp6_fmtaddr(NULL, (void *)to,
					    sizeof(struct sockaddr_in6));
        DEBUGMSGTL(("netsnmp_udp6", "send %d bytes from %p to %s on fd %d\n",
                    size, buf, str, t->sock));
        free(str);
	while (rc < 0) {
	    rc = sendto(t->sock, buf, size, 0, to,sizeof(struct sockaddr_in6));
	    if (rc < 0 && errno != EINTR) {
		break;
	    }
	}
    }
    return rc;
}



static int
netsnmp_udp6_close(netsnmp_transport *t)
{
    int rc = -1;
    if (t != NULL && t->sock >= 0) {
        DEBUGMSGTL(("netsnmp_udp6", "close fd %d\n", t->sock));
#ifndef HAVE_CLOSESOCKET
        rc = close(t->sock);
#else
        rc = closesocket(t->sock);
#endif
        t->sock = -1;
    }
    return rc;
}



/*
 * Open a UDP/IPv6-based transport for SNMP.  Local is TRUE if addr is the
 * local address to bind to (i.e. this is a server-type session); otherwise
 * addr is the remote address to send things to.  
 */

netsnmp_transport *
netsnmp_udp6_transport(struct sockaddr_in6 *addr, int local)
{
    netsnmp_transport *t = NULL;
    int             rc = 0;
    char           *str = NULL;

    if (addr == NULL || addr->sin6_family != AF_INET6) {
        return NULL;
    }

    t = (netsnmp_transport *) malloc(sizeof(netsnmp_transport));
    if (t == NULL) {
        return NULL;
    }

    str = netsnmp_udp6_fmtaddr(NULL, (void *) addr,
				  sizeof(struct sockaddr_in6));
    DEBUGMSGTL(("netsnmp_udp6", "open %s %s\n", local ? "local" : "remote",
                str));
    free(str);

    memset(t, 0, sizeof(netsnmp_transport));

    t->domain = netsnmp_UDPIPv6Domain;
    t->domain_length =
        sizeof(netsnmp_UDPIPv6Domain) / sizeof(netsnmp_UDPIPv6Domain[0]);

    t->sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (t->sock < 0) {
        netsnmp_transport_free(t);
        return NULL;
    }

    _netsnmp_udp_sockopt_set(t->sock, local);

    if (local) {
        /*
         * This session is inteneded as a server, so we must bind on to the
         * given IP address, which may include an interface address, or could
         * be INADDR_ANY, but certainly includes a port number.
         */

#ifdef IPV6_V6ONLY
        /* Try to restrict PF_INET6 socket to IPv6 communications only. */
        {
	  int one=1;
	  if (setsockopt(t->sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&one, sizeof(one)) != 0) {
	    DEBUGMSGTL(("netsnmp_udp6", "couldn't set IPV6_V6ONLY to %d bytes: %s\n", one, strerror(errno)));
	  } 
	}
#endif

        rc = bind(t->sock, (struct sockaddr *) addr,
		  sizeof(struct sockaddr_in6));
        if (rc != 0) {
            netsnmp_udp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        t->local = (unsigned char*)malloc(18);
        if (t->local == NULL) {
            netsnmp_udp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->local, addr->sin6_addr.s6_addr, 16);
        t->local[16] = (addr->sin6_port & 0xff00) >> 8;
        t->local[17] = (addr->sin6_port & 0x00ff) >> 0;
        t->local_length = 18;
        t->data = NULL;
        t->data_length = 0;
    } else {
        /*
         * This is a client session.  Save the address in the
         * transport-specific data pointer for later use by netsnmp_udp6_send.
         */

        t->data = malloc(sizeof(struct sockaddr_in6));
        if (t->data == NULL) {
            netsnmp_udp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->data, addr, sizeof(struct sockaddr_in6));
        t->data_length = sizeof(struct sockaddr_in6);
        t->remote = (unsigned char*)malloc(18);
        if (t->remote == NULL) {
            netsnmp_udp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->remote, addr->sin6_addr.s6_addr, 16);
        t->remote[16] = (addr->sin6_port & 0xff00) >> 8;
        t->remote[17] = (addr->sin6_port & 0x00ff) >> 0;
        t->remote_length = 18;
    }

    /*
     * 16-bit length field, 8 byte UDP header, 40 byte IPv6 header.  
     */

    t->msgMaxSize = 0xffff - 8 - 40;
    t->f_recv     = netsnmp_udp6_recv;
    t->f_send     = netsnmp_udp6_send;
    t->f_close    = netsnmp_udp6_close;
    t->f_accept   = NULL;
    t->f_fmtaddr  = netsnmp_udp6_fmtaddr;

    return t;
}


/*
 * Not extern but used from here and snmpTCPIPv6Domain.C
 */
int
netsnmp_sockaddr_in6_2(struct sockaddr_in6 *addr,
                       const char *inpeername, const char *default_target)
{
    char           *cp = NULL, *peername = NULL;
    char            debug_addr[INET6_ADDRSTRLEN];
#if HAVE_GETADDRINFO
    struct addrinfo *addrs = NULL;
    struct addrinfo hint;
    int             err;
#elif HAVE_GETIPNODEBYNAME
    struct hostent *hp = NULL;
    int             err;
#elif HAVE_GETHOSTBYNAME
    struct hostent *hp = NULL;
#endif
    int             portno;

    if (addr == NULL) {
        return 0;
    }

    DEBUGMSGTL(("netsnmp_sockaddr_in6",
		"addr %p, peername \"%s\", default_target \"%s\"\n",
                addr, inpeername ? inpeername : "[NIL]",
		default_target ? default_target : "[NIL]"));

    memset(addr, 0, sizeof(struct sockaddr_in6));
    addr->sin6_family = AF_INET6;
    addr->sin6_addr = in6addr_any;
    addr->sin6_port = htons((u_short)SNMP_PORT);

    {
      int port = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
				    NETSNMP_DS_LIB_DEFAULT_PORT);
      if (port != 0)
        addr->sin6_port = htons((u_short)port);
      else if (default_target != NULL)
	netsnmp_sockaddr_in6_2(addr, default_target, NULL);
    }

    if (inpeername != NULL) {
        /*
         * Duplicate the peername because we might want to mank around with
         * it.  
         */

        peername = strdup(inpeername);
        if (peername == NULL) {
            return 0;
        }

        for (cp = peername; *cp && isdigit((unsigned char) *cp); cp++);
        portno = atoi(peername);
        if (!*cp &&  portno != 0) {
            /*
             * Okay, it looks like JUST a port number.  
             */
            DEBUGMSGTL(("netsnmp_sockaddr_in6", "totally numeric: %d\n",
                        portno));
            addr->sin6_port = htons(portno);
            goto resolved;
        }

        /*
         * See if it is an IPv6 address, which covered with square brankets
         * with an appended :port.  
         */
        if (*peername == '[') {
            cp = strchr(peername, ']');
            if (cp != NULL) {
	      /*
	       * See if it is an IPv6 link-local address with interface
	       * name as <zone_id>, like fe80::1234%eth0.
	       * Please refer to the internet draft, IPv6 Scoped Address Architecture
	       * http://www.ietf.org/internet-drafts/draft-ietf-ipngwg-scoping-arch-04.txt
	       *
	       */
	        char *scope_id;
#ifdef HAVE_IF_NAMETOINDEX
	        unsigned int if_index = 0;
#endif
                *cp = '\0';
		scope_id = strchr(peername + 1, '%');
		if (scope_id != NULL) {
		    *scope_id = '\0';
#ifdef HAVE_IF_NAMETOINDEX
		    if_index = if_nametoindex(scope_id + 1);
#endif
		}
                if (*(cp + 1) == ':') {
                    portno = atoi(cp+2);
                    if (portno != 0 &&
                        inet_pton(AF_INET6, peername + 1,
                                  (void *) &(addr->sin6_addr))) {
                        DEBUGMSGTL(("netsnmp_sockaddr_in6",
                                    "IPv6 address with port suffix :%d\n",
                                    portno));
                        if (portno > 0 && portno < 0xffff) {
                            addr->sin6_port = htons(portno);
                        } else {
                            DEBUGMSGTL(("netsnmp_sockaddr_in6", "invalid port number: %d", portno));
                            return 0;
                        }

#if defined(HAVE_IF_NAMETOINDEX) && defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID)
                        addr->sin6_scope_id = if_index;
#endif
                        goto resolved;
                    }
                } else {
                    if (inet_pton
                        (AF_INET6, peername + 1,
                         (void *) &(addr->sin6_addr))) {
                        DEBUGMSGTL(("netsnmp_sockaddr_in6",
                                    "IPv6 address with square brankets\n"));
                        portno = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
				                    NETSNMP_DS_LIB_DEFAULT_PORT);
                        if (portno <= 0)
                            portno = SNMP_PORT;
                        addr->sin6_port = htons(portno);
#if defined(HAVE_IF_NAMETOINDEX) && defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID)
                        addr->sin6_scope_id = if_index;
#endif
                        goto resolved;
                    }
                }
		if (scope_id != NULL) {
		  *scope_id = '%';
		}
		*cp = ']';
            }
        }

        cp = strrchr(peername, ':');
        if (cp != NULL) {
	    char *scope_id;
#ifdef HAVE_IF_NAMETOINDEX
	    unsigned int if_index = 0;
#endif
	    *cp = '\0';
	    scope_id = strchr(peername + 1, '%');
	    if (scope_id != NULL) {
	        *scope_id = '\0';
#ifdef HAVE_IF_NAMETOINDEX
	        if_index = if_nametoindex(scope_id + 1);
#endif
	    }
            portno = atoi(cp + 1);
            if (portno != 0 &&
                inet_pton(AF_INET6, peername,
                          (void *) &(addr->sin6_addr))) {
                DEBUGMSGTL(("netsnmp_sockaddr_in6",
                            "IPv6 address with port suffix :%d\n",
                            atoi(cp + 1)));
                if (portno > 0 && portno < 0xffff) {
                    addr->sin6_port = htons(portno);
                } else {
                    DEBUGMSGTL(("netsnmp_sockaddr_in6", "invalid port number: %d", portno));
                    return 0;
                }

#if defined(HAVE_IF_NAMETOINDEX) && defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID)
                addr->sin6_scope_id = if_index;
#endif
                goto resolved;
            }
	    if (scope_id != NULL) {
	      *scope_id = '%';
	    }
            *cp = ':';
        }

        /*
         * See if it is JUST an IPv6 address.  
         */
        if (inet_pton(AF_INET6, peername, (void *) &(addr->sin6_addr))) {
            DEBUGMSGTL(("netsnmp_sockaddr_in6", "just IPv6 address\n"));
            goto resolved;
        }

        /*
         * Well, it must be a hostname then, possibly with an appended :port.
         * Sort that out first.  
         */

        cp = strrchr(peername, ':');
        if (cp != NULL) {
            *cp = '\0';
            portno = atoi(cp + 1);
            if (portno != 0) {
                DEBUGMSGTL(("netsnmp_sockaddr_in6",
                            "hostname(?) with port suffix :%d\n",
                            portno));
                if (portno > 0 && portno < 0xffff) {
                    addr->sin6_port = htons(portno);
                } else {
                    DEBUGMSGTL(("netsnmp_sockaddr_in6", "invalid port number: %d", portno));
                    return 0;
                }

            } else {
                /*
                 * No idea, looks bogus but we might as well pass the full thing to
                 * the name resolver below.  
                 */
                *cp = ':';
                DEBUGMSGTL(("netsnmp_sockaddr_in6",
                            "hostname(?) with embedded ':'?\n"));
            }
            /*
             * Fall through.  
             */
        }

        if (peername[0] == '\0') {
          DEBUGMSGTL(("netsnmp_sockaddr_in6", "empty hostname\n"));
          free(peername);
          return 0;
        }

#if HAVE_GETADDRINFO
        memset(&hint, 0, sizeof hint);
        hint.ai_flags = 0;
        hint.ai_family = PF_INET6;
        hint.ai_socktype = SOCK_DGRAM;
        hint.ai_protocol = 0;

        err = getaddrinfo(peername, NULL, &hint, &addrs);
        if (err != 0) {
#if HAVE_GAI_STRERROR
            snmp_log(LOG_ERR, "getaddrinfo(\"%s\", NULL, ...): %s\n", peername,
                     gai_strerror(err));
#else
            snmp_log(LOG_ERR, "getaddrinfo(\"%s\", NULL, ...): (error %d)\n",
                     peername, err);
#endif
            free(peername);
            return 0;
        }
        if (addrs != NULL) {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (resolved okay)\n"));
        memcpy(&addr->sin6_addr,
               &((struct sockaddr_in6 *) addrs->ai_addr)->sin6_addr,
               sizeof(struct in6_addr));
		freeaddrinfo(addrs);
        }
		else {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "Failed to resolve IPv6 hostname\n"));
		}
#elif HAVE_GETIPNODEBYNAME
        hp = getipnodebyname(peername, AF_INET6, 0, &err);
        if (hp == NULL) {
            DEBUGMSGTL(("netsnmp_sockaddr_in6",
                        "hostname (couldn't resolve = %d)\n", err));
            free(peername);
            return 0;
        }
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (resolved okay)\n"));
        memcpy(&(addr->sin6_addr), hp->h_addr, hp->h_length);
#elif HAVE_GETHOSTBYNAME
        hp = gethostbyname(peername);
        if (hp == NULL) {
            DEBUGMSGTL(("netsnmp_sockaddr_in6",
                        "hostname (couldn't resolve)\n"));
            free(peername);
            return 0;
        } else {
            if (hp->h_addrtype != AF_INET6) {
                DEBUGMSGTL(("netsnmp_sockaddr_in6",
                            "hostname (not AF_INET6!)\n"));
                free(peername);
                return 0;
            } else {
                DEBUGMSGTL(("netsnmp_sockaddr_in6",
                            "hostname (resolved okay)\n"));
                memcpy(&(addr->sin6_addr), hp->h_addr, hp->h_length);
            }
        }
#else                           /*HAVE_GETHOSTBYNAME */
        /*
         * There is no name resolving function available.  
         */
        snmp_log(LOG_ERR,
                 "no getaddrinfo()/getipnodebyname()/gethostbyname()\n");
        free(peername);
        return 0;
#endif                          /*HAVE_GETHOSTBYNAME */
    } else {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "NULL peername"));
        return 0;
    }

  resolved:
    DEBUGMSGTL(("netsnmp_sockaddr_in6", "return { AF_INET6, [%s]:%hu }\n",
                inet_ntop(AF_INET6, &addr->sin6_addr, debug_addr,
                          sizeof(debug_addr)), ntohs(addr->sin6_port)));
    free(peername);
    return 1;
}


int
netsnmp_sockaddr_in6(struct sockaddr_in6 *addr,
                     const char *inpeername, int remote_port)
{
    char buf[sizeof(remote_port) * 3 + 2];
    sprintf(buf, ":%u", remote_port);
    return netsnmp_sockaddr_in6_2(addr, inpeername, remote_port ? buf : NULL);
}

/*
 * int
 * inet_make_mask_addr( int pf, void *dst, int masklength )
 *      convert from bit length specified masklength to network format, 
 *      which fills 1 from until specified bit length.
 *      dst is usally the structer of sockaddr_in or sockaddr_in6. 
 *      makelength must be an interger from 0 to 32 if pf is PF_INET,
 *      or from 0 to 128 if pf is PF_INET6.
 * return:
 *      0 if the input data, masklength was valid for 
 *      the specified protocol family.
 *      -1 if the the input data wasn't valid.
 */

int
inet_make_mask_addr(int pf, void *dst, int masklength)
{

    unsigned long   Mask = 0;
    int             maskBit = 0x80000000L;
    unsigned char   mask = 0;
    unsigned char   maskbit = 0x80L;
    int             i, j, jj;


    switch (pf) {
    case PF_INET:
        if (masklength < 0 || masklength > 32)
            return -1;

        ((struct in_addr *) dst)->s_addr = 0;

        while (masklength--) {
            Mask |= maskBit;
            maskBit >>= 1;
        }
        ((struct in_addr *) dst)->s_addr = htonl(Mask);
        break;

    case PF_INET6:
        if (masklength < 0 || masklength > 128)
            return -1;


        for (i = 0; i < 16; i++) {
            (*(u_char *) (&((struct in6_addr *) dst)->s6_addr[i])) = 0x00;
        }

        j = (int) masklength / 8;
        jj = masklength % 8;

        for (i = 0; i < j; i++) {
            (*(u_char *) (&((struct in6_addr *) dst)->s6_addr[i])) = 0xff;
        }
        while (jj--) {
            mask |= maskbit;
            maskbit >>= 1;
        }

	if (j < sizeof (((struct in6_addr *) dst)->s6_addr)){
	    (*(u_char *) (&((struct in6_addr *) dst)->s6_addr[j])) = mask;
	}

        break;
    default:
        return -1;              /* unsupported protocol family */
    }
    return 0;
}

/*
 * int
 * inet_addr_complement( int pf, void *src, void *dst )
 *      convert from src to dst, which all bits 
 *      are bit-compliment of src.
 *      Src, dst are ususally sockaddr_in or sockaddr_in6.  
 * return:
 *      0 if the input data src and dst have the same size
 *      -1 if the the input data wasn't valid.
 */

int
inet_addr_complement(int pf, void *src, void *dst)
{

    int             i;

    if (sizeof(src) != sizeof(dst))
        return -1;

    switch (pf) {
    case PF_INET:
        ((struct in_addr *) dst)->s_addr =
            ~((struct in_addr *) src)->s_addr;
        break;
    case PF_INET6:
        for (i = 0; i < 16; i++) {
            (*(u_char *) (&((struct in6_addr *) dst)->s6_addr[i])) =
                (~(*(u_char *) (&((struct in6_addr *) src)->s6_addr[i])))
                & 0xff;
        }
        break;
    default:
        return -1;
    }
    return 0;
}

/*
 * int
 * inet_addr_and( int pf, void *src1, void *src2, void *dst) 
 *      take AND operation on src1 and src2, and output the result to dst.
 *      Src1, src2, and dst are ususally sockaddr_in or sockaddr_in6.  
 * return:
 *      0 if the input data src and dst have the same size
 *      -1 if the the input data are not the same size
 */

int
inet_addr_and(int pf, void *src1, void *src2, void *dst)
{
    int             i;

    if (sizeof(src1) != sizeof(src2) || sizeof(src2) != sizeof(dst))
        return -1;

    switch (pf) {
    case PF_INET:
        ((struct in_addr *) dst)->s_addr =
            ((struct in_addr *) src1)->s_addr & ((struct in_addr *) src2)->
            s_addr;
        break;

    case PF_INET6:
        for (i = 0; i < 16; i++) {
            (*(u_char *) (&((struct in6_addr *) dst)->s6_addr[i])) =
                (*(u_char *) (&((struct in6_addr *) src1)->s6_addr[i])) &
                (*(u_char *) (&((struct in6_addr *) src2)->s6_addr[i]));
        }
        break;
    default:
        return -1;
    }
    return 0;
}


/*
 * int
 * inet_addrs_consistence (int pf, void *net, void *mask ) 
 *      This function checks if the network address net is consistent
 *      with the netmask address, mask.
 *      Net and mask are ususally sockaddr_in or sockaddr_in6.  
 * Note:
 *      Must spefiey protocol family in pf.
 * return:
 *      0 if there is no consistence with address "net" and "mask".
 *      -1 if network address is inconsistent with netmask address, for 
 *      instance, network address is 192.168.0.128 in spite of netmask, 
 *      which is 255.255.255.0. 
 *      The case that the size of net and mask are different also returns -1.
 */

int
inet_addrs_consistence(int pf, void *net, void *mask)
{
    struct sockaddr_in *tmp, *dst;
    struct sockaddr_in6 *tmp6, *dst6;
    int             ret;

    switch (pf) {
    case PF_INET:
        tmp = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
        if (!tmp) {
            config_perror("Resource failure in inet_addr_consistence()");
            return -1;
        }
        memset(tmp, 0, sizeof(*tmp));
        tmp->sin_family = PF_INET;
        if (inet_addr_complement
            (PF_INET, (struct in_addr *) mask, &tmp->sin_addr) != 0) {
            config_perror("Fail in function of inet_addr_complement()");
            free(tmp);
            return -1;
        }
        dst = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
        if (!dst) {
            config_perror("Resource failure in inet_addr_consistence()");
            free(tmp);
            return -1;
        }
        memset(dst, 0, sizeof(*dst));
        dst->sin_family = PF_INET;
        if (inet_addr_and
            (PF_INET, (struct in_addr *) net, &tmp->sin_addr,
             &dst->sin_addr) != 0) {
            config_perror("Fail in function of inet_addr_and()");
            free(dst);
            free(tmp);
            return -1;
        }
        ret = ((dst->sin_addr.s_addr == INADDR_ANY) ? 0 : -1);
        free(dst);
        free(tmp);
        break;
    case PF_INET6:
        tmp6 = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
        if (!tmp6) {
            config_perror("Resource failure in inet_addr_consistence()");
            return -1;
        }
        memset(tmp6, 0, sizeof(*tmp6));
        tmp6->sin6_family = PF_INET6;
        if (inet_addr_complement
            (PF_INET6, (struct in6_addr *) mask, &tmp6->sin6_addr) != 0) {
            config_perror("Fail in function of inet_addr_complement()");
            free(tmp6);
            return -1;
        }
        dst6 = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
        if (!dst6) {
            config_perror("Resource failure in inet_addr_consistence()");
            free(tmp6);
            return -1;
        }
        memset(dst6, 0, sizeof(*dst6));
        dst6->sin6_family = PF_INET6;
        if (inet_addr_and
            (PF_INET6, (struct in6_addr *) net, &tmp6->sin6_addr,
             &dst6->sin6_addr)) {
            config_perror("Fail in function of inet_addr_and()");
            free(dst6);
            free(tmp6);
            return -1;
        }
        ret = (IN6_IS_ADDR_UNSPECIFIED(&dst6->sin6_addr) == 1 ? 0 : -1);
        free(dst6);
        free(tmp6);
        break;
    default:
        return -1;
    }
    return ret;
}

/*
 * int
 * masked_address_are_equal (pf, from, mask, network) 
 *      This function takes AND operation on address "from" and "mask",
 *      and check the result is equal to address "network". 
 *      From, net and mask are ususally sockaddr_in or sockaddr_in6.  
 * Note:
 *      Must spefiey protocol family in pf.
 * return:
 *      0 if address "from" masked by address "mask" is eqaul to 
 *      address "network". 
 *      -1 if address "from" masked by address "mask" isn't eqaul to 
 *      address "network". For instance, address "from" is 
 *       192.168.0.129 and "mask" is 255.255.255.128. Then, masked 
 *      address is 192.168.0.128. If address "network" is 192.168.0.128,
 *      return 0, otherwise -1.
 *      Also retunn -1 if each address family of from, mask, network
 *      isn't the same.
 */

int
masked_address_are_equal(int af, struct sockaddr_storage *from,
                         struct sockaddr_storage *mask,
                         struct sockaddr_storage *network)
{

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));

    switch (af) {
    case PF_INET:
        if (mask->SS_FAMILY != PF_INET || network->SS_FAMILY != PF_INET) {
            return -1;
        }
        ss.SS_FAMILY = PF_INET;
        inet_addr_and(PF_INET,
                      &((struct sockaddr_in *) from)->sin_addr,
                      &((struct sockaddr_in *) mask)->sin_addr,
                      &((struct sockaddr_in *) &ss)->sin_addr);
        if (((struct sockaddr_in *) &ss)->sin_addr.s_addr ==
            ((struct sockaddr_in *) network)->sin_addr.s_addr) {
            return 0;
        } else {
            return -1;
        }
        break;
    case PF_INET6:
        if (mask->SS_FAMILY != PF_INET6 || network->SS_FAMILY != PF_INET6) {
            return -1;
        }
        ss.SS_FAMILY = PF_INET6;
        inet_addr_and(PF_INET6,
                      &((struct sockaddr_in6 *) from)->sin6_addr,
                      &((struct sockaddr_in6 *) mask)->sin6_addr,
                      &((struct sockaddr_in6 *) &ss)->sin6_addr);
#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(a,b) IN6_ADDR_EQUAL(a,b)
#endif
        if (IN6_ARE_ADDR_EQUAL(&((struct sockaddr_in6 *) &ss)->sin6_addr,
                               &((struct sockaddr_in6 *) network)->
                               sin6_addr) == 1) {
            return 0;
        } else {
            return -1;
        }
        break;
    default:
        return -1;
    }
}

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
/*
 * The following functions provide the "com2sec6" configuration token
 * functionality for compatibility.  
 */

#define EXAMPLE_NETWORK       "NETWORK"
#define EXAMPLE_COMMUNITY     "COMMUNITY"

typedef struct _com2Sec6Entry {
    char            community[COMMUNITY_MAX_LEN];
    struct sockaddr_in6 network;
    struct sockaddr_in6 mask;
    char            secName[VACMSTRINGLEN];
    char            contextName[VACMSTRINGLEN];
    struct _com2Sec6Entry *next;
} com2Sec6Entry;

com2Sec6Entry  *com2Sec6List = NULL, *com2Sec6ListLast = NULL;


void
memmove_com2Sec6Entry(com2Sec6Entry * c,
                      char *secName,
                      char *community,
                      struct sockaddr_in6 net, struct sockaddr_in6 mask,
                      char *contextName)
{
    snprintf(c->secName, strlen(secName) + 1, "%s", secName);
    snprintf(c->contextName, strlen(contextName) + 1, "%s", contextName);
    snprintf(c->community, strlen(community) + 1, "%s", community);
    memmove(&c->network, &net, sizeof(net));
    memmove(&c->mask, &mask, sizeof(mask));
    c->next = NULL;
}


#ifndef IPV6_STRING_LEN
#define IPV6_STRING_LEN 55
#endif

void
netsnmp_udp6_parse_security(const char *token, char *param)
{
    char            secName[VACMSTRINGLEN];
    char            contextName[VACMSTRINGLEN];
    char            community[COMMUNITY_MAX_LEN];
    char            source[IPV6_STRING_LEN];
    char           *cp = NULL, *strnetwork = NULL, *strmask = NULL;
    com2Sec6Entry  *e = NULL;
    struct sockaddr_in6 net, mask;
    struct sockaddr_in tmp;

    memset(&net, 0, sizeof(net));
    memset(&mask, 0, sizeof(mask));
    memset(&tmp, 0, sizeof(tmp));
    net.sin6_family = AF_INET6;
    mask.sin6_family = AF_INET6;
    tmp.sin_family = AF_INET;


    /*
     * Get security, source address/netmask and community strings.  
     */
    cp = copy_nword( param, secName, sizeof(secName));
    if (strcmp(secName, "-Cn") == 0) {
        if (!cp) {
            config_perror("missing CONTEXT_NAME parameter");
            return;
        }
        cp = copy_nword( cp, contextName, sizeof(contextName));
        cp = copy_nword( cp, secName, sizeof(secName));
    } else {
        contextName[0] = '\0';
    }
    if (secName[0] == '\0') {
        config_perror("missing NAME parameter");
        return;
    } else if (strlen(secName) > (VACMSTRINGLEN - 1)) {
        config_perror("security name too long");
        return;
    }
    cp = copy_nword( cp, source, sizeof(source));
    if (source[0] == '\0') {
        config_perror("missing SOURCE parameter");
        return;
    } else if (strncmp(source, EXAMPLE_NETWORK, strlen(EXAMPLE_NETWORK)) ==
               0) {
        config_perror("example config NETWORK not properly configured");
        return;
    }
    cp = copy_nword( cp, community, sizeof(community));
    if (community[0] == '\0') {
        config_perror("missing COMMUNITY parameter\n");
        return;
    } else
        if (strncmp
            (community, EXAMPLE_COMMUNITY, strlen(EXAMPLE_COMMUNITY))
            == 0) {
        config_perror("example config COMMUNITY not properly configured");
        return;
    } else if (strlen(community) > (COMMUNITY_MAX_LEN - 1)) {
        config_perror("community name too long");
        return;
    }

    /*
     * Process the source address/netmask string.  
     */
    cp = strchr(source, '/');
    if (cp != NULL) {
        /*
         * Mask given.  
         */
        *cp = '\0';
        strmask = cp + 1;
    }

    /*
     * Deal with the network part first.  
     */
    if ((strcmp(source, "default") == 0) || (strcmp(source, "::") == 0)) {
        if ((strnetwork = strdup("0::0")) != NULL)
	{
	    if ((strmask = strdup("0::0")) != NULL)
	    {
	
		inet_pton(AF_INET6, strnetwork, &net.sin6_addr);
		inet_pton(AF_INET6, strmask, &mask.sin6_addr);
		
		e = (com2Sec6Entry *) malloc(sizeof(com2Sec6Entry));
		if (e != NULL) {
		    memmove_com2Sec6Entry(e, secName, community, net, mask, contextName);
		    if (com2Sec6ListLast != NULL) {
			com2Sec6ListLast->next = e;
			com2Sec6ListLast = e;
		    } else {
			com2Sec6ListLast = com2Sec6List = e;
		    }
		}
		else {
		    config_perror ("memory error");
		}
		free (strmask);
	    }
	    else {
		DEBUGMSGTL(("netsnmp_udp6_parse_security",
			    "Couldn't allocate enough memory\n"));
	    }
	    free (strnetwork);
	}
	else {
	    DEBUGMSGTL(("netsnmp_udp6_parse_security",
			"Couldn't allocate enough memory\n"));
	}
    } else {
        /*
         * Try interpreting as IPv6 address.  
         */
        if (inet_pton(AF_INET6, source, &net.sin6_addr) == 1) {
            if (strmask == NULL || *strmask == '\0') {
                inet_make_mask_addr(PF_INET6, &mask.sin6_addr, 128);
            } else {
                if (strchr(strmask, ':')) {
                    if (inet_pton(PF_INET6, strmask, &net.sin6_addr) != 1) {
                        config_perror("bad mask");
                        return;
                    }
                } else {
                    if (inet_make_mask_addr
                        (PF_INET6, &mask.sin6_addr, atoi(strmask)) != 0) {
                        config_perror("bad mask");
                        return;

                    }
                }
            }
            /*
             * Check that the network and mask are consistent.  
             */
            if (inet_addrs_consistence
                (PF_INET6, &net.sin6_addr, &mask.sin6_addr) != 0) {
                config_perror("source/mask mismatch");
                return;
            }

            e = (com2Sec6Entry *) malloc(sizeof(com2Sec6Entry));
            if (e == NULL) {
                config_perror("memory error");
                return;
            }

            /*
             * Everything is okay.  Copy the parameters to the structure allocated
             * above and add it to END of the list.  
             */
            if (strmask != NULL && strnetwork != NULL) {
                DEBUGMSGTL(("netsnmp_udp6_parse_security",
                            "<\"%s\", %s/%s> => \"%s\"\n", community,
                            strnetwork, strmask, secName));
                free(strmask);
                free(strnetwork);
            } else {
                DEBUGMSGTL(("netsnmp_udp6_parse_security",
                            "Couldn't allocate enough memory\n"));
            }
            memmove_com2Sec6Entry(e, secName, community, net, mask,
                                  contextName);
            if (com2Sec6ListLast != NULL) {
                com2Sec6ListLast->next = e;
                com2Sec6ListLast = e;
            } else {
                com2Sec6ListLast = com2Sec6List = e;
            }

#if HAVE_GETADDRINFO

        } else {
            /*
             * Nope, Must be a hostname.  
             */
            struct addrinfo hints, *ai, *res;
            char            hbuf[NI_MAXHOST];
            int             gai_error;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = PF_INET6;
            hints.ai_socktype = SOCK_DGRAM;
            if ((gai_error = getaddrinfo(source, NULL, &hints, &res)) != 0) {
                config_perror(gai_strerror(gai_error));
                return;
            }

            for (ai = res; ai != NULL; ai = ai->ai_next) {
                if (getnameinfo
                    (ai->ai_addr, ai->ai_addrlen, hbuf, sizeof(hbuf), NULL,
                     0, NI_NUMERICHOST)) {
                    config_perror("getnameinfo failed");
                }
                memmove(&net, ai->ai_addr, sizeof(struct sockaddr_in6));
                inet_make_mask_addr(PF_INET6, &mask.sin6_addr, 128);

                e = (com2Sec6Entry *) malloc(sizeof(com2Sec6Entry));
                if (e == NULL) {
                    config_perror("memory error");
                    return;
                }

                /*
                 * Everything is okay.  Copy the parameters to the structure allocated
                 * above and add it to END of the list.  
                 */
                DEBUGMSGTL(("netsnmp_udp6_parse_security",
                            "<\"%s\", %s> => \"%s\"\n", community, hbuf,
                            secName));
                memmove_com2Sec6Entry(e, secName, community, net, mask,
                                      contextName);
                if (com2Sec6ListLast != NULL) {
                    com2Sec6ListLast->next = e;
                    com2Sec6ListLast = e;
                } else {
                    com2Sec6ListLast = com2Sec6List = e;
                }
            }
            if (res != NULL)
                freeaddrinfo(res);

#endif /* HAVE_GETADDRINFO */

        }
        /*
         * free(strnetwork); 
         */
    }
}

void
netsnmp_udp6_com2Sec6List_free(void)
{
    com2Sec6Entry  *e = com2Sec6List;
    while (e != NULL) {
        com2Sec6Entry  *tmp = e;
        e = e->next;
        free(tmp);
    }
    com2Sec6List = com2Sec6ListLast = NULL;
}

#endif /* support for community based SNMP */

void
netsnmp_udp6_agent_config_tokens_register(void)
{
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    register_app_config_handler("com2sec6", netsnmp_udp6_parse_security,
                                netsnmp_udp6_com2Sec6List_free,
                                "[-Cn CONTEXT] secName IPv6-network-address[/netmask] community");
#endif /* support for community based SNMP */
}



#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
/*
 * Return 0 if there are no com2sec entries, or return 1 if there ARE com2sec 
 * entries.  On return, if a com2sec entry matched the passed parameters,
 * then *secName points at the appropriate security name, or is NULL if the
 * parameters did not match any com2sec entry.  
 */

int
netsnmp_udp6_getSecName(void *opaque, int olength,
                        const char *community,
                        int community_len, char **secName, char **contextName)
{
    com2Sec6Entry  *c;
    struct sockaddr_in6 *from = (struct sockaddr_in6 *) opaque;
    char           *ztcommunity = NULL;
    char            str6[INET6_ADDRSTRLEN];

    if (secName != NULL) {
        *secName = NULL;  /* Haven't found anything yet */
    }

    /*
     * Special case if there are NO entries (as opposed to no MATCHING
     * entries).  
     */

    if (com2Sec6List == NULL) {
        DEBUGMSGTL(("netsnmp_udp6_getSecName", "no com2sec entries\n"));
        return 0;
    }

    /*
     * If there is no IPv6 source address, 
     * then there can be no valid security name.  
     */

    if (opaque == NULL || olength != sizeof(struct sockaddr_in6)
        || from->sin6_family != PF_INET6) {
        DEBUGMSGTL(("netsnmp_udp6_getSecName",
                    "no IPv6 source address in PDU?\n"));
        return 1;
    }

    ztcommunity = (char *) malloc(community_len + 1);
    if (ztcommunity != NULL) {
        memcpy(ztcommunity, community, community_len);
        ztcommunity[community_len] = '\0';
    }

    inet_ntop(AF_INET6, &from->sin6_addr, str6, sizeof(str6));
    DEBUGMSGTL(("netsnmp_udp6_getSecName", "resolve <\"%s\", %s>\n",
                ztcommunity ? ztcommunity : "<malloc error>", str6));

    for (c = com2Sec6List; c != NULL; c = c->next) {
	char str_net[INET6_ADDRSTRLEN], str_mask[INET6_ADDRSTRLEN];
        DEBUGMSGTL(("netsnmp_udp6_getSecName",
                    "compare <\"%s\", %s/%s>", c->community,
                    inet_ntop(AF_INET6, &c->network.sin6_addr, str_net, sizeof str_net),
		    inet_ntop(AF_INET6, &c->mask.sin6_addr, str_mask, sizeof str_mask)));

        if ((community_len == (int)strlen(c->community)) &&
            (memcmp(community, c->community, community_len) == 0) &&
            (masked_address_are_equal(from->sin6_family,
                                      (struct sockaddr_storage *) from,
                                      (struct sockaddr_storage *) &c->mask,
                                      (struct sockaddr_storage *) &c->network)
					== 0)) {
            DEBUGMSG(("netsnmp_udp6_getSecName", "... SUCCESS\n"));
            if (secName != NULL) {
                *secName = c->secName;
                *contextName = c->contextName;
            }
            break;
        }
        DEBUGMSG(("netsnmp_udp6_getSecName", "... nope\n"));
    }
    if (ztcommunity != NULL) {
        free(ztcommunity);
    }
    return 1;
}
#endif /* support for community based SNMP */

netsnmp_transport *
netsnmp_udp6_create_tstring(const char *str, int local,
			    const char *default_target)
{
    struct sockaddr_in6 addr;

    if (netsnmp_sockaddr_in6_2(&addr, str, default_target)) {
        return netsnmp_udp6_transport(&addr, local);
    } else {
        return NULL;
    }
}


/*
 * See:
 * 
 * http://www.ietf.org/internet-drafts/draft-ietf-ops-taddress-mib-01.txt
 * 
 * (or newer equivalent) for details of the TC which we are using for
 * the mapping here.  
 */

netsnmp_transport *
netsnmp_udp6_create_ostring(const u_char * o, size_t o_len, int local)
{
    struct sockaddr_in6 addr;

    if (o_len == 18) {
        memset((u_char *) & addr, 0, sizeof(struct sockaddr_in6));
        addr.sin6_family = AF_INET6;
        memcpy((u_char *) & (addr.sin6_addr.s6_addr), o, 16);
        addr.sin6_port = (o[16] << 8) + o[17];
        return netsnmp_udp6_transport(&addr, local);
    }
    return NULL;
}


void
netsnmp_udp6_ctor(void)
{
    udp6Domain.name = netsnmp_UDPIPv6Domain;
    udp6Domain.name_length = sizeof(netsnmp_UDPIPv6Domain) / sizeof(oid);
    udp6Domain.f_create_from_tstring_new = netsnmp_udp6_create_tstring;
    udp6Domain.f_create_from_ostring = netsnmp_udp6_create_ostring;
    udp6Domain.prefix = (const char**)calloc(5, sizeof(char *));
    udp6Domain.prefix[0] = "udp6";
    udp6Domain.prefix[1] = "ipv6";
    udp6Domain.prefix[2] = "udpv6";
    udp6Domain.prefix[3] = "udpipv6";

    netsnmp_tdomain_register(&udp6Domain);
}

#else

#ifdef NETSNMP_DLL
/* need this hook for win32 MSVC++ DLL build */
void
netsnmp_udp6_agent_config_tokens_register(void)
{ }
#endif

#endif /* NETSNMP_TRANSPORT_UDPIPV6_DOMAIN */

