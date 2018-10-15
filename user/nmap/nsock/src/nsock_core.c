/***************************************************************************
 * nsock_core.c -- This contains the core engine routines for the nsock    *
 * parallel socket event library.                                          *
 *                                                                         *
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

/* $Id: nsock_core.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "nsock_internal.h"
#include "gh_list.h"
#include "filespace.h"

#include <assert.h>
#if HAVE_ERRNO_H
#include <errno.h>
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
#if HAVE_STRING_H
#include <string.h>
#endif

#include "netutils.h"

/* Msock time of day -- we update this at least once per
   nsock_loop round (and after most calls that are likely to block).  
   Other nsock files should grab this 
*/
struct timeval nsock_tod;


/* Returns -1 (and sets ms->errno if there is an error so severe that
   we might as well quit waiting and have nsock_loop() return an error */
static int wait_for_events(mspool *ms, int msec_timeout) {

  int event_msecs; /* Msecs before an event goes off */
  int combined_msecs;
  int sock_err = 0;
  int socketno = 0;
  struct timeval select_tv;
  struct timeval *select_tv_p;

  assert(msec_timeout >= -1);

  if (ms->evl.events_pending == 0)
    return 0; /* No need to wait on 0 events ... */

  do {  
    if (ms->evl.next_ev.tv_sec == 0) {
      event_msecs = -1; /* None of the events specified a timeout */
    } else {
      event_msecs = MAX(0, TIMEVAL_MSEC_SUBTRACT(ms->evl.next_ev, nsock_tod));
    }

    /* We cast to unsigned because we want -1 to be very high (since it means 
       no timeout) */
    combined_msecs = MIN((unsigned) event_msecs, (unsigned) msec_timeout);
    
    // printf("wait_for_events: starting wait -- combined_msecs=%d\n", combined_msecs);
    /* Set up the timeval pointer we will give to select() */
    memset(&select_tv, 0, sizeof(select_tv));
    if (combined_msecs > 0) {
      select_tv.tv_sec = combined_msecs / 1000;
      select_tv.tv_usec = (combined_msecs % 1000) * 1000;
      select_tv_p = &select_tv;
    } else if (combined_msecs == 0) {
      /* we want the tv_sec and tv_usec to be zero -- but they already are from
	 bzero */
      select_tv_p = &select_tv;
    } else {
      assert(combined_msecs == -1);
      select_tv_p = NULL;
    }
    
    /* Figure out whether there are any FDs in the sets, as @$@!$#
       Windows returns WSAINVAL (10022) if you call a select() with no
       FDs, even though the Linux man page says that doing so is a
       good, reasonably portable way to sleep with subsecond
       precision.  Sigh */
    for(socketno = ms->mioi.max_sd; socketno >= 0; socketno--) {
      if(FD_ISSET(socketno, &ms->mioi.fds_master_r) ||
	 FD_ISSET(socketno, &ms->mioi.fds_master_w) ||
	 FD_ISSET(socketno, &ms->mioi.fds_master_x))
	break;
      else ms->mioi.max_sd--;
    }

    if (ms->mioi.max_sd < 0) {
      ms->mioi.results_left = 0;
      if (combined_msecs > 0)
	usleep(combined_msecs * 1000);
    } else {

      /* Set up the descriptors for select */
      ms->mioi.fds_results_r = ms->mioi.fds_master_r;
      ms->mioi.fds_results_w = ms->mioi.fds_master_w;
      ms->mioi.fds_results_x = ms->mioi.fds_master_x;
      
      ms->mioi.results_left = select(ms->mioi.max_sd + 1, &ms->mioi.fds_results_r, &ms->mioi.fds_results_w, &ms->mioi.fds_results_x, select_tv_p);
      if (ms->mioi.results_left == -1)
	sock_err = socket_errno();
    }

    gettimeofday(&nsock_tod, NULL); /* Due to usleep or select delay */
  } while (ms->mioi.results_left == -1 && sock_err == EINTR);
  
  if (ms->mioi.results_left == -1 && sock_err != EINTR) {
    ms->errnum = sock_err;
    return -1;
  }
 
  return 0;
}


  /* A handler function is defined for each of the main event types
     (read, write, connect, timer, etc) -- the handler is called when
     new information is available for the event.  The handler makes
     any neccessary updates to the event based on any new information
     available.  If the event becomes ready for delivery, the handler
     sets nse->event_done and fills out the relevant event fields
     (status, errnum) as applicable.  The handlers also take care of
     event type specific teardown (such as clearing socket descriptors
     from select/poll lists).  If event_done is not set, the handler
     will be called again in the case of more information or an event
     timeout */

  /* The event type handlers -- the first three arguments of each are the same:
     mspool *ms
     msevent *nse -- the event we have new info on
     enum nse_status -- The reason for the call, usually NSE_STATUS_SUCCESS 
                        (which generally means a successful I/O call or 
			NSE_STATUS_TIMEOUT or NSE_STATUS_CANCELLED

     Some of the event type handlers have other parameters, specific
     to their needs.  All the handlers can assume that the calling
     function has checked that select or poll said their descriptors
     were readable/writeable (as appropriate).

     The idea is that each handler will take care of the stuff that is 
     specific to it and the calling function will handle the stuff that
     can be generalized to dispatching/deleting/etc. all events.  But the
     calling function may use type-specific info to determine whether
     the handler should be called at all (to save CPU time).
  */


