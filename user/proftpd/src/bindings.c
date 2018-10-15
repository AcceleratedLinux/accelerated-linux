/*
 * ProFTPD - FTP server daemon
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Routines to work with ProFTPD bindings
 *
 * $Id: bindings.c,v 1.25 2005/01/06 18:55:03 castaglia Exp $
 */

#include "conf.h"

/* Some convenience macros */
#define PR_CLOSE_NAMEBIND(n, a, p) \
  if ((res = pr_namebind_close((n), (a), (p))) < 0) \
    pr_log_pri(PR_LOG_NOTICE, \
      "%s:%d: notice, unable to close namebind '%s': %s", \
      __FILE__, __LINE__, (n), strerror(errno))

#define PR_CREATE_NAMEBIND(s, n, a, p) \
  if ((res = pr_namebind_create((s), (n), (a), (p))) < 0) \
    pr_log_pri(PR_LOG_NOTICE, \
      "%s:%d: notice: unable to create namebind '%s': %s", \
      __FILE__, __LINE__, (n), strerror(errno))

#define PR_OPEN_NAMEBIND(n, a, p) \
  if ((res = pr_namebind_open((n), (a), (p))) < 0) \
    pr_log_pri(PR_LOG_NOTICE, \
      "%s:%d: notice: unable to open namebind '%s': %s", \
      __FILE__, __LINE__, (n), strerror(errno))

/* From src/dirtree.c */
extern xaset_t *server_list;
extern server_rec *main_server;

static pr_ipbind_t *ipbind_table[PR_BINDINGS_TABLE_SIZE];
static pool *binding_pool = NULL;
static pr_ipbind_t *ipbind_default_server = NULL,
                   *ipbind_localhost_server = NULL;

/* Server cleanup callback function */
static void server_cleanup_cb(void *conn) {
  *((conn_t **) conn) = NULL;
}

/* The hashing function for the hash table of bindings.  This algorithm
 * is stolen from Apache's http_vhost.c
 */
static unsigned int ipbind_hash_addr(pr_netaddr_t *addr) {
  size_t offset = pr_netaddr_get_inaddr_len(addr);

  /* The key is the last four bytes of the IP address.
   * For IPv4, this is the entire address, as always.
   * For IPv6, this is usually part of the MAC address.
   */
  unsigned int key = *(unsigned *) ((char *) pr_netaddr_get_inaddr(addr) +
    offset - 4);

  key ^= (key >> 16);
  return ((key >> 8) ^ key) % PR_BINDINGS_TABLE_SIZE;
}

/* Slight (clever?) optimization: the loop in server_loop() always
 * calls pr_ipbind_listen(), selects, then pr_ipbind_accept_conn().  Now,
 * rather than having both pr_ipbind_listen() and pr_ipbind_accept_conn()
 * scan the entire ipbind table looking for matches, what if pr_ipbind_listen
 * kept track of which listeners (connt_s) it used, so that
 * pr_ipbind_accept_conn() need merely check those listeners, rather than
 * scanning the entire table itself?
 */

static array_header *listener_list = NULL;

conn_t *pr_ipbind_accept_conn(fd_set *readfds, int *listenfd) {
  conn_t **listeners = listener_list->elts;
  register unsigned int i = 0;

  /* sanity checks */
  if (!readfds) {
    errno = EINVAL;
    return NULL;
  }

  if (!listenfd) {
    errno = EINVAL;
    return NULL;
  }

  for (i = 0; i < listener_list->nelts; i++) {
    conn_t *listener = listeners[i];

    pr_signals_handle();
    if (FD_ISSET(listener->listen_fd, readfds) &&
        listener->mode == CM_LISTEN) {
      int fd = pr_inet_accept_nowait(listener->pool, listener);

      if (fd == -1) {
        /* Handle errors gracefully.  If we're here, then
         * ipbind->ib_server->listen contains either error information, or
         * we just got caught in a blocking condition.
         */
        if (listener->mode == CM_ERROR) {
          pr_log_pri(PR_LOG_ERR, "error: unable to accept an incoming "
            "connection (%s)", strerror(listener->xerrno));
          listener->xerrno = 0;
          listener->mode = CM_LISTEN;
          return NULL;
        }
      }

      *listenfd = fd;
      return listener;
    }
  }

  errno = ENOENT;
  return NULL;
}

