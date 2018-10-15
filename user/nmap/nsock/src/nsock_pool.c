
/***************************************************************************
 * nsock_pool.c -- This contains the functions that deal with creating,    *
 * destroying, and otherwise manipulating nsock_pools (and their internal  *
 * mspool representation).  An nsock_pool aggregates and manages events    *
 * and i/o descriptors                                                     *
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

/* $Id: nsock_pool.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "nsock_internal.h"
#include "gh_list.h"
#include "netutils.h"

#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>

extern struct timeval nsock_tod;
unsigned long nsp_next_id = 2;

static int nsocklib_initialized = 0; /* To use this library, the first thing
					they must do is create a pool -- so
					we do the initialization during the 
					first pool creation */

static void nsock_library_initialize(void) {
  int res;

  /* We want to make darn sure the evil SIGPIPE is ignored */
#ifndef WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  /* And we're gonna need sockets -- LOTS of sockets ... */
  res = maximize_fdlimit();
#ifndef WIN32
  assert(res > 7);
#endif
}

/* Every mst has an ID that is unique across the program execution */
unsigned long nsp_getid(nsock_pool nsp) {
  mspool *mt = (mspool *) nsp;
  return mt->id;
}

/* This next function returns the errno style error code -- which is only
   valid if the status is NSOCK_LOOP_ERROR was returned by nsock_loop() */
int nsp_geterrorcode(nsock_pool nsp) {
  mspool *mt = (mspool *) nsp;
  return mt->errnum;
}

/* Sometimes it is useful to store a pointer to information inside
   the NSP so you can retrieve it during a callback. */
void nsp_setud(nsock_pool nsp, void *data) {
  mspool *mt = (mspool *) nsp;
  mt->userdata = data;
}

/* And the define above wouldn't make much sense if we didn't have a way
   to retrieve that data ... */
void *nsp_getud(nsock_pool nsp) {
  mspool *mt = (mspool *) nsp;
  return mt->userdata;
}

/* Sets a trace/debug level.  Zero (the default) turns tracing off,
   while higher numbers are more verbose.  This is generally only used
   for debugging purposes.  Trace logs are printed to stdout.  The
   initial value is set in nsp_new().  A level of 1 or 2 is usually
   sufficient, but 10 will ensure you get everything.  The basetime
   can be NULL to print trace lines with the current time, otherwise
   the difference between the current time and basetime will be used
   (the time program execution starts would be a good candidate) */
void nsp_settrace(nsock_pool nsp, int level, const struct timeval *basetime) {
  mspool *mt = (mspool *) nsp;
  mt->tracelevel = level;
  if (!basetime) 
    memset(&(mt->tracebasetime), 0, sizeof(struct timeval));
  else mt->tracebasetime = *basetime;
}

/* And here is how you create an nsock_pool.  This allocates, initializes,
   and returns an nsock_pool event aggregator.  In the case of error,
   NULL will be returned.  If you do not wish to immediately associate
   any userdata, pass in NULL. */
nsock_pool nsp_new(void *userdata) {
  mspool *nsp;
  nsp = (mspool *) safe_malloc(sizeof(*nsp));
  memset(nsp, 0, sizeof(*nsp));

  gettimeofday(&nsock_tod, NULL);

  nsp->tracelevel = 0;
  if (!nsocklib_initialized) {
    nsock_library_initialize();
    nsocklib_initialized = 1;
  }

  nsp->id = nsp_next_id++;

  /* Now to init the nsock_io_info */
  FD_ZERO(&nsp->mioi.fds_master_r);
  FD_ZERO(&nsp->mioi.fds_master_w);
  FD_ZERO(&nsp->mioi.fds_master_x);
  nsp->mioi.max_sd = -1;
  nsp->mioi.results_left = 0;

  /* Next comes the event list structure */
  gh_list_init(&nsp->evl.connect_events);
  gh_list_init(&nsp->evl.read_events);
  gh_list_init(&nsp->evl.write_events);
  gh_list_init(&nsp->evl.timer_events);
  gh_list_init(&nsp->evl.free_events);
  nsp->evl.next_ev.tv_sec = 0;
  nsp->evl.events_pending = 0;

  nsp->userdata = userdata;

  gh_list_init(&nsp->free_iods);
  gh_list_init(&nsp->active_iods);
  nsp->next_event_serial = 1;

  return (nsock_pool) nsp;
}

/* If nsp_new returned success, you must free the nsp when you are
   done with it to conserve memory (and in some cases, sockets).
   After this call, nsp may no longer be used.  Any pending events are
   sent an NSE_STATUS_KILL callback and all outstanding iods are
   deleted. */
void nsp_delete(nsock_pool ms_pool) {
  mspool *nsp = (mspool *) ms_pool;
   gh_list *event_lists[] = { &nsp->evl.connect_events,
                               &nsp->evl.read_events,
                               &nsp->evl.write_events,
                               &nsp->evl.timer_events,
                               0
                             };
   int current_list_idx;
   msevent *nse;
   msiod *nsi;
   gh_list_elem *current, *next;

   assert(nsp);


  /* First I go through all the events sending NSE_STATUS_KILL */
    /* foreach list */
    for(current_list_idx = 0; event_lists[current_list_idx] != NULL;
	current_list_idx++) {
      while(GH_LIST_COUNT(event_lists[current_list_idx]) > 0) {
	nse = (msevent *) gh_list_pop(event_lists[current_list_idx]);
	nse->status = NSE_STATUS_KILL;
	nsock_trace_handler_callback(nsp, nse);
	nse->handler(nsp, nse, nse->userdata);
	if (nse->iod) {
	  nse->iod->events_pending--;
	  assert(nse->iod->events_pending >= 0);
	}
	msevent_delete(nsp, nse);
      }      
      gh_list_free(event_lists[current_list_idx]);
    }

  /* Then I go through and kill the iods */
    for(current = GH_LIST_FIRST_ELEM(&nsp->active_iods);
	current != NULL; current = next) {
      next = GH_LIST_ELEM_NEXT(current);
      nsi = (msiod *) GH_LIST_ELEM_DATA(current);
      nsi_delete(nsi, NSOCK_PENDING_ERROR);
    }

    /* Now we free all the memory in the free iod list */
    while((nsi = (msiod *) gh_list_pop(&nsp->free_iods))) {
      free(nsi);
    }

    while((nsi = (msiod *) gh_list_pop(&nsp->evl.free_events))) {
      free(nsi);
    }
    gh_list_free(&nsp->evl.free_events);
    gh_list_free(&nsp->active_iods);
    gh_list_free(&nsp->free_iods);

    free(nsp);
}