/* handle_connect_results assumes that select or poll have already
   shown the descriptor to be active */
void handle_connect_result(mspool *ms, msevent *nse, 
				  enum nse_status status)
  {
  int optval;
  socklen_t optlen = sizeof(int);
  char buf[1024];
  msiod *iod = nse->iod;
#if HAVE_OPENSSL
  struct NsockSSLInfo *sslnfo;
  int sslerr;
  int sslconnect_inprogress = nse->type == NSE_TYPE_CONNECT_SSL && iod->ssl;
#else
  int sslconnect_inprogress = 0;
#endif
  int rc;
  rc = 0;

  if (status == NSE_STATUS_TIMEOUT || status == NSE_STATUS_CANCELLED) {
    nse->status = status;
    nse->event_done = 1;
  } else if (sslconnect_inprogress) {
    /* Do nothing */
  } else if (status == NSE_STATUS_SUCCESS) {
    /* First we want to determine whether the socket really is connected */
    if (getsockopt(iod->sd, SOL_SOCKET, SO_ERROR, (char *) &optval, &optlen) != 0)
      optval = socket_errno(); /* Stupid Solaris */
    
    switch(optval) {
    case 0:
#ifdef LINUX
      if (!FD_ISSET(iod->sd, &ms->mioi.fds_results_r)) {
	/* Linux goofiness -- We need to actually test that it is writeable */
	rc = send(iod->sd, "", 0, 0);
	
	if (rc < 0 ) {
	  nse->status = NSE_STATUS_ERROR;
	  nse->errnum = ECONNREFUSED;
	} else {
	  nse->status = NSE_STATUS_SUCCESS;
	}
      } else {
	nse->status = NSE_STATUS_SUCCESS;
      }
#else
      nse->status = NSE_STATUS_SUCCESS;
#endif
      break;
    case ECONNREFUSED:
    case EHOSTUNREACH:
      nse->status = NSE_STATUS_ERROR;
      nse->errnum = optval;
      break;
    case ENETDOWN:
    case ENETUNREACH:
    case ENETRESET:
    case ECONNABORTED:
    case ETIMEDOUT:
    case EHOSTDOWN:
    case ECONNRESET:
      nse->status = NSE_STATUS_ERROR;
      nse->errnum = optval;
      break;
    default:
      snprintf(buf, sizeof(buf), "Strange connect error from %s (%d)", inet_ntop_ez(&iod->peer, iod->peerlen), optval);
      perror(buf);
      assert(0); /* I'd like for someone to report it */
      break;
    }


    /* Now special code for the SSL case where the TCP connection was successful. */
    if (nse->type == NSE_TYPE_CONNECT_SSL && 
	nse->status == NSE_STATUS_SUCCESS) {
#if HAVE_OPENSSL
      sslnfo = Nsock_SSLGetInfo();
      iod->ssl = SSL_new(sslnfo->ctx);
      if (!iod->ssl)
	fatal("SSL_new failed: %s", ERR_error_string(ERR_get_error(), NULL));

      /* Associate our new SSL with the connected socket.  It will inherit
	 the non-blocking nature of the sd */
      if (SSL_set_fd(iod->ssl, iod->sd) != 1) {
	fatal("SSL_set_fd failed: %s", ERR_error_string(ERR_get_error(), NULL));
      }
      /* Event not done -- need to do SSL connect below */
      nse->sslinfo.ssl_desire = SSL_ERROR_WANT_CONNECT;
#endif
    } else {
      /* This is not an SSL connect (in which case we are always done), or
	 the TCP connect() underlying the SSL failed (in which case we are also
	 done */
      nse->event_done = 1;
    }
  } else {
    assert(0); /* Currently we only know about TIMEOUT and SUCCESS callbacks */
  }

  /* Clear the socket descriptors from all the lists -- we might
     put it back on some of them in the SSL case */
  if (iod->sd != -1) {  
    FD_CLR(iod->sd, &ms->mioi.fds_master_r);
    FD_CLR(iod->sd, &ms->mioi.fds_master_w);
    FD_CLR(iod->sd, &ms->mioi.fds_master_x);
    FD_CLR(iod->sd, &ms->mioi.fds_results_r);
    FD_CLR(iod->sd, &ms->mioi.fds_results_w);
    FD_CLR(iod->sd, &ms->mioi.fds_results_x);
    
    if (ms->mioi.max_sd == iod->sd)
      ms->mioi.max_sd--;
  }

#if HAVE_OPENSSL
  if (nse->type == NSE_TYPE_CONNECT_SSL && !nse->event_done) {
    /* Lets now start/continue/finish the connect! */
    if (iod->ssl_session) {    
     rc = SSL_set_session(iod->ssl, iod->ssl_session);
     if (rc == 0) { printf("Uh-oh: SSL_set_session() failed - please tell Fyodor\n"); }
     iod->ssl_session = NULL; /* No need for this any more */
    }
    rc = SSL_connect(iod->ssl);
    /* printf("DBG: SSL_connect()=%d", rc); */
    if (rc == 1) {
      /* Woop!  Connect is done! */
      nse->event_done = 1;
      nse->status = NSE_STATUS_SUCCESS;
    } else {
      sslerr = SSL_get_error(iod->ssl, rc);
      if (rc == -1 && sslerr == SSL_ERROR_WANT_READ) {
	nse->sslinfo.ssl_desire = sslerr;
	FD_SET(iod->sd, &ms->mioi.fds_master_r);
	ms->mioi.max_sd = MAX(ms->mioi.max_sd, iod->sd);
      } else if  (rc == -1 && sslerr == SSL_ERROR_WANT_WRITE) {
	nse->sslinfo.ssl_desire = sslerr;
	FD_SET(iod->sd, &ms->mioi.fds_master_w);
	ms->mioi.max_sd = MAX(ms->mioi.max_sd, iod->sd);
      } else {
	/* Unexpected error */
	nse->event_done = 1;
	nse->status = NSE_STATUS_ERROR;
	nse->errnum = EIO;
      }
    }
  }
#endif

  return;
}