int pr_ipbind_add_binds(server_rec *serv) {
  int res = 0;
  config_rec *c = NULL;
  conn_t *listen_conn = NULL;
  pr_netaddr_t *addr = NULL;

  /* sanity check */
  if (!serv)
    return -1;

  c = find_config(serv->conf, CONF_PARAM, "_bind", FALSE);

  while (c) {
    listen_conn = NULL;

    addr = pr_netaddr_get_addr(serv->pool, c->argv[0], NULL);
    if (!addr) {
      pr_log_pri(PR_LOG_NOTICE,
       "notice: unable to determine IP address of '%s'", (char *) c->argv[0]);
      c = find_config_next(c, c->next, CONF_PARAM, "_bind", FALSE);
      continue;
    }

    /* If the SocketBindTight directive is in effect, create a separate
     * listen socket for this address, and add it to the binding list.
     */
    if (SocketBindTight && serv->ServerPort) {
      listen_conn = pr_inet_create_connection(serv->pool, server_list, -1, addr,
        serv->ServerPort, FALSE);

      PR_CREATE_IPBIND(serv, addr);
      PR_OPEN_IPBIND(addr, serv->ServerPort, listen_conn, FALSE, FALSE, TRUE);

    } else {

      PR_CREATE_IPBIND(serv, addr);
      PR_OPEN_IPBIND(addr, serv->ServerPort, serv->listen, FALSE, FALSE, TRUE);
    }

    /* Move on to the next bind directive */
    c = find_config_next(c, c->next, CONF_PARAM, "_bind", FALSE);
  }

  /* done */
  return 0;
}

