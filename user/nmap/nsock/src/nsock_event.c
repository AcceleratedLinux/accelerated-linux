/***************************************************************************
 * nsock_event.c -- Functions dealing with nsock_events (and their         *
 * msevent internal representation.  An event is created when you do       *
 * various calls (for reading, writing, connecting, timers, etc) and is    *
 * provided back to you in the callback when the call completes or         *
 * fails. It is automatically destroyed after the callback returns         *
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

/* $Id: nsock_event.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "nsock_internal.h"
#include "gh_list.h"

#include <string.h>

extern struct timeval nsock_tod;

/* Find the type of an event that spawned a callback */
enum nse_type nse_type(nsock_event nse) {
  msevent *me = (msevent *)nse;
  return me->type;
}

enum nse_status nse_status(nsock_event nse) {
  msevent *me = (msevent *)nse;
  return me->status;
}

int nse_eof(nsock_event nse) {
  msevent *me = (msevent *)nse;
  return me->eof;
}

/* Obtains the nsock_iod (see below) associated with the event.  Note that 
   some events (such as timers) don't have an nsock_iod associated with them
*/
nsock_iod nse_iod(nsock_event ms_event) {
  msevent *nse = (msevent *) ms_event;
  return (nsock_iod) nse->iod;
}

/* This next function returns the errno style error code -- which is only
   valid if the status is NSE_STATUS_ERROR */
int nse_errorcode(nsock_event nse) {
  msevent *me = (msevent *)nse;
  return me->errnum;
}

/* Every event has an ID which will be unique throughout the program's execution unless you use (literally) billions of them */
nsock_event_id nse_id(nsock_event nse) {
  msevent *me = (msevent *)nse;
  return me->id;
}

/* If you did a read request, and the result was STATUS_SUCCESS, this 
   function provides the buffer that was read in as well as the number
   of chars read.  The buffer should not be modified or free'd */
char *nse_readbuf(nsock_event nse, int *nbytes) {
  msevent *me = (msevent *)nse;

  if (nbytes) {
    *nbytes = FILESPACE_LENGTH(&(me->iobuf));
  }
  return FILESPACE_STR(&(me->iobuf));
}


/* Cancel an event (such as a timer or read request).  If notify is
 nonzero, the requester will be sent an event CANCELLED status back to
 the given handler.  But in some cases there is no need to do this
 (like if the function deleting it is the one which created it), in
 which case 0 can be passed to skip the step.  This function returns
 zero if the event is not found, nonzero otherwise */
int nsock_event_cancel(nsock_pool ms_pool, nsock_event_id id, int notify ) {
  mspool *nsp = (mspool *) ms_pool;
  enum nse_type type = get_event_id_type(id);
  gh_list *event_list = NULL;
  gh_list_elem *current, *next;
  msevent *nse = NULL;

  assert(nsp);

 if (nsp->tracelevel > 0) {
    nsock_trace(nsp, "Event #%li (type %s) cancelled", id, nse_type2str(type));
  }

  /* First we figure out what list it is in */
  switch(type) {
  case NSE_TYPE_CONNECT:
    event_list = &nsp->evl.connect_events;
    break;
  case NSE_TYPE_READ: 
    event_list = &nsp->evl.read_events;
    break;
  case NSE_TYPE_WRITE:
    event_list = &nsp->evl.write_events;
    break;
  case NSE_TYPE_TIMER:
    event_list = &nsp->evl.timer_events;
    break;
  default:
    fatal("Bogus event type in nsock_event_cancel"); break;
  }

  /* Now we try to find the event in the list */
  for(current = GH_LIST_FIRST_ELEM(event_list);	current != NULL; 
      current = next) {
    next = GH_LIST_ELEM_NEXT(current);
    nse = (msevent *) GH_LIST_ELEM_DATA(current);
    if (nse->id == id)
      break;
  }

  if (current == NULL)
    return 0;

  return msevent_cancel(nsp, nse, event_list, current, notify);
}

/* An inernal function for cancelling an event when you already have a
   pointer to the msevent (use nsock_event_cancel if you just have an
   ID).  The event_list passed in should correspond to the type of the
   event.  For example, with NSE_TYPE_READ, you would pass in
   &nsp->evl.read_events;.  elem is the list element in event_list which
   holds the event.  Pass a nonzero for notify if you want the
   program owning the event to be notified that it has been cancelled */
int msevent_cancel(mspool *nsp, msevent *nse, gh_list *event_list, gh_list_elem *elem,
		   int notify) {


  if (nse->event_done) {
    /* This event has already been marked for death somewhere else -- it
       will be gone soon (and if we try to kill it now all hell will break
       loose due to reentrancy */
    return 0;
  }

  /* Now that we found the event ... we go through the motions of cleanly
     cancelling it */
  switch(nse->type) {
  case NSE_TYPE_CONNECT:
  case NSE_TYPE_CONNECT_SSL:
    handle_connect_result(nsp, nse, NSE_STATUS_CANCELLED);
    break;
  case NSE_TYPE_READ: 
    handle_read_result(nsp, nse, NSE_STATUS_CANCELLED);
    break;
  case NSE_TYPE_WRITE:
    handle_write_result(nsp, nse, NSE_STATUS_CANCELLED);  
    break;
  case NSE_TYPE_TIMER:
    handle_timer_result(nsp, nse, NSE_STATUS_CANCELLED);  
    break;
  }

  assert(nse->event_done);
  gh_list_remove_elem(event_list, elem);

  msevent_dispatch_and_delete(nsp, nse, notify);

  return 1;
}

