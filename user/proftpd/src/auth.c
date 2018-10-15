/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2005 The ProFTPD Project team
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

/* Authentication front-end for ProFTPD
 * $Id: auth.c,v 1.42 2005/03/05 17:46:02 castaglia Exp $
 */

#include "conf.h"

/* The difference between this function, and pr_cmd_alloc(), is that this
 * allocates the cmd_rec directly from the given pool, whereas pr_cmd_alloc()
 * will allocate a subpool from the given pool, and allocate its cmd_rec
 * from the subpool.  This means that pr_cmd_alloc()'s cmd_rec's can be
 * subsequently destroyed easily; this function's cmd_rec's will be destroyed
 * when the given pool is destroyed.
 */
static cmd_rec *make_cmd(pool *cp, int argc, ...) {
  va_list args;
  cmd_rec *c;
  int     i;

  c = pcalloc(cp, sizeof(cmd_rec));

  c->argc = argc;
  c->stash_index = -1;

  if (argc) {
    c->argv = pcalloc(cp, sizeof(void *) * (argc + 1));

    va_start(args, argc);

    for (i = 0; i < argc; i++)
      c->argv[i] = (void *) va_arg(args, char *);

    va_end(args);

    c->argv[argc] = NULL;
  }

  return c;
}

static modret_t *dispatch_auth(cmd_rec *cmd, char *match) {
  authtable *authtab = NULL;
  modret_t *mr = NULL;

  authtab = pr_stash_get_symbol(PR_SYM_AUTH, match, NULL,
    &cmd->stash_index);

  while (authtab) {
    pr_log_debug(DEBUG6, "dispatching auth request \"%s\" to module mod_%s",
      match, authtab->m->name);

    mr = call_module(authtab->m, authtab->handler, cmd);

    if (authtab->auth_flags & PR_AUTH_FL_REQUIRED)
      break;

    if (MODRET_ISHANDLED(mr) ||
        MODRET_ISERROR(mr))
      break;

    authtab = pr_stash_get_symbol(PR_SYM_AUTH, match, authtab,
      &cmd->stash_index);
  }

  return mr;
}

void pr_auth_setpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "setpwent");

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

void pr_auth_endpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "endpwent");

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

void pr_auth_setgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "setgrent");

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

void pr_auth_endgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "endgrent");

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

struct passwd *pr_auth_getpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "getpwent");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL)
    return NULL;

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct group *pr_auth_getgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "getgrent");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL)
    return NULL;

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct passwd *pr_auth_getpwnam(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "getpwnam");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "no such user '%s'", name);
    return NULL;
  }

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct passwd *pr_auth_getpwuid(pool *p, uid_t uid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;

  cmd = make_cmd(p, 1, (void *) &uid);
  mr = dispatch_auth(cmd, "getpwuid");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "no such UID '%lu'", (unsigned long)uid);
    return NULL;
  }

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct group *pr_auth_getgrnam(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "getgrnam");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "no such group '%s'", name);
    return NULL;
  }

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct group *pr_auth_getgrgid(pool *p, gid_t gid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 1, (void *) &gid);
  mr = dispatch_auth(cmd, "getgrgid");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "no such GID '%lu'", (unsigned long)gid);
    return NULL;
  }

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

int pr_auth_authenticate(pool *p, const char *name, const char *pw) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  int res = PR_AUTH_NOPWD;

  if (!pw)
    pw = "";

  cmd = make_cmd(p, 2, name, pw);
  mr = dispatch_auth(cmd, "auth");

  if (MODRET_ISHANDLED(mr))
    res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

  else if (MODRET_ISERROR(mr))
    res = MODRET_ERROR(mr);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_check(pool *p, const char *cpw, const char *name, const char *pw) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  int res = PR_AUTH_BADPWD;

  cmd = make_cmd(p, 3, cpw, name, pw);
  mr = dispatch_auth(cmd, "check");

  if (MODRET_ISHANDLED(mr))
    res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

