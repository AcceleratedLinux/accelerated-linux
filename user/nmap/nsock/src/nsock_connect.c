
/***************************************************************************
 * nsock_connect.c -- This contains the functions for requesting TCP       *
 * connections from the nsock parallel socket event library                *
 ***********************IMPORTANT NSOCK LICENSE TERMS***********************
 *                                                                         *
 * The nsock parallel socket event library is (C) 1999-2006 Insecure.Com   *
 * LLC This library is free software; you may redistribute and/or          *
 * modify it under the terms of the GNU General Public License as          *
 * published by the Free Software Foundation; Version 2.  This guarantees  *
 * your right to use, modify, and redistribute this software under certain *
 * conditions.  If this license is unacceptable to you, Insecure.Com LLC   *
 * may be willing to sell alternative licenses (contact                    *
 * sales@insecure.com ).                                                   *
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
 * If you received these files with a written license agreement stating    *
 * terms other than the (GPL) terms above, then that alternative license   *
 * agreement takes precedence over this comment.                          *
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
 * insecure.org development mailing lists, it is assumed that you are      *
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
 * General Public License for more details (                               *
 * http://www.gnu.org/copyleft/gpl.html ).                                 *
 *                                                                         *
 ***************************************************************************/

/* $Id: nsock_connect.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "nsock.h"
#include "nsock_internal.h"
#include "netutils.h"

#include <sys/types.h>
#include <errno.h>
#include <string.h>

extern struct timeval nsock_tod;

/* This does the actual logistics of requesting a TCP connection.  It is
 * shared by nsock_connect_tcp and nsock_connect_ssl */
static void nsock_connect_internal(mspool *ms, msevent *nse, int proto,
				   struct sockaddr_storage *ss, size_t sslen,
				   unsigned short port)
{

  int res;
  struct sockaddr_in *sin = (struct sockaddr_in *) ss;
#if HAVE_IPV6
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) ss;
#endif

/* Now it is time to actually attempt the connection */
  if ((nse->iod->sd = (int) socket(sin->sin_family, 
			     (proto == IPPROTO_UDP)? SOCK_DGRAM : SOCK_STREAM, 
			     proto)) == -1) {
    perror("Socket troubles");
    nse->event_done = 1; nse->status = NSE_STATUS_ERROR; nse->errnum = socket_errno();
  } else { 
    nsock_unblock_socket(nse->iod->sd);
    
    if (sin->sin_family == AF_INET) {
      sin->sin_port = htons(port);
    } else {
      assert(sin->sin_family == AF_INET6);
#if HAVE_IPV6
      sin6->sin6_port = htons(port);
#else
      fatal("IPv6 address passed to nsock_connect_* call, but nsock was not compiled w/IPv6 support");
#endif
    }

    assert(sslen <= sizeof(nse->iod->peer));
    memcpy(&nse->iod->peer, ss, sslen);
    nse->iod->peerlen = sslen;
    nse->iod->lastproto = proto;

    if ((res = connect(nse->iod->sd, (struct sockaddr *) ss, sslen)) != -1) {
      nse->event_done = 1;
      nse->status = NSE_STATUS_SUCCESS;
    }
    else {
      int err = socket_errno();

      if (proto != IPPROTO_TCP || (err != EINPROGRESS && err != EAGAIN)) {
        nse->event_done = 1;
        nse->status = NSE_STATUS_ERROR;
        nse->errnum = err;
      }
    }
  }
}


/* Request a TCP connection to another system (by IP address).  The
   in_addr is normal network byte order, but the port number should be
   given in HOST BYTE ORDER.  ss should be a sockaddr_storage,
   sockaddr_in6, or sockaddr_in as appropriate (just like what you
   would pass to connect).  sslen should be the sizeof the structure
   you are passing in. */
nsock_event_id nsock_connect_tcp(nsock_pool nsp, nsock_iod ms_iod, 
				 nsock_ev_handler handler, int timeout_msecs, 
				 void *userdata, struct sockaddr *saddr, 
				 size_t sslen, unsigned short port) {

  msiod *nsi = (msiod *) ms_iod;
  mspool *ms = (mspool *) nsp;
  msevent *nse;
  struct sockaddr_storage *ss = (struct sockaddr_storage *) saddr;
  
  assert(nsi->state == NSIOD_STATE_INITIAL || nsi->state == NSIOD_STATE_UNKNOWN);

  /* Just in case someone waits a long time and then does a new connect */
  gettimeofday(&nsock_tod, NULL);
  
  nse = msevent_new(ms, NSE_TYPE_CONNECT, nsi, timeout_msecs, handler, userdata);
  assert(nse);
  
  if (ms->tracelevel > 0)
    nsock_trace(ms, "TCP connection requested to %s:%hi (IOD #%li) EID %li", 
		inet_ntop_ez(ss, sslen), port, nsi->id, nse->id);
  
  /* Do the actual connect() */ 
  nsock_connect_internal(ms, nse, IPPROTO_TCP, ss, sslen, port); 
  nsp_add_event(ms, nse);
  
  return nse->id;
}