/* Adjust various statistics, dispatches the event handler (if notify is
   nonzero) and then deletes the event.  This function does NOT delete
   the event from any lists it might be on (eg nsp->evl.read_list etc.) 
   nse->event_done MUST be true when you call this */
void msevent_dispatch_and_delete(mspool *nsp, msevent *nse, int notify) {
  assert(nsp);
  assert(nse);

  assert(nse->event_done);

  nsp->evl.events_pending--;
  assert(nsp->evl.events_pending >= 0);

  
  if (nse->iod) {
    nse->iod->events_pending--;
    assert(nse->iod->events_pending >= 0);
  }
 
  if (notify) {
    nsock_trace_handler_callback(nsp, nse);
    nse->handler(nsp, nse, nse->userdata);
  }

/* FIXME: We should be updating stats here ... */

  /* Now we clobber the event ... */
  msevent_delete(nsp, nse);
}

/* OK -- the idea is that we want the type included in the rightmost
   two bits and the serial number in the leftmost 30 or 62.  But we
   also want to insure a correct wrap-around in the case of an obscene
   number of event.  One definition of a "correct" wraparound is that
   it goes from the highest number back to one (not zero) because we
   don't want event numbers to ever be zero. */
nsock_event_id get_new_event_id(mspool *ms, enum nse_type type) {
  int type_code = (int) type;
  unsigned long serial = ms->next_event_serial++;
  unsigned long max_serial_allowed;
  int shiftbits;
  assert(type <= 4);
 
  shiftbits = sizeof(nsock_event_id) * 8 - TYPE_CODE_NUM_BITS;
  max_serial_allowed = ( 1 << shiftbits ) - 1;
  if (serial == max_serial_allowed ) {
    /* then the next serial will be one because 0 is forbidden */
    ms->next_event_serial = 1;
  }

  return (serial << TYPE_CODE_NUM_BITS) | type_code;
}

/* Take an event ID and return the type (NSE_TYPE_CONNECT, etc */
enum nse_type get_event_id_type(nsock_event_id event_id) {

  return (enum nse_type) ((event_id & ((1 << TYPE_CODE_NUM_BITS) - 1)));

}


/* Create a new event structure -- must be deleted later with msevent_delete,
 unless it returns NULL (failure).  NULL can be passed in for the
 msiod and the userdata if not available */
msevent *msevent_new(mspool *nsp, enum nse_type type, msiod *msiod, 
		     int timeout_msecs, nsock_ev_handler handler,
		     void *userdata) {

  msevent *nse;

  if (msiod) {
    msiod->events_pending++;
    assert(msiod->state != NSIOD_STATE_DELETED);
  }

  /* First we check if one is available from the free list ... */
  nse = (msevent *) gh_list_pop(&nsp->evl.free_events);
  if (!nse) nse = (msevent *) safe_malloc(sizeof(msevent));
  memset(nse, 0, sizeof(*nse));
  nse->id = get_new_event_id(nsp, type);
  nse->type = type;
  nse->status = NSE_STATUS_NONE;
#if HAVE_OPENSSL
  nse->sslinfo.ssl_desire = SSL_ERROR_NONE;
#endif
  if (type == NSE_TYPE_READ || type ==  NSE_TYPE_WRITE) {  
    filespace_init(&(nse->iobuf), 1024);
  }
  if (timeout_msecs != -1) {
    assert(timeout_msecs >= 0);
    TIMEVAL_MSEC_ADD(nse->timeout, nsock_tod, timeout_msecs);
  }
  nse->iod = msiod;
  nse->handler = handler;
  nse->userdata = userdata;
  nse->time_created = nsock_tod;
  return nse;
}

/* Free an msevent which was allocated with msevent_new, including all
   internal resources.  Note -- we assume that
   nse->iod->events_pending (if it exists) has ALREADY been
   decremented (done during msevent_dispatch_and_delete) -- so
   remember to do this if you call msevent_delete() directly */
void msevent_delete(mspool *nsp, msevent *nse) {

  /* First free the IOBuf inside it if neccessary */
  if (nse->type == NSE_TYPE_READ || nse->type ==  NSE_TYPE_WRITE) {  
    fs_free(&nse->iobuf);
  }

  /* Now we add the event back into the free pool */
  gh_list_prepend(&nsp->evl.free_events, nse);

}


/* Takes an nse_type (as returned by nse_type() and returns a static
   string name that you can use for printing, etc. */
const char *nse_type2str(enum nse_type type) {
  switch(type) {
  case NSE_TYPE_CONNECT: return "CONNECT";
  case NSE_TYPE_CONNECT_SSL: return "SSL-CONNECT";
  case NSE_TYPE_READ: return "READ";
  case NSE_TYPE_WRITE: return "WRITE";
  case NSE_TYPE_TIMER: return "TIMER";
  default:
    return "UNKNOWN!";
  }

  return "WTF????"; /* Unreached */
}

/* Takes an nse_status (as returned by nse_status() and returns a static
   string name that you can use for printing, etc. */
const char *nse_status2str(enum nse_status status) {
  switch(status) {
  case NSE_STATUS_NONE: return "NONE";
  case NSE_STATUS_SUCCESS: return "SUCCESS";
  case NSE_STATUS_ERROR: return "ERROR";
  case NSE_STATUS_TIMEOUT: return "TIMEOUT";
  case NSE_STATUS_CANCELLED: return "CANCELLED";
  case NSE_STATUS_KILL: return "KILL";
  case NSE_STATUS_EOF: return "EOF";
  default:
    return "UNKNOWN!";
  }

  return "WTF????"; /* Unreached */

}







