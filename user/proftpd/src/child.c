/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004 The ProFTPD Project team
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

/* Children management code
 * $Id: child.c,v 1.4 2004/05/07 03:36:28 castaglia Exp $
 */

#include "conf.h"

#include <signal.h>

static pool *child_pool = NULL;
static xaset_t *child_list = NULL;
static unsigned long child_listlen = 0;

int child_add(pid_t pid, int fd) {
  pool *p;
  pr_child_t *ch;

  /* If no child-tracking list has been allocated, create one. */
  if (!child_pool) {
    child_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(child_pool, "Child Pool");
  }

  if (!child_list)
    child_list = xaset_create(make_sub_pool(child_pool), NULL);

  p = make_sub_pool(child_pool);
  pr_pool_tag(p, "child session pool");

  ch = pcalloc(p, sizeof(pr_child_t));
  ch->ch_pool = p;  
  ch->ch_pid = pid;
  time(&ch->ch_when);
  ch->ch_pipefd = fd;
  ch->ch_dead = FALSE;

  xaset_insert(child_list, (xasetmember_t *) ch);
  child_listlen++;

  return 0;
}

unsigned long child_count(void) {
  return child_listlen;
}

pr_child_t *child_get(pr_child_t *ch) {
  if (!ch)
    return (pr_child_t *) child_list->xas_list;

  return ch->next;
}

int child_remove(pid_t pid) {
  pr_child_t *ch;

  if (!child_list) {
    errno = EPERM;
    return -1;
  }

  for (ch = (pr_child_t *) child_list->xas_list; ch; ch = ch->next) {
    if (ch->ch_pid == pid) {
      ch->ch_dead = TRUE;
      child_listlen--;
      return 0;
    }
  }

  errno = ENOENT;
  return -1;
}

void child_signal(int signo) {
  pr_child_t *ch;

  if (!child_list)
    return;

  for (ch = (pr_child_t *) child_list->xas_list; ch; ch = ch->next) {
    kill(ch->ch_pid, signo);
  }

  return;
}

void child_update(void) {
  pr_child_t *ch, *chn = NULL;

  if (!child_list)
    return;

  /* Scan the child list, removing those entries marked as 'dead'. */
  for (ch = (pr_child_t *) child_list->xas_list; ch; ch = chn) {
    chn = ch->next;

    if (ch->ch_dead) {
      if (ch->ch_pipefd != -1)
        close(ch->ch_pipefd);

      xaset_remove(child_list, (xasetmember_t *) ch);
      destroy_pool(ch->ch_pool);
    }
  }

  /* If the child list is empty, recover the list pool memory. */
  if (!child_list->xas_list) {
    destroy_pool(child_list->pool);
    child_list = NULL;
  }

  return;
}