int pr_ipbind_close(pr_netaddr_t *addr, unsigned int port,
    unsigned char close_namebinds) {
  int res = 0;
  register unsigned int i = 0;

  if (addr) {
    pr_ipbind_t *ipbind = NULL;
    unsigned char have_ipbind = FALSE;

    i = ipbind_hash_addr(addr);

    if (ipbind_table[i] == NULL) {
      pr_log_pri(PR_LOG_NOTICE, "notice: no ipbind found for %s:%d",
        pr_netaddr_get_ipstr(addr), port);
      errno = ENOENT;
      return -1;
    }

    for (ipbind = ipbind_table[i]; ipbind; ipbind = ipbind->ib_next) {
      if (pr_netaddr_cmp(ipbind->ib_addr, addr) == 0 &&
          (!ipbind->ib_port || ipbind->ib_port == port)) {
        have_ipbind = TRUE;
        break;
      }
    }

    if (!have_ipbind) {
      pr_log_pri(PR_LOG_NOTICE, "notice: no ipbind found for %s:%d",
        pr_netaddr_get_ipstr(addr), port);
      errno = ENOENT;
      return -1;
    }

    /* If already closed, exit now. */
    if (!ipbind->ib_isactive) {
      errno = EPERM;
      return -1;
    }

    /* Close the ipbinding's listen connection, if present.  The trick
     * here is determining whether this binding's listen member is
     * _the_ listening socket for the master daemon, or whether it's
     * been created for SocketBindTight, and can be closed.
     *
     * Actually, it's not that hard.  It's only _the_ listening socket
     * for the master daemon in inetd mode, in which case virtual servers
     * can't be shutdown via ftpdctl, anyway.
     */
    if (SocketBindTight && ipbind->ib_listener != NULL) {
      pr_inet_close(ipbind->ib_server->pool, ipbind->ib_listener);
      ipbind->ib_listener = ipbind->ib_server->listen = NULL;
    }

    /* Mark this ipbind as inactive.  For SocketBindTight sockets, the
     * closing of the listening connection will suffice, from the clients'
     * point of view.  However, this covers the non-SocketBindTight case,
     * and will prevent this binding from returning its server_rec pointer
     * on future lookup requests via pr_ipbind_get_server().
     */
    ipbind->ib_isactive = FALSE;

    if (close_namebinds && ipbind->ib_namebinds) {
      register unsigned int j = 0;
      pr_namebind_t **namebinds = NULL;

      namebinds = (pr_namebind_t **) ipbind->ib_namebinds->elts;
      for (j = 0; j < ipbind->ib_namebinds->nelts; j++) {
        pr_namebind_t *nb = namebinds[j];

        PR_CLOSE_NAMEBIND(nb->nb_name, nb->nb_server->addr,
          nb->nb_server->ServerPort);
      }
    }

  } else {

    /* A NULL addr has a special meaning: close _all_ ipbinds in the
     * list.
     */

    for (i = 0; i < PR_BINDINGS_TABLE_SIZE; i++) {
      pr_ipbind_t *ipbind = NULL;
      for (ipbind = ipbind_table[i]; ipbind; ipbind = ipbind->ib_next) {

        if (SocketBindTight && ipbind->ib_listener != NULL) {
          pr_inet_close(main_server->pool, ipbind->ib_listener);
          ipbind->ib_listener = ipbind->ib_server->listen = NULL;
        }

        /* Note: do not need to check if this ipbind was previously closed,
         * for the NULL addr is a request to shut down all ipbinds,
         * regardless of their current state.
         */
        ipbind->ib_isactive = FALSE;

        if (close_namebinds && ipbind->ib_namebinds) {
          register unsigned int j = 0;
          pr_namebind_t **namebinds = NULL;

          namebinds = (pr_namebind_t **) ipbind->ib_namebinds->elts;
          for (j = 0; j < ipbind->ib_namebinds->nelts; j++) {
            pr_namebind_t *nb = namebinds[j];

            PR_CLOSE_NAMEBIND(nb->nb_name, nb->nb_server->addr,
              nb->nb_server->ServerPort);
          }
        }
      }
    }
  }

  /* Done */
  return 0;
}

/* Need a way to close all listening fds in a child process. */
int pr_ipbind_close_listeners(void) {
  conn_t **listeners;
  register unsigned int i = 0;

  /* sanity checks */
  if (!listener_list ||
      listener_list->nelts == 0)
    return 0;

  listeners = listener_list->elts;
  for (i = 0; i < listener_list->nelts; i++) {
    conn_t *listener = listeners[i];

    pr_signals_handle();

    if (listener->listen_fd != -1) {
      close(listener->listen_fd);
      listener->listen_fd = -1;
    }
  }

  return 0;
}

int pr_ipbind_create(server_rec *server, pr_netaddr_t *addr) {
  int res = 0;
  pr_ipbind_t *ipbind = NULL;
  config_rec *c = NULL;
  server_rec *s = NULL;
  register unsigned int i = 0;

  /* Sanity checks */
  if (!server || !addr) {
    errno = EINVAL;
    return -1;
  }

  i = ipbind_hash_addr(addr);

  /* Make sure the address is not already in use */
  for (ipbind = ipbind_table[i]; ipbind; ipbind = ipbind->ib_next) {
    if (pr_netaddr_cmp(ipbind->ib_addr, addr) == 0 &&
        ipbind->ib_port == server->ServerPort) {

      /* An ipbind already exists for this IP address */
      pr_log_pri(PR_LOG_NOTICE, "notice: '%s' (%s:%u) already bound to '%s'",
        server->ServerName, pr_netaddr_get_ipstr(addr), server->ServerPort,
        ipbind->ib_server->ServerName);

      errno = EADDRINUSE;
      return -1;
    }
  }

  if (!binding_pool) {
    /* Initialize the working pool, if not present */
    binding_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(binding_pool, "Bindings Pool");
  }

  ipbind = pcalloc(server->pool, sizeof(pr_ipbind_t));
  ipbind->ib_server = server;
  ipbind->ib_addr = addr;
  ipbind->ib_port = server->ServerPort;
  ipbind->ib_namebinds = NULL;
  ipbind->ib_isdefault = FALSE;
  ipbind->ib_islocalhost = FALSE;
  ipbind->ib_isactive = FALSE;

  /* Add the ipbind to the table. */
  if (ipbind_table[i])
    ipbind->ib_next = ipbind_table[i];

  ipbind_table[i] = ipbind;

  /* Create any namebinds associated with this server. */
  c = find_config(server->conf, CONF_NAMED, NULL, FALSE);

  while (c) {
    s = (server_rec *) c->argv[0];
    PR_CREATE_NAMEBIND(s, c->name, server->addr, server->ServerPort);
    c = find_config_next(c, c->next, CONF_NAMED, NULL, FALSE);
  }

  return 0;
}