/* Request an SSL over TCP connection to another system (by IP
   address).  The in_addr is normal network byte order, but the port
   number should be given in HOST BYTE ORDER.  This function will call
   back only after it has made the TCP connection AND done the initial
   SSL negotiation.  From that point on, you use the normal read/write
   calls and decryption will happen transparently. ss should be a
   sockaddr_storage, sockaddr_in6, or sockaddr_in as appropriate (just
   like what you would pass to connect).  sslen should be the sizeof
   the structure you are passing in. */
nsock_event_id nsock_connect_ssl(nsock_pool nsp, nsock_iod nsiod, 
				 nsock_ev_handler handler, int timeout_msecs, 
				 void *userdata, struct sockaddr *saddr, 
				 size_t sslen, unsigned short port,
				 nsock_ssl_session ssl_session) {

#ifndef HAVE_OPENSSL
  fatal("nsock_connect_ssl called - but nsock was built w/o SSL support.  QUITTING");
  return (nsock_event_id) 0; /* UNREACHED */
#else
  struct sockaddr_storage *ss = (struct sockaddr_storage *) saddr;
  msiod *nsi = (msiod *) nsiod;
  mspool *ms = (mspool *) nsp;
  msevent *nse;
  static int ssl_initialized = 0;

  /* Just in case someone waits a long time and then does a new connect */
  gettimeofday(&nsock_tod, NULL);

  if (!ssl_initialized) {  
    Nsock_SSL_Init();
    ssl_initialized = 1;
  }
 
  assert(nsi->state == NSIOD_STATE_INITIAL || nsi->state == NSIOD_STATE_UNKNOWN);
  
  nse = msevent_new(ms, NSE_TYPE_CONNECT_SSL, nsi, timeout_msecs, handler, 
		    userdata);
  assert(nse);
  
  // Set our SSL_SESSION so we can benefit from session-id reuse.
  nsi_set_ssl_session(nsi, (SSL_SESSION *) ssl_session);
  
  if (ms->tracelevel > 0)
    nsock_trace(ms, "SSL/TCP connection requested to %s:%hi (IOD #%li) EID %li", inet_ntop_ez(ss, sslen), port, nsi->id, nse->id);
  
  /* Do the actual connect() */ 
  nsock_connect_internal(ms, nse, IPPROTO_TCP, ss, sslen, port); 
  nsp_add_event(ms, nse);
  
  return nse->id;
#endif /* HAVE_OPENSSL */
}

/* Request ssl connection over already established TCP connection.
   nsiod must be socket that is already connected to target
   using nsock_connect_tcp.
   All parameters have the same meaning as in 'nsock_connect_ssl' */
nsock_event_id nsock_reconnect_ssl(nsock_pool nsp, nsock_iod nsiod,
				 nsock_ev_handler handler, int timeout_msecs,
				 void *userdata, nsock_ssl_session ssl_session)
{

#ifndef HAVE_OPENSSL
  fatal("nsock_reconnect_ssl called - but nsock was built w/o SSL support.  QUITTING");
  return (nsock_event_id) 0; /* UNREACHED */
#else
  msiod *nsi = (msiod *) nsiod;
  mspool *ms = (mspool *) nsp;
  msevent *nse;

  Nsock_SSL_Init();

  nse = msevent_new(ms, NSE_TYPE_CONNECT_SSL, nsi, timeout_msecs, handler, userdata);
  assert(nse);

  // Set our SSL_SESSION so we can benefit from session-id reuse.
  nsi_set_ssl_session(nsi, (SSL_SESSION *) ssl_session);

  if (ms->tracelevel > 0)
    nsock_trace(ms, "SSL/TCP reconnection requested (IOD #%li) EID %li", nsi->id, nse->id);

  /* Do the actual connect() */
  nse->event_done = 0;
  nse->status = NSE_STATUS_SUCCESS;
  nsp_add_event(ms, nse);
  return nse->id;
#endif /* HAVE_OPENSSL */
}