void handle_write_result(mspool *ms, msevent *nse, 
			       enum nse_status status)  
{
  int bytesleft;
  char *str;
  int res;
  int err;
  msiod *iod = nse->iod;

  if (status == NSE_STATUS_TIMEOUT || status == NSE_STATUS_CANCELLED) {
    nse->event_done = 1;
    nse->status = status;
  } else if (status == NSE_STATUS_SUCCESS) {
    str = FILESPACE_STR(&nse->iobuf) + nse->writeinfo.written_so_far;
    bytesleft = FILESPACE_LENGTH(&nse->iobuf) - nse->writeinfo.written_so_far;
    assert(bytesleft > 0);
#if HAVE_OPENSSL
    if (iod->ssl)
      res = SSL_write(iod->ssl, str, bytesleft);
    else
#endif
      res = send(nse->iod->sd, str, bytesleft, 0);
    if (res == bytesleft) {
      nse->event_done = 1;
      nse->status = NSE_STATUS_SUCCESS;
    } else if (res >= 0) {
      nse->writeinfo.written_so_far += res;      
    } else {
      assert(res == -1);
      if (iod->ssl) {
#if HAVE_OPENSSL
	err = SSL_get_error(iod->ssl, res);
	if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ) {
	  nse->sslinfo.ssl_desire = err;
	} else {
	  /* Unexpected error */
	  nse->event_done = 1;
	  nse->status = NSE_STATUS_ERROR;
	  nse->errnum = EIO;
	}
#endif
      } else {
	err = socket_errno();
	if (err != EINTR && err != EAGAIN 
#ifndef WIN32
	    && err != EBUSY
#endif
	    ) {
	  nse->event_done = 1;
	  nse->status = NSE_STATUS_ERROR;
	  nse->errnum = err;
	}
      }
    }
  }

  if (nse->event_done && nse->iod->sd != -1) {
    if (!iod->ssl) {
      FD_CLR(nse->iod->sd, &ms->mioi.fds_master_w);
      FD_CLR(nse->iod->sd, &ms->mioi.fds_results_w);
    } else if (iod->events_pending <= 1) {
      FD_CLR(iod->sd, &ms->mioi.fds_master_r);
      FD_CLR(iod->sd, &ms->mioi.fds_results_r);
      FD_CLR(iod->sd, &ms->mioi.fds_master_w);
      FD_CLR(iod->sd, &ms->mioi.fds_results_w);
    }

    /* Note -- I only want to decrement IOD if there are no other
       events hinging on it.  For example, there could be a READ and
       WRITE outstanding at once */
    if (nse->iod->events_pending <= 1 && ms->mioi.max_sd == nse->iod->sd)
      ms->mioi.max_sd--;
  }

  return;
}

