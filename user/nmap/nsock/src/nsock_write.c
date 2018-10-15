
/***************************************************************************
 * nsock_write.c -- This contains the functions relating to writing to     *
 * sockets using the nsock parallel socket event library                   *
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

/* $Id: nsock_write.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "nsock.h"
#include "nsock_internal.h"

#include <nbase.h>
#include <stdarg.h>
#include <errno.h>

/* Write some data to the socket.  If the write is not COMPLETED within timeout_msecs , NSE_STATUS_TIMEOUT will be returned.  If you are supplying NUL-terminated data, you can optionally pass -1 for datalen and nsock_write will figure out the length itself */
nsock_event_id  nsock_write(nsock_pool ms_pool, nsock_iod ms_iod, 
		      nsock_ev_handler handler, int timeout_msecs, 
		      void *userdata, const char *data, int datalen) {
  mspool *nsp = (mspool *) ms_pool;
  msiod *nsi = (msiod *) ms_iod;
  msevent *nse;
  char displaystr[256];

  nse = msevent_new(nsp, NSE_TYPE_WRITE, nsi, timeout_msecs, handler,
		    userdata);
  assert(nse);

  if (nsp->tracelevel > 0) {
    if (nsp->tracelevel > 1 && datalen < 80) {
      memcpy(displaystr, ": ", 2);
      memcpy(displaystr + 2, data, datalen);
      displaystr[2 + datalen] = '\0';
      replacenonprintable(displaystr + 2, datalen, '.');
    } else displaystr[0] = '\0';
    if (nsi->peerlen > 0)
      nsock_trace(nsp, "Write request for %d bytes to IOD #%li EID %li [%s:%hi]%s", datalen, nsi->id, 
		  nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), nsi_peerport(nsi), displaystr);
    else 
      nsock_trace(nsp, "Write request for %d bytes to IOD #%li EID %li (peer unspecified)%s", datalen, 
		  nsi->id, nse->id, displaystr);
  }

  if (datalen < 0)
    datalen = (int) strlen(data);

  fscat(&nse->iobuf, data, datalen);

  nsp_add_event(nsp, nse);
  
  return nse->id;
}

/* Same as nsock_write except you can use a printf-style format and you can only use this for ASCII strings */
nsock_event_id nsock_printf(nsock_pool ms_pool, nsock_iod ms_iod, 
		      nsock_ev_handler handler, int timeout_msecs, 
		      void *userdata, char *format, ... ) {

  mspool *nsp = (mspool *) ms_pool;
  msiod *nsi = (msiod *) ms_iod;
  msevent *nse;
  char buf[4096];
  char *buf2 = NULL;
  int res, res2;
  int strlength = 0;
  char displaystr[256];

  va_list ap; 
  va_start(ap,format);

  nse = msevent_new(nsp, NSE_TYPE_WRITE, nsi, timeout_msecs, handler,
		    userdata);
  assert(nse);

  res = vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  if (res != -1) {
    if (res > sizeof(buf)) {
      buf2 = (char * ) safe_malloc(res + 16);
      res2 = vsnprintf(buf2, sizeof(buf), format, ap);
      if (res2 == -1 || res2 > res) {
	free(buf2);
	buf2 = NULL;
      } else strlength = res2;
    } else {
      buf2 = buf;
      strlength = res;
    }
  }

  if (!buf2) {
    nse->event_done = 1;
    nse->status = NSE_STATUS_ERROR;
    nse->errnum = EMSGSIZE;
  } else {
    if (strlength == 0) {
      nse->event_done = 1;
      nse->status = NSE_STATUS_SUCCESS;      
    } else {    
      fscat(&nse->iobuf, buf2, strlength);
    }
  }

  if (nsp->tracelevel > 0) {
    if (nsp->tracelevel > 1 && nse->status != NSE_STATUS_ERROR && strlength < 80) {
      memcpy(displaystr, ": ", 2);
      memcpy(displaystr + 2, buf2, strlength);
      displaystr[2 + strlength] = '\0';
      replacenonprintable(displaystr + 2, strlength, '.');
    } else displaystr[0] = '\0';
    if (nsi->peerlen > 0)
      nsock_trace(nsp, "Write request for %d bytes to IOD #%li EID %li [%s:%hi]%s", strlength, nsi->id, 
		  nse->id, inet_ntop_ez(&nsi->peer, nsi->peerlen), nsi_peerport(nsi), displaystr);
    else 
      nsock_trace(nsp, "Write request for %d bytes to IOD #%li EID %li (peer unspecified)%s", strlength, 
		  nsi->id, nse->id, displaystr);
  }

  if (buf2 != buf) {
    free(buf2);
  }

  nsp_add_event(nsp, nse);

  return nse->id;
}