const char *pr_auth_uid2name(pool *p, uid_t uid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  static char namebuf[64];
  char *res = "(?)";

  memset(namebuf, '\0', sizeof(namebuf));

  cmd = make_cmd(p, 1, (void *) &uid);
  mr = dispatch_auth(cmd, "uid2name");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr)) {
    res = mr->data;
    sstrncpy(namebuf, res, sizeof(namebuf));
    res = namebuf;
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

const char *pr_auth_gid2name(pool *p, gid_t gid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  static char namebuf[64];
  char *res = "(?)";

  memset(namebuf, '\0', sizeof(namebuf));

  cmd = make_cmd(p, 1, (void *) &gid);
  mr = dispatch_auth(cmd, "gid2name");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr)) {
    res = mr->data;
    sstrncpy(namebuf, res, sizeof(namebuf));
    res = namebuf;
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

uid_t pr_auth_name2uid(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  uid_t res = (uid_t) -1;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "name2uid");

  if (MODRET_ISHANDLED(mr))
    res = *((uid_t *) mr->data);
  else
    errno = EINVAL;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

gid_t pr_auth_name2gid(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  gid_t res = (gid_t) -1;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "name2gid");

  if (MODRET_ISHANDLED(mr))
    res = *((gid_t *) mr->data);
  else
    errno = EINVAL;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_getgroups(pool *p, const char *name, array_header **group_ids,
    array_header **group_names) {

  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  int res = -1;

  /* Allocate memory for the array_headers of GIDs and group names. */
  if (group_ids)
    *group_ids = make_array(permanent_pool, 2, sizeof(gid_t));

  if (group_names)
    *group_names = make_array(permanent_pool, 2, sizeof(char *));

  cmd = make_cmd(p, 3, name, group_ids ? *group_ids : NULL,
    group_names ? *group_names : NULL);

  mr = dispatch_auth(cmd, "getgroups");

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr)) {
    res = *((int *) mr->data);

    /* Note: the number of groups returned should, barring error,
     * always be at least 1, as per getgroups(2) behavior.  This one
     * ID is present because it is the primary group membership set in
     * struct passwd, from /etc/passwd.  This will need to be documented
     * for the benefit of auth_getgroup() implementors.
     */

    if (group_ids) {
      register unsigned int i;
      char *strgids = "";
      gid_t *gids = (*group_ids)->elts;

      for (i = 0; i < (*group_ids)->nelts; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf)-1, "%lu", (unsigned long) gids[i]);
        buf[sizeof(buf)-1] = '\0';

        strgids = pstrcat(p, strgids, i != 0 ? ", " : "", buf, NULL);
      }

      pr_log_debug(DEBUG10, "retrieved group %s: %s",
        (*group_ids)->nelts == 1 ? "ID" : "IDs", strgids);
    }

    if (group_names) {
      register unsigned int i;
      char *strgroups = ""; 
      char **groups = (*group_names)->elts;

      for (i = 0; i < (*group_names)->nelts; i++)
        strgroups = pstrcat(p, strgroups, i != 0 ? ", " : "", groups[i], NULL);
    
      pr_log_debug(DEBUG10, "retrieved group %s: %s",
        (*group_names)->nelts == 1 ? "name" : "names", strgroups);
    }
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

/* Helper function for pr_auth_get_anon_config(), for handling the
 * (horrible) AnonymousGroup directive.
 */
static config_rec *auth_anonymous_group(pool *p, char *user) {
  config_rec *c;
  int ret = 0;

  /* Retrieve the session group membership information, so that this check
   * may work properly.
   */
  if (!session.gids && !session.groups &&
      (ret = pr_auth_getgroups(p, user, &session.gids, &session.groups)) < 1)
    pr_log_debug(DEBUG2, "no supplemental groups found for user '%s'", user);

  c = find_config(main_server->conf, CONF_PARAM, "AnonymousGroup", FALSE);

  if (c)
    do {
      ret = pr_expr_eval_group_and((char **) c->argv);

    } while (!ret &&
      (c = find_config_next(c, c->next, CONF_PARAM, "AnonymousGroup",
        FALSE)) != NULL);

  return ret ? c : NULL;
}