void handle_timer_result(mspool *ms, msevent *nse, 
			       enum nse_status status)  
{

  /* Ooh this is a hard job :) */

  nse->event_done = 1;
  nse->status = status;

  return;
}

/* Returns -1 if an error, otherwise the number of newly written bytes */
static int do_actual_read(mspool *ms, msevent *nse) {
  char buf[8192];
  int buflen = 0;
  msiod *iod = nse->iod;
  int err = 0;
  int max_chunk = NSOCK_READ_CHUNK_SIZE;
  int startlen = FILESPACE_LENGTH(&nse->iobuf);

  if (nse->readinfo.read_type == NSOCK_READBYTES && nse->readinfo.num > max_chunk)
    max_chunk = nse->readinfo.num;

  if (!iod->ssl) {
    /* Traditional read() - no SSL - using recv() because that works better on Windows */
    do {
      buflen = recv(iod->sd, buf, sizeof(buf), 0);
      if (buflen == -1) err = socket_errno();
      if (buflen > 0) {
	if (fscat(&nse->iobuf, buf, buflen) == -1) {
	  nse->event_done = 1;
	  nse->status = NSE_STATUS_ERROR;
	  nse->errnum = ENOMEM;
	  return -1;
	}

	// Sometimes a service just spews and spews data.  So we return
	// after a somewhat large amount to avoid monopolizing resources
	// and avoid DOS attacks.
	if (FILESPACE_LENGTH(&nse->iobuf) > max_chunk)
	  return FILESPACE_LENGTH(&nse->iobuf) - startlen;
	
	// No good reason to read again if we we were successful in the read but
	// didn't fill up the buffer.  I'll insist on it being TCP too, because I
	// think UDP might only give me one packet worth at a time (I dunno ...).
	if (buflen > 0 && buflen < sizeof(buf) && iod->lastproto == IPPROTO_TCP)
	  return FILESPACE_LENGTH(&nse->iobuf) - startlen;
      }
    } while (buflen > 0 || (buflen == -1 && err == EINTR));

    if (buflen == -1) {
      if (err != EINTR && err != EAGAIN) {
	nse->event_done = 1;
	nse->status = NSE_STATUS_ERROR;
	nse->errnum = err;
	return -1;
      }
    }
  } else {
#if HAVE_OPENSSL
    /* OpenSSL read */
    while ((buflen = SSL_read(iod->ssl, buf, sizeof(buf))) > 0) {

      if (fscat(&nse->iobuf, buf, buflen) == -1) {
	nse->event_done = 1;
	nse->status = NSE_STATUS_ERROR;
	nse->errnum = ENOMEM;
	return -1;
      }
      
      // Sometimes a service just spews and spews data.  So we return
      // after a somewhat large amount to avoid monopolizing resources
      // and avoid DOS attacks.
      if (FILESPACE_LENGTH(&nse->iobuf) > NSOCK_READ_CHUNK_SIZE)
	return FILESPACE_LENGTH(&nse->iobuf) - startlen;
    }

    if (buflen == -1) {
      err = SSL_get_error(iod->ssl, buflen);
      if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ) {
	nse->sslinfo.ssl_desire = err;
      } else {
	/* Unexpected error */
	nse->event_done = 1;
	nse->status = NSE_STATUS_ERROR;
	nse->errnum = EIO;
	if (ms->tracelevel > 2)
	  nsock_trace(ms, "SSL_read() failed for reason %s on NSI %li", 
		      ERR_reason_error_string(err), iod->id);
	return -1;
      }
    }
#endif /* HAVE_OPENSSL */
  }
 
  if (buflen == 0) {
    nse->event_done = 1;
    nse->eof = 1;
    if (FILESPACE_LENGTH(&nse->iobuf) > 0) {
      nse->status = NSE_STATUS_SUCCESS;
      return FILESPACE_LENGTH(&nse->iobuf) - startlen;
    } else {    
      nse->status = NSE_STATUS_EOF;
      return 0;
    }
  }

  return FILESPACE_LENGTH(&nse->iobuf) - startlen;
}


void handle_read_result(mspool *ms, msevent *nse, 
			       enum nse_status status)  
{

  unsigned int count;
  char *str;
  int rc, len;
  msiod *iod = nse->iod;