pr_ipbind_t *pr_ipbind_find(pr_netaddr_t *addr, unsigned int port,
    unsigned char skip_inactive) {
  pr_ipbind_t *ipbind = NULL;
  register unsigned int i = ipbind_hash_addr(addr);

  for (ipbind = ipbind_table[i]; ipbind; ipbind = ipbind->ib_next) {

    if (skip_inactive && !ipbind->ib_isactive)
      continue;

    if (pr_netaddr_cmp(ipbind->ib_addr, addr) == 0 &&
        (!ipbind->ib_port || ipbind->ib_port == port))
      return ipbind;
  }

  /* default return value */
  return NULL;
}

pr_ipbind_t *pr_ipbind_get(pr_ipbind_t *prev) {
  static unsigned int i = 0;

  if (prev) {

    /* If there's another ipbind in this chain, simply return that. */
    if (prev->ib_next)
      return prev->ib_next;

    /* If the increment is at the maximum size, return NULL (no more chains
     * to be examined).
     */
    if (i == PR_BINDINGS_TABLE_SIZE)
      return NULL;

    /* Increment the index. At this point, we know that the given pointer is
     * the last in the chain, and that there are more chains in the table
     * to be examined.
     */
    i++;

  } else
    /* Reset the index if prev is NULL. */
    i = 0;

  /* Search for the next non-empty chain in the table. */
  for (; i < PR_BINDINGS_TABLE_SIZE; i++) {
    if (ipbind_table[i])
      return ipbind_table[i];
  }

  return NULL;
}

server_rec *pr_ipbind_get_server(pr_netaddr_t *addr, unsigned int port) {
  pr_ipbind_t *ipbind = NULL;

  /* If we've got a binding configured for this exact address, return it
   * straightaway.
   */
  ipbind = pr_ipbind_find(addr, port, TRUE);
  if (ipbind != NULL)
    return ipbind->ib_server;

  /* Use the default server, if set */
  if (ipbind_default_server && ipbind_default_server->ib_isactive)
    return ipbind_default_server->ib_server;

  /* Not found in binding list, and no DefaultServer, so see if it's the
   * loopback address
   */
  if (ipbind_localhost_server && pr_netaddr_loopback(addr))
      return ipbind_localhost_server->ib_server;

  return NULL;
}

