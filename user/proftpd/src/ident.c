/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 2001, 2002, 2003 The ProFTPD Project team
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
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/*
 * Ident (RFC1413) protocol support
 * $Id: ident.c,v 1.21 2004/10/30 20:45:52 castaglia Exp $
 */

#include "conf.h"

static int ident_timeout;
static pr_netio_stream_t *nstrm = NULL;

static int ident_timeout_cb(CALLBACK_FRAME) {
  ident_timeout++;

  if (nstrm)
    /* Abort the NetIO stream, which will cause netio_poll (and thus
     * netio_read) to also abort.  This is similar to the way data connects
     * are aborted.
     */
    pr_netio_abort(nstrm);

  return 0;
}

char *pr_ident_lookup(pool *p, conn_t *c) {
  char *ret = "UNKNOWN";
  pool *tmp_pool = NULL;
  conn_t *ident_conn = NULL, *ident_io = NULL;
  char buf[256] = {'\0'}, *tok = NULL, *tmp = NULL;
  int timerno, i = 0;
  int ident_port = pr_inet_getservport(p, "ident", "tcp");

  tmp_pool = make_sub_pool(p);
  ident_timeout = 0;
  nstrm = NULL;

  if (ident_port == -1) {
    destroy_pool(tmp_pool);
    return pstrdup(p, ret);
  }

  /* Set up our timer before going any further. */
  timerno = pr_timer_add(PR_TUNABLE_TIMEOUTIDENT, -1, NULL,
    (callback_t) ident_timeout_cb);
  if (timerno <= 0) {
    destroy_pool(tmp_pool);
    return pstrdup(p, ret);
  }

  ident_conn = pr_inet_create_connection(tmp_pool, NULL, -1, c->local_addr,
    INPORT_ANY, FALSE);
  pr_inet_set_nonblock(tmp_pool, ident_conn);

  if ((i = pr_inet_connect_nowait(tmp_pool, ident_conn, c->remote_addr,
      ident_port)) < 0) {
    pr_timer_remove(timerno, ANY_MODULE);
    pr_inet_close(tmp_pool, ident_conn);
    pr_log_debug(DEBUG6, "ident connection failed: %s", strerror(errno));
    destroy_pool(tmp_pool);
    return pstrdup(p, ret);
  }

  if (!i) {
    /* Not yet connected. */
    nstrm = pr_netio_open(p, PR_NETIO_STRM_OTHR, ident_conn->listen_fd,
      PR_NETIO_IO_RD);
    pr_netio_set_poll_interval(nstrm, 1);

    switch (pr_netio_poll(nstrm)) {

      /* Aborted, timed out */
      case 1:
        if (ident_timeout) {
          pr_timer_remove(timerno, ANY_MODULE);
          pr_netio_close(nstrm);
          pr_inet_close(tmp_pool, ident_conn);
          pr_log_debug(DEBUG6, "ident lookup timed out, returning '%s'", ret);
          destroy_pool(tmp_pool);
          return pstrdup(p, ret);
        }
        break;

      /* Error. */
      case -1:
        pr_timer_remove(timerno, ANY_MODULE);
        pr_netio_close(nstrm);
        pr_inet_close(tmp_pool, ident_conn);
        pr_log_debug(DEBUG6, "ident lookup failed (%s), returning '%s'",
          strerror(errno), ret);
        destroy_pool(tmp_pool);
        return pstrdup(p, ret);

      /* Connected. */
      default:
        ident_conn->mode = CM_OPEN;

        if (pr_inet_get_conn_info(ident_conn, ident_conn->listen_fd) < 0) {
          pr_timer_remove(timerno, ANY_MODULE);
          pr_netio_close(nstrm);
          pr_inet_close(tmp_pool, ident_conn);
          pr_log_debug(DEBUG2, "ident lookup timed out, returning '%s'", ret);
          destroy_pool(tmp_pool);
          return pstrdup(p, ret);
        }
        break;
    }
  }

  if ((ident_io = pr_inet_openrw(tmp_pool, ident_conn, NULL, PR_NETIO_STRM_OTHR,
      -1, -1, -1, FALSE)) == NULL) {
    pr_timer_remove(timerno, ANY_MODULE);
    pr_inet_close(tmp_pool, ident_conn);
    destroy_pool(tmp_pool);
    return pstrdup(p, ret);
  }

  nstrm = ident_io->instrm;

  pr_inet_set_nonblock(tmp_pool, ident_io);
  pr_netio_set_poll_interval(ident_io->instrm, 1);
  pr_netio_set_poll_interval(ident_io->outstrm, 1);

  pr_netio_printf(ident_io->outstrm, "%d, %d\r\n", c->remote_port,
    c->local_port);

  /* If the timer fires while in netio_gets(), netio_gets() will simply return
   * either a partial string, or NULL.  This works because ident_timeout_cb
   * aborts the stream from which we are reading.  netio_set_poll_interval() is
   * used to make sure significant delays don't occur on systems that
   * automatically restart syscalls after the SIGALRM signal.
   */

  if (pr_netio_gets(buf, sizeof(buf), ident_io->instrm)) {
    strip_end(buf, "\r\n");

    tmp = buf;
    tok = get_token(&tmp, ":");
    if (tok && (tok = get_token(&tmp, ":"))) {
      while (*tok && isspace((int) *tok))
        tok++;
      strip_end(tok, " \t");

      if (strcasecmp(tok, "ERROR") == 0) {
        if (tmp) {
          while (*tmp && isspace((int) *tmp))
            tmp++;
	  strip_end(tmp, " \t");
          if (strcasecmp(tmp, "HIDDEN-USER") == 0)
            ret = "HIDDEN-USER";
        }

      } else if (strcasecmp(tok, "USERID") == 0) {
        if (tmp && (tok = get_token(&tmp, ":"))) {
          if (tmp) {
            while (*tmp && isspace((int) *tmp))
              tmp++;
            strip_end(tmp, " \t");
            ret = tmp;
          }
        }
      }
    }
  }

  pr_timer_remove(timerno, ANY_MODULE);
  pr_inet_close(tmp_pool, ident_io);
  pr_inet_close(tmp_pool, ident_conn);
  destroy_pool(tmp_pool);

  return pstrdup(p, ret);
}