  if (status == NSE_STATUS_TIMEOUT) {
    nse->event_done = 1;
    if (FILESPACE_LENGTH(&nse->iobuf) > 0) {
      nse->status = NSE_STATUS_SUCCESS;
    } else {    
      nse->status = NSE_STATUS_TIMEOUT;
    }
  } else if (status == NSE_STATUS_CANCELLED) {
    nse->status = status;
    nse->event_done = 1;
  } else if (status == NSE_STATUS_SUCCESS) {
    rc = do_actual_read(ms, nse);
    /* printf("DBG: Just read %d new bytes%s.\n", rc, iod->ssl? "( SSL!)" : ""); */
    if (rc > 0) {
      /* We decide whether we have read enough to return */      
      switch(nse->readinfo.read_type) {
      case NSOCK_READ:
	nse->status = NSE_STATUS_SUCCESS;
	nse->event_done = 1;
	break;
      case NSOCK_READBYTES:
	if (FILESPACE_LENGTH(&nse->iobuf) >= nse->readinfo.num) {
	  nse->status = NSE_STATUS_SUCCESS;
	  nse->event_done = 1;
	} 
	/* else we are not done */
	break;
      case NSOCK_READLINES:
	/* Lets count the number of lines we have ... */
	count = 0;
	len = FILESPACE_LENGTH(&nse->iobuf) -1;
	str = FILESPACE_STR(&nse->iobuf);
	for(count=0; len >= 0; len--) {
	  if (str[len] == '\n') {
	    count++;
	    if ((int) count >= nse->readinfo.num)
	      break;
	  }
	}
	if ((int) count >= nse->readinfo.num) {
	  nse->event_done = 1;
	  nse->status = NSE_STATUS_SUCCESS;
	} 
	/* Else we are not done */
	break;
      default:
	assert(0);
	break; /* unreached */
      }
    } 
  } else {
    assert(0); /* Currently we only know about TIMEOUT, CANCELLED, and SUCCESS callbacks */
  }
  
  /* If we asked for an event dispatch, we are done reading on the socket so 
     we can take it off the descriptor list ... */
  if (nse->event_done && iod->sd >= 0) {
    /* Exception: If this is an SSL socket and there is another
       pending event (such as a write), it might actually be waiting
       on a read so we can't clear in that case */
    if (!iod->ssl) {    
      FD_CLR(iod->sd, &ms->mioi.fds_master_r);
      FD_CLR(iod->sd, &ms->mioi.fds_results_r);
    } else if (iod->events_pending <= 1) {
      FD_CLR(iod->sd, &ms->mioi.fds_master_r);
      FD_CLR(iod->sd, &ms->mioi.fds_results_r);
      FD_CLR(iod->sd, &ms->mioi.fds_master_w);
      FD_CLR(iod->sd, &ms->mioi.fds_results_w);
    }

    // Note -- I only want to decrement IOD if there are no other events hinging on it. 
    // For example, there could be a READ and WRITE outstanding at once
    if (iod->events_pending <= 1 && ms->mioi.max_sd == iod->sd)
      ms->mioi.max_sd--;
  }

#if HAVE_OPENSSL
  /* For SSL the type of listening we have to do varies.  I can't
     clear the other type due to the possibility of a pending event
     needing it */
  if (iod->ssl && nse->event_done == 0) {  
    if (nse->sslinfo.ssl_desire == SSL_ERROR_WANT_READ) {
      FD_SET(iod->sd, &ms->mioi.fds_master_r);
      ms->mioi.max_sd = MAX(ms->mioi.max_sd, iod->sd);
    } else if (nse->sslinfo.ssl_desire == SSL_ERROR_WANT_WRITE) {
      FD_SET(iod->sd, &ms->mioi.fds_master_w);
      ms->mioi.max_sd = MAX(ms->mioi.max_sd, iod->sd);
    }
  }
#endif /* HAVE_OPENSSL */
 
  return;
}

  /* Iterate through all the event lists (such as 
     connect_events, read_events, timer_events, etc) and take action
     for those that have completed (due to timeout, i/o, etc) */