int pr_ipbind_listen(fd_set *readfds) {
  int maxfd = 0;
  register unsigned int i = 0;

  /* sanity check */
  if (!readfds)
    return -1;

  FD_ZERO(readfds);

  /* Reset the listener list. */
  if (!listener_list)
    listener_list = make_array(binding_pool, 1, sizeof(conn_t *));

  else
    /* Nasty hack to "clear" the list by making it think it has no
     * elements.
     */
    listener_list->nelts = 0;

  /* Slower than the hash lookup, but...we have to check each and every
   * ipbind in the table.
   */
  for (i = 0; i < PR_BINDINGS_TABLE_SIZE; i++) {
    pr_ipbind_t *ipbind = NULL;

    for (ipbind = ipbind_table[i]; ipbind; ipbind = ipbind->ib_next) {

      /* Skip inactive bindings, but only if SocketBindTight is in effect. */
      if (SocketBindTight && !ipbind->ib_isactive)
        continue;

      if (ipbind->ib_listener) {

        if (ipbind->ib_listener->mode == CM_NONE)
          pr_inet_listen(ipbind->ib_listener->pool, ipbind->ib_listener,
            tcpBackLog);

        if (ipbind->ib_listener->mode == CM_ACCEPT)
          pr_inet_resetlisten(ipbind->ib_listener->pool, ipbind->ib_listener);

        if (ipbind->ib_listener->mode == CM_LISTEN) {
          FD_SET(ipbind->ib_listener->listen_fd, readfds);
          if (ipbind->ib_listener->listen_fd > maxfd)
            maxfd = ipbind->ib_listener->listen_fd;

          /* Add this to the listener list as well. */
          *((conn_t **) push_array(listener_list)) = ipbind->ib_listener;
        }
      }
    }
  }

  return maxfd;
}

int pr_ipbind_open(pr_netaddr_t *addr, unsigned int port, conn_t *listen_conn,
    unsigned char isdefault, unsigned char islocalhost,
    unsigned char open_namebinds) {
  int res = 0;
  pr_ipbind_t *ipbind = NULL;

  /* sanity checks */
  if (!addr) {
    errno = EINVAL;
    return -1;
  }

  /* Find the binding for this server/address */
  if ((ipbind = pr_ipbind_find(addr, port, FALSE)) == NULL) {
    errno = ENOENT;
    return -1;
  }

  if (listen_conn)
    listen_conn->next = NULL;

  ipbind->ib_listener = ipbind->ib_server->listen = listen_conn;
  ipbind->ib_listener = listen_conn;
  ipbind->ib_isdefault = isdefault;
  ipbind->ib_islocalhost = islocalhost;

  /* Stash a pointer to this ipbind, since it is designated as the
   * default server (via the DefaultServer directive), for use in the
   * lookup functions.
   */

  /* Stash pointers to this ipbind for use in the lookup functions if:
   *
   * - It's the default server (specified via the DefaultServer directive)
   * - It handles connections to the loopback interface
   */
  if (isdefault)
    ipbind_default_server = ipbind;
  if (islocalhost)
    ipbind_localhost_server = ipbind;

  /* If requested, look for any namebinds for this ipbind, and open them. */
  if (open_namebinds && ipbind->ib_namebinds) {
    register unsigned int i = 0;
    pr_namebind_t **namebinds = NULL;

    /* NOTE: in the future, these namebinds may need to be stored/
     * manipulated in hash tables themselves, but, for now, linked lists
     * should suffice.
     */
    namebinds = (pr_namebind_t **) ipbind->ib_namebinds->elts;
    for (i = 0; i < ipbind->ib_namebinds->nelts; i++) {
      pr_namebind_t *nb = namebinds[i];

      PR_OPEN_NAMEBIND(nb->nb_name, nb->nb_server->addr,
        nb->nb_server->ServerPort);
    }
  }

  /* Mark this binding as now being active. */
  ipbind->ib_isactive = TRUE;

  return 0;
}

int pr_namebind_close(const char *name, pr_netaddr_t *addr,
    unsigned int port) {
  pr_namebind_t *namebind = NULL;

  /* sanity checks */
  if (!name || !addr) {
    errno = EINVAL;
    return -1;
  }

  /* find the requested namebind */
  if ((namebind = pr_namebind_find(name, addr, port, FALSE)) == NULL) {
    errno = ENOENT;
    return -1;
  }

  /* Mark this binding as inactive */
  namebind->nb_isactive = FALSE;

  /* default return value */
  return 0;
}