/* Request a UDP "connection" to another system (by IP address).  The
   in_addr is normal network byte order, but the port number should be
   given in HOST BYTE ORDER.  Since this is UDP, no packets are
   actually sent.  The destination IP and port are just associated
   with the nsiod (an actual OS connect() call is made).  You can then
   use the normal nsock write calls on the socket.  There is no
   timeout since this call always calls your callback at the next
   opportunity.  The advantages to having a connected UDP socket (as
   opposed to just specifying an address with sendto() are that we can
   now use a consistent set of write/read calls for TCP/UDP, received
   packets from the non-partner are automatically dropped by the OS,
   and the OS can provide asynchronous errors (see Unix Network
   Programming pp224).  ss should be a sockaddr_storage,
   sockaddr_in6, or sockaddr_in as appropriate (just like what you
   would pass to connect).  sslen should be the sizeof the structure
   you are passing in. */
nsock_event_id nsock_connect_udp(nsock_pool nsp, nsock_iod nsiod, 
				 nsock_ev_handler handler, void *userdata, 
				 struct sockaddr *saddr, size_t sslen, 
				 unsigned short port) {

msiod *nsi = (msiod *) nsiod;
mspool *ms = (mspool *) nsp;
msevent *nse;
struct sockaddr_storage *ss = (struct sockaddr_storage *) saddr;

assert(nsi->state == NSIOD_STATE_INITIAL || nsi->state == NSIOD_STATE_UNKNOWN);

/* Just in case someone waits a long time and then does a new connect */
gettimeofday(&nsock_tod, NULL);

nse = msevent_new(ms, NSE_TYPE_CONNECT, nsi, -1, handler, userdata);
assert(nse);

if (ms->tracelevel > 0)
  nsock_trace(ms, "UDP connection requested to %s:%hi (IOD #%li) EID %li", inet_ntop_ez(ss, sslen), port, nsi->id, nse->id);

 nsock_connect_internal(ms, nse, IPPROTO_UDP, ss, sslen, port);
 
 nsp_add_event(ms, nse);

 return nse->id;
}

/* Returns that host/port/protocol information for the last
   communication (or comm. attempt) this nsi has been involved with.
   By "involved" with I mean interactions like establishing (or trying
   to) a connection or sending a UDP datagram through an unconnected
   nsock_iod.  AF is the address family (AF_INET or AF_INET6), Protocl
   is IPPROTO_TCP or IPPROTO_UDP.  Pass NULL for information you do
   not need.  If ANY of the information you requested is not
   available, 0 will be returned and the unavailable sockets are
   zeroed.  If protocol or af is requested but not available, it will
   be set to -1 (and 0 returned).  The pointers you pass in must be
   NULL or point to allocated address space.  The sockaddr members
   should actually be sockaddr_storage, sockaddr_in6, or sockaddr_in
   with the socklen of them set appropriately (eg
   sizeof(sockaddr_storage) if that is what you are passing). */
int nsi_getlastcommunicationinfo(nsock_iod ms_iod, int *protocol,
				 int *af, struct sockaddr *local, 
				 struct sockaddr *remote, size_t socklen) {
  msiod *nsi = (msiod *) ms_iod;
  int ret = 1;
  struct sockaddr_storage sock;
  socklen_t slen = sizeof(struct sockaddr_storage);
  int res;

  assert(socklen > 0);

  if (nsi->peerlen > 0) {
    if (remote)
      memcpy(remote, &(nsi->peer), MIN((unsigned)socklen, nsi->peerlen));
    if (protocol) { 
      *protocol = nsi->lastproto; 
      if (*protocol == -1) res = 0; 
    }
    if (af) {
      *af = ((struct sockaddr_in *) &nsi->peer)->sin_family;
    }
    if (local) {    
      if (nsi->sd >= 0) {
	res = getsockname(nsi->sd, (struct sockaddr *) &sock, &slen);
	if (res == -1) {
	  memset(local, 0, socklen); 
	  ret = 0;
	} else {
	  assert(slen > 0 );
	  memcpy(local, &sock, MIN((unsigned)slen, socklen));
	}
      } else {
	memset(local, 0, socklen); 
	ret = 0;
      }
    }
  } else {
    if (local || remote || protocol || af)
      ret = 0;
    if (remote) memset(remote, 0, socklen);
    if (local) memset(local, 0, socklen);
    if (protocol) *protocol = -1;
    if (*af) *af = -1;
  }
  return ret;
}