static void iterate_through_event_lists(mspool *nsp) {
    gh_list_elem *current, *next;
    msevent *nse;
    int match_r = 0, match_w = 0;
#if HAVE_OPENSSL
    int desire_r = 0, desire_w = 0;
#endif
    gh_list *event_lists[] = { &nsp->evl.connect_events,
                               &nsp->evl.read_events,
                               &nsp->evl.write_events,
                               &nsp->evl.timer_events,
                               0
                             };
    int current_list_idx;
    nsp->evl.next_ev.tv_sec = 0; /* Clear it -- We will find the next
				   event as we go through the list */

  /* We keep the events seperate because we want to handle them in the 
     order: connect => read => write => timer for several reasons:
     1) Makes sure we have gone through all the net i/o events before 
        a timer expires (would be a shame to timeout after the data was 
        available but before we delivered the events
     2) The connect() results often lead to a read or write that can be 
        processed in the same cycle.  In the same way, read() often 
	leads to write().
  */

    /* foreach list */
    for(current_list_idx = 0; event_lists[current_list_idx] != NULL;
	current_list_idx++) {

    /* foreach element in the list */
      for(current = GH_LIST_FIRST_ELEM(event_lists[current_list_idx]);
	  current != NULL; current = next) {

	nse = (msevent *) GH_LIST_ELEM_DATA(current);
	if ( ! nse->event_done) {	
	  switch(nse->type) {
	  case NSE_TYPE_CONNECT:
	  case NSE_TYPE_CONNECT_SSL:
	    if (FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_r) ||
		FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_w) ||
		FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_x)) {
	      handle_connect_result(nsp, nse, NSE_STATUS_SUCCESS);
	    } 
	    if (!nse->event_done && nse->timeout.tv_sec &&
		TIMEVAL_MSEC_SUBTRACT(nse->timeout, nsock_tod) <= 0) {
	      handle_connect_result(nsp, nse, NSE_STATUS_TIMEOUT);
	    }
	    break;
	    
	  case NSE_TYPE_READ:
	    match_r = FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_r);
	    match_w = FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_w);
#if HAVE_OPENSSL
	    desire_r = nse->sslinfo.ssl_desire == SSL_ERROR_WANT_READ;
	    desire_w = nse->sslinfo.ssl_desire == SSL_ERROR_WANT_WRITE;
	    if (nse->iod->ssl && ((desire_r && match_r) || (desire_w && match_w)))
              handle_read_result(nsp, nse, NSE_STATUS_SUCCESS);
	    else
#endif
	    if (!nse->iod->ssl && match_r)
	      handle_read_result(nsp, nse, NSE_STATUS_SUCCESS);
	    
	    if (!nse->event_done && nse->timeout.tv_sec &&
		TIMEVAL_MSEC_SUBTRACT(nse->timeout, nsock_tod) <= 0) {
	      handle_read_result(nsp, nse, NSE_STATUS_TIMEOUT);
	    }
	    break;
	    
	  case NSE_TYPE_WRITE:
	    match_r = FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_r);
	    match_w = FD_ISSET(nse->iod->sd, &nsp->mioi.fds_results_w);
#if HAVE_OPENSSL
	    desire_r = nse->sslinfo.ssl_desire == SSL_ERROR_WANT_READ;
	    desire_w = nse->sslinfo.ssl_desire == SSL_ERROR_WANT_WRITE;
	    if (nse->iod->ssl && ((desire_r && match_r) || 
				  (desire_w && match_w)))
	      handle_write_result(nsp, nse, NSE_STATUS_SUCCESS);
	    else
#endif
	    if (!nse->iod->ssl && match_w)
	      handle_write_result(nsp, nse, NSE_STATUS_SUCCESS);

	    if (!nse->event_done && nse->timeout.tv_sec &&
		TIMEVAL_MSEC_SUBTRACT(nse->timeout, nsock_tod) <= 0) {
	      handle_write_result(nsp, nse, NSE_STATUS_TIMEOUT);
	    }
	    break;
	    
	  case NSE_TYPE_TIMER:
	    if (nse->timeout.tv_sec && 
		TIMEVAL_MSEC_SUBTRACT(nse->timeout, nsock_tod) <= 0) {
	      handle_timer_result(nsp, nse, NSE_STATUS_SUCCESS);
	    }
	    break;
	    
	  default:
	    fatal("Event has unknown type (%d)", nse->type);
	    break; /* unreached */
	  }
	}

	if (nse->event_done) {
	  /* WooHoo!  The event is ready to be sent */
	  msevent_dispatch_and_delete(nsp, nse, 1);
	  next = GH_LIST_ELEM_NEXT(current);
	  gh_list_remove_elem(event_lists[current_list_idx], current);
	} else {
	  next = GH_LIST_ELEM_NEXT(current);
	  /* Is this event the next-to-timeout? */
	  if (nse->timeout.tv_sec != 0) {
	    if (nsp->evl.next_ev.tv_sec == 0)
	      nsp->evl.next_ev = nse->timeout;
	    else if (TIMEVAL_MSEC_SUBTRACT(nsp->evl.next_ev, nse->timeout) > 0)
	      nsp->evl.next_ev = nse->timeout;
	  }
	}
      }
    }
}