int pr_namebind_create(server_rec *server, const char *name,
    pr_netaddr_t *addr, unsigned int port) {
  pr_ipbind_t *ipbind = NULL;
  pr_namebind_t *namebind = NULL, **namebinds = NULL;

  /* sanity checks */
  if (!server || !name) {
    errno = EINVAL;
    return -1;
  }

  /* First, find the ipbind to hold this namebind. */
  ipbind = pr_ipbind_find(addr, port, FALSE);
  if (ipbind == NULL) {
    errno = ENOENT;
    return -1;
  }

  /* Make sure we can add this namebind. */
  if (!ipbind->ib_namebinds) {
    ipbind->ib_namebinds = make_array(binding_pool, 0, sizeof(pr_namebind_t *));

  } else {
    register unsigned int i = 0;
    namebinds = (pr_namebind_t **) ipbind->ib_namebinds->elts;

    /* See if there is already a namebind for the given name. */
    for (i = 0; i < ipbind->ib_namebinds->nelts; i++) {
      namebind = namebinds[i];
      if (namebind && namebind->nb_name && !strcmp(namebind->nb_name, name)) {
        errno = EEXIST;
        return -1;
      }
    }
  }

  /* Allocate a new namebind */
  namebind = (pr_namebind_t *) pcalloc(server->pool, sizeof(pr_namebind_t));

  namebind->nb_name = name;
  namebind->nb_server = server;
  namebind->nb_isactive = FALSE;

  /* Inherit server fields from the container server */
  namebind->nb_server->ServerAdmin = (namebind->nb_server->ServerAdmin ?
    namebind->nb_server->ServerAdmin : server->ServerAdmin ?
    server->ServerAdmin : main_server->ServerAdmin);

  /* These three assignments enforce the use of DNS names as HOST names.
   * Use of DNS names is not a requirement, so in order to be very flexible,
   * these may need to change...
   */
  namebind->nb_server->ServerName = (namebind->nb_server->ServerName ?
    namebind->nb_server->ServerName : (char *) name);
  namebind->nb_server->ServerAddress = (server->ServerAddress ?
    server->ServerAddress : main_server->ServerAddress);
  namebind->nb_server->ServerFQDN = (server->ServerFQDN ?
    server->ServerFQDN : main_server->ServerFQDN);

  namebind->nb_server->tcp_mss_len = (server->tcp_mss_len ?
    server->tcp_mss_len : main_server->tcp_mss_len);
  namebind->nb_server->tcp_rcvbuf_len = (server->tcp_rcvbuf_len ?
    server->tcp_rcvbuf_len : main_server->tcp_rcvbuf_len);
  namebind->nb_server->tcp_rcvbuf_override = (server->tcp_rcvbuf_override ?
    TRUE : main_server->tcp_rcvbuf_override);
  namebind->nb_server->tcp_sndbuf_len = (server->tcp_sndbuf_len ?
    server->tcp_sndbuf_len : main_server->tcp_sndbuf_len);
  namebind->nb_server->tcp_sndbuf_override = (server->tcp_sndbuf_override ?
    TRUE : main_server->tcp_sndbuf_override);

  namebind->nb_server->addr = (server->addr ? server->addr :
    main_server->addr);
  namebind->nb_server->ServerPort = (server->ServerPort ? server->ServerPort :
    main_server->ServerPort);
  namebind->nb_listener = (server->listen ? server->listen :
    main_server->listen);

  /* Add this namebind to the ipbind's list */
  *((pr_namebind_t **) push_array(ipbind->ib_namebinds)) = namebind;

  /* default return value */
  return 0;
}