/* This is one messy function.  Yuck.  Yay legacy code. */
config_rec *pr_auth_get_anon_config(pool *p, char **login_name,
    char **user_name, char **anon_name) {
  config_rec *c = NULL, *topc = NULL;
  char *config_user_name, *config_anon_name = NULL;
  unsigned char is_alias = FALSE, force_anon = FALSE, *auth_alias_only = NULL;

  /* Precendence rules:
   *   1. Search for UserAlias directive.
   *   2. Search for Anonymous directive.
   *   3. Normal user login
   */

  config_user_name = get_param_ptr(main_server->conf, "UserName", FALSE);
  if (config_user_name && user_name)
    *user_name = config_user_name;

  c = find_config(main_server->conf, CONF_PARAM, "UserAlias", TRUE);
  if (c) {
    do {
      if (strcmp(c->argv[0], "*") == 0 ||
          strcmp(c->argv[0], *login_name) == 0) {
        is_alias = TRUE;
        break;
      }

    } while ((c = find_config_next(c, c->next, CONF_PARAM, "UserAlias",
      TRUE)) != NULL);
  }

  /* This is where things get messy, rapidly. */
  topc = c;

  while (c && c->parent &&
    (auth_alias_only = get_param_ptr(c->parent->set, "AuthAliasOnly", FALSE))) {

    /* while() loops should always handle signals. */
    pr_signals_handle();

    /* If AuthAliasOnly is on, ignore this one and continue. */
    if (auth_alias_only &&
        *auth_alias_only == TRUE) {
      c = find_config_next(c, c->next, CONF_PARAM, "UserAlias", TRUE);
      continue;
    }

    is_alias = FALSE;

    find_config_set_top(topc);
    c = find_config_next(c, c->next, CONF_PARAM, "UserAlias", TRUE);

    if (c &&
        (strcmp(c->argv[0], "*") == 0 ||
         strcmp(c->argv[0], *login_name) == 0))
      is_alias = TRUE;
  }

  if (c) {
    *login_name = c->argv[1];

    /* If the alias is applied inside an <Anonymous> context, we have found
     * our anon block.
     */
    if (c->parent &&
        c->parent->config_type == CONF_ANON)
      c = c->parent;
    else
      c = NULL;
  }

  /* Next, search for an anonymous entry. */

  if (!c)
    c = find_config(main_server->conf, CONF_ANON, NULL, FALSE);
  else
    find_config_set_top(c);

  if (c) {
    do {
      config_anon_name = get_param_ptr(c->subset, "UserName", FALSE);

      if (!config_anon_name)
        config_anon_name = config_user_name;

      if (config_anon_name &&
          strcmp(config_anon_name, *login_name) == 0) {
         if (anon_name)
           *anon_name = config_anon_name;
         break;
      }
 
    } while ((c = find_config_next(c, c->next, CONF_ANON, NULL,
      FALSE)) != NULL);
  }

  if (!c) {
    c = auth_anonymous_group(p, *login_name);

    if (c)
      force_anon = TRUE;
  }

  if (!is_alias && !force_anon) {
    auth_alias_only = get_param_ptr(c ? c->subset : main_server->conf,
      "AuthAliasOnly", FALSE);

    if (auth_alias_only &&
        *auth_alias_only == TRUE) {
      if (c && c->config_type == CONF_ANON)
        c = NULL;
      else
        *login_name = NULL;

      auth_alias_only = get_param_ptr(main_server->conf, "AuthAliasOnly",
        FALSE);
      if (*login_name &&
          auth_alias_only &&
          *auth_alias_only == TRUE)
        *login_name = NULL;

      if ((!login_name || !c) &&
          anon_name)
        *anon_name = NULL;
    }
  }

  return c;
}

int set_groups(pool *p, gid_t primary_gid, array_header *suppl_gids) {
  int res = 0;
  pool *tmp_pool = NULL;

#ifdef HAVE_SETGROUPS
  register unsigned int i = 0;
  gid_t *gids = NULL, *proc_gids = NULL;
  size_t ngids = 0, nproc_gids = 0;
  char *strgids = "";

  /* sanity check */
  if (!p || !suppl_gids)
    return 0;

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "set_groups() tmp pool");

  /* Check for a NULL supplemental group ID list. */
  if (suppl_gids) {
    ngids = suppl_gids->nelts;
    gids = suppl_gids->elts;

    if (ngids && gids) {
      proc_gids = pcalloc(tmp_pool, sizeof(gid_t) * (ngids));

      /* Note: the list of supplemental GIDs may contain duplicates.  Sort
       * through the list and keep only the unique IDs - this should help avoid
       * running into the NGROUPS limit when possible.  This algorithm may slow
       * things down some; optimize it if/when possible.
       */
      proc_gids[nproc_gids++] = gids[0];
    }
  }

  for (i = 1; i < ngids; i++) {
    register unsigned int j = 0;
    unsigned char skip_gid = FALSE;

    /* This duplicate ID search only needs to be done after the first GID
     * in the given list is examined, as the first GID cannot be a duplicate.
     */
    for (j = 0; j < nproc_gids; j++) {
      if (proc_gids[j] == gids[i]) {
        skip_gid = TRUE;
        break;
      }
    }

    if (!skip_gid)
      proc_gids[nproc_gids++] = gids[i];
  }

  for (i = 0; i < nproc_gids; i++) {
    char buf[64];
    snprintf(buf, sizeof(buf)-1, "%lu", (unsigned long) proc_gids[i]);
    buf[sizeof(buf)-1] = '\0';

    strgids = pstrcat(p, strgids, i != 0 ? ", " : "", buf, NULL);
  }

  pr_log_debug(DEBUG10, "setting group %s: %s", nproc_gids == 1 ? "ID" : "IDs",
    strgids);

  /* Set the supplemental groups. */
  res = setgroups(nproc_gids, proc_gids);
  if (res < 0) {
    destroy_pool(tmp_pool);
    return res;
  }
#endif /* !HAVE_SETGROUPS */

#ifndef PR_DEVEL_COREDUMP
  /* Set the primary GID of the process.
   */
  res = setgid(primary_gid);
  if (res < 0) {
    if (tmp_pool)
      destroy_pool(tmp_pool);
    return res;
  }
#endif /* PR_DEVEL_COREDUMP */

  if (tmp_pool)
    destroy_pool(tmp_pool);

  return res;
}