/* Here is the all important looping function that tells the event
   engine to start up and begin processing events.  It will continue
   until all events have been delivered (including new ones started
   from event handlers), or the msec_timeout is reached, or a major
   error has occured.  Use -1 if you don't want to set a maximum time
   for it to run.  A timeout of 0 will return after 1 non-blocking
   loop.  The nsock loop can be restarted again after it returns.  For
   example you could do a series of 15 second runs, allowing you to do
   other stuff between them */
enum nsock_loopstatus nsock_loop(nsock_pool nsp, int msec_timeout) {
mspool *ms = (mspool *) nsp;
struct timeval loop_timeout;
int msecs_left;
unsigned long loopnum = 0;
enum nsock_loopstatus quitstatus = NSOCK_LOOP_ERROR;

gettimeofday(&nsock_tod, NULL);

 if (msec_timeout < -1) {
   ms->errnum = EINVAL;
   return NSOCK_LOOP_ERROR;
 }
 TIMEVAL_MSEC_ADD(loop_timeout, nsock_tod, msec_timeout);
 msecs_left = msec_timeout;
 
 if (ms->tracelevel > 1) {
   if (msec_timeout >= 0) {
     nsock_trace(ms, "nsock_loop() started (timeout=%dms). %d events pending", msec_timeout, 
		 ms->evl.events_pending);
   } else {
     nsock_trace(ms, "nsock_loop() started (no timeout). %d events pending", ms->evl.events_pending);
   }
 }

while(1) {

  if (ms->evl.events_pending == 0) {
    /* if no events at all are pending, then none can be created until
       we quit nsock_loop() -- so we do that now. */
    quitstatus = NSOCK_LOOP_NOEVENTS;
    break;
  }
 
  if (msec_timeout > 0) {
    msecs_left = MAX(0, TIMEVAL_MSEC_SUBTRACT(loop_timeout, nsock_tod));
    if (msecs_left == 0 && loopnum > 0) {
      quitstatus = NSOCK_LOOP_TIMEOUT;
      break;
    }
  }

  if (wait_for_events(ms, msecs_left) == -1) {  
    quitstatus = NSOCK_LOOP_ERROR;
    break;
  }
  
  /* Now we go through the event lists, doing callbacks for those which
     have completed */
  iterate_through_event_lists(ms);

  gettimeofday(&nsock_tod, NULL); /* we do this at end because there is one
			       at beginning of function */
  loopnum++;
}

 return quitstatus;
}

/* Grab the latest time as recorded by the nsock library, which does
   so at least once per event loop (in main_loop).  Not only does this
   function (generally) avoid a system call, but in many circumstances
   it is better to use nsock's time rather than the system time.  If
   nsock has never obtained the time when you call it, it will do so
   before returning */
const struct timeval *nsock_gettimeofday() {
  if (nsock_tod.tv_sec == 0) 
    gettimeofday(&nsock_tod, NULL);

  return &nsock_tod;
}


/* Adds an event to the appropriate nsp event list, handles housekeeping
   such as adjusting the descriptor select/poll lists, registering the
   timeout value, etc. */
void nsp_add_event(mspool *nsp, msevent *nse) {

  /* First lets do the event-type independant stuff -- starting with
     timeouts */
  if (nse->event_done) { 
    nsp->evl.next_ev = nsock_tod;
  } else {
    if (nse->timeout.tv_sec != 0) {
      if (nsp->evl.next_ev.tv_sec == 0) {
	nsp->evl.next_ev = nse->timeout;
      } else if (TIMEVAL_MSEC_SUBTRACT(nsp->evl.next_ev, nse->timeout) > 0) {
	nsp->evl.next_ev = nse->timeout;
      }
    }
  }

  nsp->evl.events_pending++;

  /* Now we do the event type specific actions */
  switch(nse->type) {
  case NSE_TYPE_CONNECT:
  case NSE_TYPE_CONNECT_SSL:
    if (!nse->event_done) {
      FD_SET( nse->iod->sd, &nsp->mioi.fds_master_r);
      FD_SET( nse->iod->sd, &nsp->mioi.fds_master_w);
      FD_SET( nse->iod->sd, &nsp->mioi.fds_master_x);
      nsp->mioi.max_sd = MAX(nsp->mioi.max_sd, nse->iod->sd);
    }
    gh_list_prepend(&nsp->evl.connect_events, nse);
    break;

  case NSE_TYPE_READ:
    if (!nse->event_done) {
      FD_SET( nse->iod->sd, &nsp->mioi.fds_master_r);
      nsp->mioi.max_sd = MAX(nsp->mioi.max_sd, nse->iod->sd);
#if HAVE_OPENSSL
      if (nse->iod->ssl) nse->sslinfo.ssl_desire = SSL_ERROR_WANT_READ;
#endif
    }
    gh_list_prepend(&nsp->evl.read_events, nse);
    break;

  case NSE_TYPE_WRITE:
    if (!nse->event_done) {
      FD_SET( nse->iod->sd, &nsp->mioi.fds_master_w);
      nsp->mioi.max_sd = MAX(nsp->mioi.max_sd, nse->iod->sd);
#if HAVE_OPENSSL
      if (nse->iod->ssl) nse->sslinfo.ssl_desire = SSL_ERROR_WANT_WRITE;
#endif
    }
    gh_list_prepend(&nsp->evl.write_events, nse);
    break;

  case NSE_TYPE_TIMER:
    gh_list_prepend(&nsp->evl.timer_events, nse);
    break;

  default:
    assert(0);
    break; /* unreached */

  }
}