pr_namebind_t *pr_namebind_find(const char *name, pr_netaddr_t *addr,
    unsigned int port, unsigned char skip_inactive) {
  pr_ipbind_t *ipbind = NULL;
  pr_namebind_t *namebind = NULL;

  /* sanity checks */
  if (!name || !addr) {
    errno = EINVAL;
    return NULL;
  }

  /* first, find an active ipbind for the given addr/port */
  if ((ipbind = pr_ipbind_find(addr, port, skip_inactive)) == NULL) {
    errno = ENOENT;
    return NULL;
  }

  if (!ipbind->ib_namebinds) {
    return NULL;

  } else {
    register unsigned int i = 0;
    pr_namebind_t **namebinds = (pr_namebind_t **) ipbind->ib_namebinds->elts;

    for (i = 0; i < ipbind->ib_namebinds->nelts; i++) {
      namebind = namebinds[i];

      /* skip inactive namebinds */
      if (skip_inactive && namebind && !namebind->nb_isactive)
        continue;

      /* At present, this looks for an exactly matching name.  In the future,
       * we may want to have something like Apache's matching scheme, which
       * looks for the most specific domain to the most general.  Note that
       * that scheme, however, is specific to DNS; should any other naming
       * scheme be desired, that sort of matching will be unnecessary.
       */
      if (namebind && namebind->nb_name && !strcmp(namebind->nb_name, name))
        return namebind;
    }
  }

  /* default return value */
  return NULL;
}

server_rec *pr_namebind_get_server(const char *name, pr_netaddr_t *addr,
    unsigned int port) {
  pr_namebind_t *namebind = NULL;

  /* Basically, just a wrapper around pr_namebind_find() */
  if ((namebind = pr_namebind_find(name, addr, port, TRUE)) == NULL)
    return NULL;

  return namebind->nb_server;
}

int pr_namebind_open(const char *name, pr_netaddr_t *addr, unsigned int port) {
  pr_namebind_t *namebind = NULL;

  /* sanity checks */
  if (!name || !addr) {
    errno = EINVAL;
    return -1;
  }

  /* Find the requested namebind */
  if ((namebind = pr_namebind_find(name, addr, port, FALSE)) == NULL) {
    errno = ENOENT;
    return -1;
  }

  /* Mark this binding as active */
  namebind->nb_isactive = TRUE;

  /* Default return value */
  return 0;
}

void free_bindings(void) {
  if (binding_pool) {
    destroy_pool(binding_pool);
    binding_pool = NULL;
    listener_list = NULL;
  }

  memset(ipbind_table, 0, sizeof(ipbind_table));
}

static void init_inetd_bindings(void) {
  int res = 0;
  server_rec *serv = NULL;
  unsigned char *default_server = NULL, is_default = FALSE;

  main_server->listen = pr_inet_create_connection(main_server->pool,
    server_list, STDIN_FILENO, NULL, INPORT_ANY, FALSE);

  /* Fill in all the important connection information. */
  if (pr_inet_get_conn_info(main_server->listen, STDIN_FILENO) == -1) {
    pr_log_pri(PR_LOG_ERR, "fatal: %s", strerror(errno));

    if (errno == ENOTSOCK)
      pr_log_pri(PR_LOG_ERR, "(Running from command line? "
        "Use `ServerType standalone' in config file!)");
    exit(1);
  }

  if ((default_server = get_param_ptr(main_server->conf, "DefaultServer",
      FALSE)) != NULL && *default_server == TRUE)
    is_default = TRUE;

  PR_CREATE_IPBIND(main_server, main_server->addr);
  PR_OPEN_IPBIND(main_server->addr, main_server->ServerPort,
    main_server->listen, is_default, TRUE, TRUE);
  PR_ADD_IPBINDS(main_server);

  /* Now attach the faked connection to all virtual servers. */
  for (serv = main_server->next; serv; serv = serv->next) {

    /* Because this server is sharing the connection with the
     * main server, we need a cleanup handler to remove
     * the server's reference when the original connection's
     * pool is destroyed.
     */

    serv->listen = main_server->listen;
    register_cleanup(serv->listen->pool, &serv->listen, server_cleanup_cb,
      server_cleanup_cb);

    is_default = FALSE;
    if ((default_server = get_param_ptr(serv->conf, "DefaultServer",
        FALSE)) != NULL && *default_server == TRUE)
      is_default = TRUE;

    PR_CREATE_IPBIND(serv, serv->addr);
    PR_OPEN_IPBIND(serv->addr, serv->ServerPort, serv->listen, is_default,
      FALSE, TRUE);
    PR_ADD_IPBINDS(serv);
  }

  return;
}

static void init_standalone_bindings(void) {
  int res = 0;
  server_rec *serv = NULL;
  unsigned char *default_server = NULL, is_default = FALSE;

  /* If a port is set to zero, the address/port is not bound to a socket
   * at all.
   */
  if (main_server->ServerPort) {

    /* If SocketBindTight is off, then pr_inet_create_connection() will
     * create and bind to a wildcard socket.  However, should it be an
     * IPv4 or an IPv6 wildcard socket?  Check the main_server->addr's
     * family, and use that.
     */
    if (!SocketBindTight)
      pr_inet_set_default_family(NULL,
        pr_netaddr_get_family(main_server->addr));

    main_server->listen =
      pr_inet_create_connection(main_server->pool, server_list, -1,
        (SocketBindTight ? main_server->addr : NULL),
        main_server->ServerPort, FALSE);

  } else
    main_server->listen = NULL;

  if ((default_server = get_param_ptr(main_server->conf, "DefaultServer",
      FALSE)) != NULL && *default_server == TRUE)
    is_default = TRUE;

  if (main_server->ServerPort || is_default) {
    PR_CREATE_IPBIND(main_server, main_server->addr);
    PR_OPEN_IPBIND(main_server->addr, main_server->ServerPort,
      main_server->listen, is_default, TRUE, TRUE);
    PR_ADD_IPBINDS(main_server);
  }

  for (serv = main_server->next; serv; serv = serv->next) {
    if (serv->ServerPort != main_server->ServerPort || SocketBindTight ||
        !main_server->listen) {
      is_default = FALSE;

      if ((default_server = get_param_ptr(serv->conf, "DefaultServer",
          FALSE)) != NULL && *default_server == TRUE)
        is_default = TRUE;

      if (serv->ServerPort) {
        if (!SocketBindTight)
          pr_inet_set_default_family(NULL,
            pr_netaddr_get_family(serv->addr));

        serv->listen = pr_inet_create_connection(serv->pool, server_list, -1,
          (SocketBindTight ? serv->addr : NULL), serv->ServerPort, FALSE);

        PR_CREATE_IPBIND(serv, serv->addr);
        PR_OPEN_IPBIND(serv->addr, serv->ServerPort, serv->listen, is_default,
          FALSE, TRUE);
        PR_ADD_IPBINDS(serv);

      } else if (is_default) {
        serv->listen = NULL;

        PR_CREATE_IPBIND(serv, serv->addr);
        PR_OPEN_IPBIND(serv->addr, serv->ServerPort, serv->listen, is_default,
          FALSE, TRUE);
        PR_ADD_IPBINDS(serv);

      } else
        serv->listen = NULL;

    } else {

      /* Because this server is sharing the connection with the
       * main server, we need a cleanup handler to remove
       * the server's reference when the original connection's
       * pool is destroyed.
       */

      is_default = FALSE;
      if ((default_server = get_param_ptr(serv->conf, "DefaultServer",
          FALSE)) != NULL && *default_server == TRUE)
        is_default = TRUE;

      serv->listen = main_server->listen;
      register_cleanup(serv->listen->pool, &serv->listen, server_cleanup_cb,
        server_cleanup_cb);

      PR_CREATE_IPBIND(serv, serv->addr);
      PR_OPEN_IPBIND(serv->addr, serv->ServerPort, NULL, is_default, FALSE,
        TRUE);
      PR_ADD_IPBINDS(serv);
    }
  }

  /* done */
  return;
}

void init_bindings(void) {
  if (ServerType == SERVER_INETD)
    init_inetd_bindings();

  else if (ServerType == SERVER_STANDALONE)
    init_standalone_bindings();

  return;
}