void nsock_trace(mspool *ms, char *fmt, ...) {
va_list  ap;
int elapsedTimeMS;

elapsedTimeMS = TIMEVAL_MSEC_SUBTRACT(nsock_tod, ms->tracebasetime);
va_start(ap, fmt);
fflush(stdout);
printf("NSOCK (%.4fs) ", elapsedTimeMS / 1000.0);
vfprintf(stdout, fmt, ap);
printf("\n"); 
va_end(ap);
return;
}

/* An event has been completed and the handler is about to be called.  This function
   writes out tracing data about the event if neccessary */
void nsock_trace_handler_callback(mspool *ms, msevent *nse) {
  msiod *nsi;
  char *str;
  int strlength = 0;
  char displaystr[256];
  char errstr[256];
  
  if (ms->tracelevel == 0)
    return;

  nsi = nse->iod;

  if (nse->status == NSE_STATUS_ERROR) {
    snprintf(errstr, sizeof(errstr), "[%s (%d)] ", strerror(nse->errnum), nse->errnum);
  } else errstr[0] = '\0';

  // Some types have special tracing treatment
  switch(nse->type) {
  case NSE_TYPE_CONNECT:
  case NSE_TYPE_CONNECT_SSL:
    nsock_trace(ms, "Callback: %s %s %sfor EID %li [%s:%hi]",  
		nse_type2str(nse->type), nse_status2str(nse->status), errstr, 
		nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), nsi_peerport(nsi));
    break;

  case NSE_TYPE_READ:
    if (nse->status != NSE_STATUS_SUCCESS) {
      if (nsi->peerlen > 0)
	 nsock_trace(ms, "Callback: %s %s %sfor EID %li [%s:%hi]", 
		  nse_type2str(nse->type), nse_status2str(nse->status), 
		  errstr, nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), 
		  nsi_peerport(nsi));
      else
	 nsock_trace(ms, "Callback: %s %s %sfor EID %li (peer unspecified)",
		     nse_type2str(nse->type), nse_status2str(nse->status),
		     errstr, nse->id);
    } else {    
      str = nse_readbuf(nse, &strlength);
      if (ms->tracelevel > 1 && strlength < 80) {
	memcpy(displaystr, ": ", 2);
	memcpy(displaystr + 2, str, strlength);
	displaystr[2 + strlength] = '\0';
	replacenonprintable(displaystr + 2, strlength, '.');
      } else displaystr[0] = '\0';
      
      if (nsi->peerlen > 0)
	nsock_trace(ms, "Callback: %s %s for EID %li [%s:%hi] %s(%d bytes)%s", 
      		    nse_type2str(nse->type), nse_status2str(nse->status), 
      		    nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), 
      		    nsi_peerport(nsi), nse_eof(nse)? "[EOF]" : "", strlength, 
      		    displaystr);
      else
	nsock_trace(ms, "Callback %s %s for EID %li (peer unspecified) %s(%d bytes)%s",
		    nse_type2str(nse->type), nse_status2str(nse->status),
		    nse->id, nse_eof(nse)? "[EOF]" : "", strlength, displaystr);
    }
    break;

  case NSE_TYPE_WRITE:
    nsock_trace(ms, "Callback: %s %s %sfor EID %li [%s:%hi]", 
		nse_type2str(nse->type), nse_status2str(nse->status), errstr, 
		nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), 
		  nsi_peerport(nsi));
    break;
  case NSE_TYPE_TIMER:
    nsock_trace(ms, "Callback: %s %s %sfor EID %li", 
		nse_type2str(nse->type), nse_status2str(nse->status), errstr, 
		nse->id);
    break;
  }

}
