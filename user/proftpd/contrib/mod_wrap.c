/*
 * ProFTPD: mod_wrap -- use Wietse Venema's TCP wrappers library for
 *                      access control
 *
 * Copyright (c) 2000-2003 TJ Saunders
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
 * As a special exemption, TJ Saunders gives permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * -- DO NOT MODIFY THE TWO LINES BELOW --
 * $Libraries: -lwrap -lnsl$
 * $Id: mod_wrap.c,v 1.14 2004/10/30 23:16:41 castaglia Exp $
 */

#define MOD_WRAP_VERSION "mod_wrap/1.2.3"

#include "conf.h"
#include "privs.h"
#include "tcpd.h"

/* these need to be defined for the libwrap functions -- default settings
 * are those from tcpd.h
 */
int allow_severity = PR_LOG_INFO;
int deny_severity = PR_LOG_WARNING;

/* function prototypes */
static int wrap_eval_expression(char **, array_header *);
static char *wrap_get_user_table(cmd_rec *, char *, char *);
static int wrap_is_usable_file(char *);
static void wrap_log_request_allowed(int, struct request_info *);
static void wrap_log_request_denied(int, struct request_info *);
static config_rec *wrap_resolve_user(pool *, char **);

static char *wrap_service_name = "proftpd";

/* Support routines
 */

/* boolean "expression" matching, returns TRUE if the entire expression matches
 */
static int wrap_eval_expression(char **config_expr,
    array_header *session_expr) {

  unsigned char found = FALSE;
  int index = 0;
  char *elem = NULL, **list = NULL;

  /* sanity check */
  if (!config_expr || !*config_expr || !session_expr)
    return FALSE;

  list = (char **) session_expr->elts;

  for (; *config_expr; config_expr++) {
    elem = *config_expr;
    found = FALSE;

    if (*elem == '!') {
      found = !found;
      elem++;
    }

    for (index = 0; index < session_expr->nelts; index++) {
      if (list[index] && !strcmp(list[index], elem)) {
        found = !found;
        break;
      }
    }

    if (!found) {
      config_expr = NULL;
      break;
    }
  }

  if (config_expr)
    return TRUE;

  return FALSE;
}

/* Determine logging-in user's access table locations.  This function was
 * "borrowed" (ie plagiarized/copied/whatever) liberally from modules/
 * mod_auth.c -- the _true_ author is MacGuyver <macguyver@tos.net>.
 */
static char *wrap_get_user_table(cmd_rec *cmd, char *user,
    char *path) {

  char *realpath = NULL;
  struct passwd *pw = NULL;

  pw = pr_auth_getpwnam(cmd->pool, user);

  /* For the dir_realpath() function to work, some session members need to
   * be set.
   */
  session.user = pstrdup(cmd->pool, pw->pw_name);
  session.login_uid = pw->pw_uid;

  PRIVS_USER
  realpath = dir_realpath(cmd->pool, path);
  PRIVS_RELINQUISH

  if (realpath)
    path = realpath;

  return path;
}

static int wrap_is_usable_file(char *filename) {
  struct stat statbuf;
  pr_fh_t *fh = NULL;

  /* check the easy case first */
  if (filename == NULL)
    return FALSE;

  if (pr_fsio_stat(filename, &statbuf) == -1) {
    pr_log_pri(PR_LOG_INFO, MOD_WRAP_VERSION ": \"%s\": %s", filename,
      strerror(errno));
    return FALSE;
  }

  /* OK, the file exists.  Now, to make sure that the current process
   * can _read_ the file
   */
  fh = pr_fsio_open(filename, O_RDONLY);
  if (fh == NULL) {
    pr_log_pri(PR_LOG_INFO, MOD_WRAP_VERSION ": \"%s\": %s", filename,
      strerror(errno));
    return FALSE;
  }
  pr_fsio_close(fh);

  return TRUE;
}

static void wrap_log_request_allowed(int priority,
    struct request_info *request) {
  pr_log_pri(priority, MOD_WRAP_VERSION ": allowed connection from %s",
    eval_client(request));

  /* done */
  return;
}

static void wrap_log_request_denied(int priority,
    struct request_info *request) {
  pr_log_pri(priority, MOD_WRAP_VERSION ": refused connection from %s",
    eval_client(request));

  /* done */
  return;
}

/* yet more plagiarizing...this one raided from mod_auth's _auth_resolve_user()
 * function [in case you haven't noticed yet, I'm quite the hack, in the
 * _true_ sense of the world]. =) hmmm...I wonder if it'd be feasible
 * to make some of mod_auth's functions visible from src/auth.c?
 */
static config_rec *wrap_resolve_user(pool *pool, char **user) {
  config_rec *conf = NULL, *top_conf;
  char *ourname = NULL, *anonname = NULL;
  unsigned char is_alias = FALSE, force_anon = FALSE;

  /* Precendence rules:
   *   1. Search for UserAlias directive.
   *   2. Search for Anonymous directive.
   *   3. Normal user login
   */

  ourname = (char*) get_param_ptr(main_server->conf, "UserName", FALSE);

  conf = find_config(main_server->conf, CONF_PARAM, "UserAlias", TRUE);

  if (conf) do {
    if (!strcmp(conf->argv[0], "*") || !strcmp(conf->argv[0], *user)) {
      is_alias = TRUE;
      break;
    } 

  } while ((conf = find_config_next(conf, conf->next, CONF_PARAM,
    "UserAlias", TRUE)) != NULL);

  /* if AuthAliasOnly is set, ignore this one and continue */
  top_conf = conf;

  while (conf && conf->parent &&
      find_config(conf->parent->set, CONF_PARAM, "AuthAliasOnly", FALSE)) {

    is_alias = FALSE;
    find_config_set_top(top_conf);
    conf = find_config_next(conf, conf->next, CONF_PARAM, "UserAlias", TRUE);

    if (conf && (!strcmp(conf->argv[0], "*") || !strcmp(conf->argv[0], *user)))
      is_alias = TRUE;
  }

  if (conf) {
    *user = conf->argv[1];

    /* If the alias is applied inside an <Anonymous> context, we have found
     * our anon block
     */
    if (conf->parent && conf->parent->config_type == CONF_ANON)
      conf = conf->parent;
    else
      conf = NULL;
  }

  /* Next, search for an anonymous entry */
  if (!conf)
    conf = find_config(main_server->conf, CONF_ANON, NULL, FALSE);

  else
    find_config_set_top(conf);

  if (conf) do {
    anonname = (char*) get_param_ptr(conf->subset, "UserName", FALSE);

    if (!anonname)
      anonname = ourname;

    if (anonname && !strcmp(anonname, *user)) {
       break;
    }

  } while ((conf = find_config_next(conf, conf->next, CONF_ANON, NULL,
    FALSE)) != NULL);

  if (!is_alias && !force_anon) {

    if (find_config((conf ? conf->subset :
        main_server->conf), CONF_PARAM, "AuthAliasOnly", FALSE)) {

      if (conf && conf->config_type == CONF_ANON)
        conf = NULL;

      else
        *user = NULL;

      if (*user && find_config(main_server->conf, CONF_PARAM, "AuthAliasOnly",
          FALSE))
        *user = NULL;
    }
  }

  return conf;
}

/* Configuration handlers
 */

MODRET add_tcpaccessfiles(cmd_rec *cmd) {
  config_rec *c = NULL;

  /* assume use of the standard TCP wrappers installation locations */
  char *allow_filename = "/etc/hosts.allow";
  char *deny_filename = "/etc/hosts.deny";

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_ANON|CONF_VIRTUAL|CONF_GLOBAL);

  /* use the user-given files, checking to make sure that they exist and
   * are readable.
   */
  allow_filename = cmd->argv[1];
  deny_filename = cmd->argv[2];

  /* if the filenames begin with a '~', AND this is not immediately followed
   * by a '/' (ie '~/'), expand it out for checking and storing for later
   * lookups.  If the filenames DO begin with '~/', do the expansion later,
   * after authenication.  In other words, do checking of static filenames
   * now, and checking of dynamic (user-authentication-based) filenames
   * later.
   */
  if (allow_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(allow_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

  } else if (allow_filename[0] == '~' && allow_filename[1] != '/') {
    char *allow_real_file = NULL;

    allow_real_file = dir_realpath(cmd->pool, allow_filename);

    if (allow_real_file == NULL || !wrap_is_usable_file(allow_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

    allow_filename = allow_real_file;

  } else if (allow_filename[0] != '~' && allow_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", allow_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  if (deny_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(deny_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

  } else if (deny_filename[0] == '~' && deny_filename[1] != '/') {
    char *deny_real_file = NULL;

    deny_real_file = dir_realpath(cmd->pool, deny_filename);

    if (deny_real_file == NULL || !wrap_is_usable_file(deny_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

    deny_filename = deny_real_file;

  } else if (deny_filename[0] != '~' && deny_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", deny_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  c = add_config_param_str(cmd->argv[0], 2, (void *) allow_filename,
    (void *) deny_filename);
  c->flags |= CF_MERGEDOWN;

  /* done */
  return HANDLED(cmd);
}

MODRET add_tcpgroupaccessfiles(cmd_rec *cmd) {
  int group_argc = 1;
  char **group_argv = NULL;
  array_header *group_acl = NULL;
  config_rec *c = NULL;

  /* assume use of the standard TCP wrappers installation locations */
  char *allow_filename = NULL, *deny_filename = NULL;

  CHECK_ARGS(cmd, 3);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* use the user-given files, checking to make sure that they exist and
   * are readable.
   */
  allow_filename = cmd->argv[2];
  deny_filename = cmd->argv[3];

  /* if the filenames begin with a '~', AND this is not immediately followed
   * by a '/' (ie '~/'), expand it out for checking and storing for later
   * lookups.  If the filenames DO begin with '~/', do the expansion later,
   * after authenication.  In other words, do checking of static filenames
   * now, and checking of dynamic (user-authentication-based) filenames
   * later.
   */
  if (allow_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(allow_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

  } else if (allow_filename[0] == '~' && allow_filename[1] != '/') {
    char *allow_real_file = NULL;

    allow_real_file = dir_realpath(cmd->pool, allow_filename);

    if (allow_real_file == NULL || !wrap_is_usable_file(allow_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

    allow_filename = allow_real_file;

  } else if (allow_filename[0] != '~' && allow_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", allow_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  if (deny_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(deny_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

  } else if (deny_filename[0] == '~' && deny_filename[1] != '/') {
    char *deny_real_file = NULL;

    deny_real_file = dir_realpath(cmd->pool, deny_filename);

    if (deny_real_file == NULL || !wrap_is_usable_file(deny_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

    deny_filename = deny_real_file;

  } else if (deny_filename[0] != '~' && deny_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", deny_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  c = add_config_param(cmd->argv[0], 0);

  group_acl = pr_expr_create(cmd->tmp_pool, &group_argc, &cmd->argv[0]);

  /* build the desired config_rec manually */
  c->argc = group_argc + 2;
  c->argv = pcalloc(c->pool, (group_argc + 3) * sizeof(char *));
  group_argv = (char **) c->argv;

  /* the access files are the first two arguments */
  *group_argv++ = pstrdup(c->pool, allow_filename);
  *group_argv++ = pstrdup(c->pool, deny_filename);

  /* and the group names follow */
  if (group_argc && group_acl)
    while (group_argc--) {
      *group_argv++ = pstrdup(c->pool, *((char **) group_acl->elts));
      group_acl->elts = ((char **) group_acl->elts) + 1;
    }

  /* don't forget to NULL-terminate */
  *group_argv = NULL;

  c->flags |= CF_MERGEDOWN;

  /* done */
  return HANDLED(cmd);
}

MODRET add_tcpuseraccessfiles(cmd_rec *cmd) {
  int user_argc = 1;
  char **user_argv = NULL;
  array_header *user_acl = NULL;
  config_rec *c = NULL;

  /* assume use of the standard TCP wrappers installation locations */
  char *allow_filename = NULL, *deny_filename = NULL;

  CHECK_ARGS(cmd, 3);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* use the user-given files, checking to make sure that they exist and
   * are readable.
   */
  allow_filename = cmd->argv[2];
  deny_filename = cmd->argv[3];

  /* if the filenames begin with a '~', AND this is not immediately followed
   * by a '/' (ie '~/'), expand it out for checking and storing for later
   * lookups.  If the filenames DO begin with '~/', do the expansion later,
   * after authenication.  In other words, do checking of static filenames
   * now, and checking of dynamic (user-authentication-based) filenames
   * later.
   */
  if (allow_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(allow_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

  } else if (allow_filename[0] == '~' && allow_filename[1] != '/') {
    char *allow_real_file = NULL;

    allow_real_file = dir_realpath(cmd->pool, allow_filename);

    if (allow_real_file == NULL || !wrap_is_usable_file(allow_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", allow_filename, "' must be a usable file", NULL));

    allow_filename = allow_real_file;

  } else if (allow_filename[0] != '~' && allow_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", allow_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  if (deny_filename[0] == '/') {

    /* it's an absolute path, so the filename will be checked as is */
    if (!wrap_is_usable_file(deny_filename))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

  } else if (deny_filename[0] == '~' && deny_filename[1] != '/') {
    char *deny_real_file = NULL;

    deny_real_file = dir_realpath(cmd->pool, deny_filename);

    if (deny_real_file == NULL || !wrap_is_usable_file(deny_real_file))
      return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
        cmd->argv[0], ": '", deny_filename, "' must be a usable file", NULL));

    deny_filename = deny_real_file;

  } else if (deny_filename[0] != '~' && deny_filename[0] != '/') {

    /* no relative paths allowed */
    return ERROR_MSG(cmd, NULL, pstrcat(cmd->tmp_pool,
      cmd->argv[0], ": '", deny_filename, "' must start with \"/\" or \"~\"",
      NULL));

  } else {

    /* it's a determine-at-login-time filename -- check it later */
    ;
  }

  c = add_config_param_str(cmd->argv[0], 0);

  user_acl = pr_expr_create(cmd->tmp_pool, &user_argc, &cmd->argv[0]);

  /* build the desired config_rec manually */
  c->argc = user_argc + 2;
  c->argv = pcalloc(c->pool, (user_argc + 3) * sizeof(char *));
  user_argv = (char **) c->argv;

  /* the access files are the first two arguments */
  *user_argv++ = pstrdup(c->pool, allow_filename);
  *user_argv++ = pstrdup(c->pool, deny_filename);

  /* and the user names follow */
  if (user_argc && user_acl)
    while (user_argc--) {
      *user_argv++ = pstrdup(c->pool, *((char **) user_acl->elts));
      user_acl->elts = ((char **) user_acl->elts) + 1;
    }

  /* don't forget to NULL-terminate */
  *user_argv = NULL;

  c->flags |= CF_MERGEDOWN;

  /* done */
  return HANDLED(cmd);
}

/* This function was copied, almost verbatim, from the set_sysloglevel()
 * function in modules/mod_core.c.  I hereby cite the source for this code
 * as MacGuyver <macguyver@tos.net>. =)
 */
MODRET set_tcpaccesssysloglevels(cmd_rec *cmd) {
  config_rec *c = NULL;
  int allow_level, deny_level;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_ANON|CONF_GLOBAL);

  if (!strcasecmp(cmd->argv[1], "emerg")) {
    allow_level = PR_LOG_EMERG;

  } else if (!strcasecmp(cmd->argv[1], "alert")) {
    allow_level = PR_LOG_ALERT;

  } else if (!strcasecmp(cmd->argv[1], "crit")) {
    allow_level = PR_LOG_CRIT;

  } else if (!strcasecmp(cmd->argv[1], "error")) {
    allow_level = PR_LOG_ERR;

  } else if (!strcasecmp(cmd->argv[1], "warn")) {
    allow_level = PR_LOG_WARNING;

  } else if (!strcasecmp(cmd->argv[1], "notice")) {
    allow_level = PR_LOG_NOTICE;

  } else if (!strcasecmp(cmd->argv[1], "info")) {
    allow_level = PR_LOG_INFO;

  } else if (!strcasecmp(cmd->argv[1], "debug")) {
    allow_level = PR_LOG_DEBUG;

  } else {
    CONF_ERROR(cmd, "TCPAccessSyslogLevels requires \"allow\" level keyword: "
      "one of emerg/alert/crit/error/warn/notice/info/debug");
  }

  if (!strcasecmp(cmd->argv[2], "emerg")) {
    deny_level = PR_LOG_EMERG;

  } else if(!strcasecmp(cmd->argv[2], "alert")) {
    deny_level = PR_LOG_ALERT;

  } else if(!strcasecmp(cmd->argv[2], "crit")) {
    deny_level = PR_LOG_CRIT;

  } else if(!strcasecmp(cmd->argv[2], "error")) {
    deny_level = PR_LOG_ERR;

  } else if(!strcasecmp(cmd->argv[2], "warn")) {
    deny_level = PR_LOG_WARNING;

  } else if(!strcasecmp(cmd->argv[2], "notice")) {
    deny_level = PR_LOG_NOTICE;

  } else if(!strcasecmp(cmd->argv[2], "info")) {
    deny_level = PR_LOG_INFO;

  } else if(!strcasecmp(cmd->argv[2], "debug")) {
    deny_level = PR_LOG_DEBUG;

  } else {
    CONF_ERROR(cmd, "TCPAccessSyslogLevels requires \"deny\" level keyword: "
      "one of emerg/alert/crit/error/warn/notice/info/debug");
  }

  c = add_config_param(cmd->argv[0], 2, (void *) allow_level,
    (void *) deny_level);
  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

/* usage: TCPServiceName <name> */
MODRET set_tcpservicename(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return HANDLED(cmd);
}

/* Command handlers
 */

MODRET wrap_handle_request(cmd_rec *cmd) {

  /* these variables are names expected to be set by the TCP wrapper code
   */
  struct request_info request;

  char *user = NULL;
  config_rec *conf = NULL, *access_conf = NULL, *syslog_conf = NULL;
  hosts_allow_table = NULL;
  hosts_deny_table = NULL;

  /* hide passwords */
  session.hide_password = TRUE;

  /* Sneaky...found in mod_auth.c's cmd_pass() function.  Need to find the
   * login UID in order to resolve the possibly-login-dependent filename.
   */
  user = (char *) get_param_ptr(cmd->server->conf, C_USER, FALSE);

  /* It's possible that a PASS command came before USER.  This is a PRE_CMD
   * handler, so it won't be protected from this case; we'll need to do
   * it manually.
   */
  if (!user)
    return DECLINED(cmd);

  /* Use mod_auth's _auth_resolve_user() [imported for use here] to get the
   * right configuration set, since the user may be logging in anonymously,
   * and the session struct hasn't yet been set for that yet (thus short-
   * circuiting the easiest way to get the right context...the macros.
   */
  conf = wrap_resolve_user(cmd->pool, &user);

  /* Search first for user-specific access files.  Multiple TCPUserAccessFiles
   * directives are allowed.
   */
  if ((access_conf = find_config(conf ? conf->subset : CURRENT_CONF, CONF_PARAM,
      "TCPUserAccessFiles", FALSE)) != NULL) {
    int matched = FALSE;
    array_header *user_array = NULL;

    while (access_conf) {

      user_array = make_array(cmd->tmp_pool, 0, sizeof(char *));
      *((char **) push_array(user_array)) = pstrdup(cmd->tmp_pool, user);

      /* Check the user expression -- don't forget the offset, to skip
       * the access file name strings in argv
       */
      if (wrap_eval_expression(((char **) access_conf->argv) + 2,
          user_array)) {
        pr_log_debug(DEBUG4, MOD_WRAP_VERSION
          ": matched TCPUserAccessFiles expression");
        matched = TRUE;
        break;
      }

      access_conf = find_config_next(access_conf, access_conf->next,
        CONF_PARAM, "TCPUserAccessFiles", FALSE);
    }

    if (!matched)
      access_conf = NULL;
  }

  /* Next, search for group-specific access files.  Multiple
   * TCPGroupAccessFiles directives are allowed.
   */ 
  if (!access_conf && (access_conf = find_config(conf ? conf->subset :
        CURRENT_CONF, CONF_PARAM, "TCPGroupAccessFiles", FALSE)) != NULL) {
    unsigned char matched = FALSE;

    /* NOTE: this gid_array is only necessary until Bug#1461 is fixed */
    array_header *gid_array = make_array(cmd->pool, 0, sizeof(gid_t));

    array_header *group_array = make_array(cmd->pool, 0, sizeof(char *));

    while (access_conf) {
      if (pr_auth_getgroups(cmd->pool, user, &gid_array, &group_array) < 1) {
        pr_log_debug(DEBUG3, MOD_WRAP_VERSION
          ": no supplemental groups found for user '%s'", user);

      } else {

        /* Check the group expression -- don't forget the offset, to skip
         * the access file names strings in argv
         */
        if (wrap_eval_expression(((char **) access_conf->argv) + 2,
            group_array)) {
          pr_log_debug(DEBUG4, MOD_WRAP_VERSION
            ": matched TCPGroupAccessFiles expression");
          matched = TRUE;
          break;
        }
      }

      access_conf = find_config_next(access_conf, access_conf->next,
        CONF_PARAM, "TCPGroupAccessFiles", FALSE);
    }

    if (!matched)
      access_conf = NULL;
  }

  /* Finally for globally-applicable access files.  Only one such directive
   * is allowed.
   */
  if (!access_conf) {
    access_conf = find_config(conf ? conf->subset : CURRENT_CONF,
      CONF_PARAM, "TCPAccessFiles", FALSE);
  }

  if (access_conf) {
    hosts_allow_table = (char *) access_conf->argv[0];
    hosts_deny_table = (char *) access_conf->argv[1];
  }

  /* Now, check the retrieved filename, and see if it requires a login-time
   * file.
   */
  if (hosts_allow_table != NULL && hosts_allow_table[0] == '~' &&
      hosts_allow_table[1] == '/') {
    char *allow_real_table = NULL;

    allow_real_table = wrap_get_user_table(cmd, user, hosts_allow_table);

    if (!wrap_is_usable_file(allow_real_table)) {
      pr_log_pri(PR_LOG_WARNING, MOD_WRAP_VERSION
        ": configured TCPAllowFile %s is unusable", hosts_allow_table);
      hosts_allow_table = NULL;

    } else
      hosts_allow_table = allow_real_table;
  }

  if (hosts_deny_table != NULL && hosts_deny_table[0] == '~' &&
      hosts_deny_table[1] == '/') {
    char *deny_real_table = NULL;

    deny_real_table = dir_realpath(cmd->pool, hosts_deny_table);

    if (!wrap_is_usable_file(deny_real_table)) {
      pr_log_pri(PR_LOG_WARNING, MOD_WRAP_VERSION
        ": configured TCPDenyFile %s is unusable", hosts_deny_table);
      hosts_deny_table = NULL;

    } else 
      hosts_deny_table = deny_real_table;
  }

  /* Make sure that _both_ allow and deny TCPAccessFiles are present.
   * If not, log the missing file, and by default allow request to succeed.
   */
  if (hosts_allow_table != NULL && hosts_deny_table != NULL) {

    /* Most common case...nothing more necessary */

  } else if (hosts_allow_table == NULL && hosts_deny_table != NULL) {

    /* Log the missing file */
    pr_log_pri(PR_LOG_INFO, MOD_WRAP_VERSION ": no usable allow access file -- "
      "allowing connection");

    return DECLINED(cmd);

  } else if (hosts_allow_table != NULL && hosts_deny_table == NULL) {

    /* log the missing file */
    pr_log_pri(PR_LOG_INFO, MOD_WRAP_VERSION ": no usable deny access file -- "
      "allowing connection");

    return DECLINED(cmd);

  } else {

    /* Neither set -- assume the admin hasn't configured these directives
     * at all.
     */
    return DECLINED(cmd);
  }

  /* Log the names of the allow/deny files being used. */
  pr_log_pri(PR_LOG_DEBUG, MOD_WRAP_VERSION ": using access files: %s, %s",
    hosts_allow_table, hosts_deny_table);

  /* retrieve the user-defined syslog priorities, if any.  Fall back to the
   * defaults as seen in tcpd.h if not defined.
   */
  syslog_conf = find_config(main_server->conf, CONF_PARAM,
    "TCPAccessSyslogLevels", FALSE);

  if (syslog_conf) {
    allow_severity = (int) syslog_conf->argv[1];
    deny_severity = (int) syslog_conf->argv[2];

  } else {

    allow_severity = PR_LOG_INFO;
    deny_severity = PR_LOG_WARNING;
  }

  pr_log_debug(DEBUG4, MOD_WRAP_VERSION ": checking under service name '%s'",
    wrap_service_name);
  request_init(&request, RQ_DAEMON, wrap_service_name, RQ_FILE,
    session.c->rfd, 0);

  fromhost(&request);

  if (STR_EQ(eval_hostname(request.client), paranoid) ||
      !hosts_access(&request)) {
    char *denymsg = NULL;

    /* log the denied connection */
    wrap_log_request_denied(deny_severity, &request);

    /* check for AccessDenyMsg */
    if ((denymsg = (char *) get_param_ptr(TOPLEVEL_CONF, "AccessDenyMsg",
        FALSE)) != NULL)
      denymsg = sreplace(cmd->tmp_pool, denymsg, "%u", user, NULL);

    if (denymsg)
      return ERROR_MSG(cmd, R_530, denymsg);
    else
      return ERROR_MSG(cmd, R_530, "Access denied.");
  }

  /* If request is allowable, return DECLINED (for engine to act as if this
   * handler was never called, else ERROR (for engine to abort processing and
   * deny request.
   */
  wrap_log_request_allowed(allow_severity, &request);

  return DECLINED(cmd);
}

/* Initialization routines
 */

static int wrap_sess_init(void) {

  /* look up any configured TCPServiceName */
  if ((wrap_service_name = get_param_ptr(main_server->conf,
      "TCPServiceName", FALSE)) == NULL)
    wrap_service_name = "proftpd";

  return 0;
}

/* Module API tables
 */

static conftable wrap_conftab[] = {
  { "TCPAccessFiles",        add_tcpaccessfiles,        NULL },
  { "TCPAccessSyslogLevels", set_tcpaccesssysloglevels, NULL },
  { "TCPGroupAccessFiles",   add_tcpgroupaccessfiles,   NULL },
  { "TCPServiceName",	     set_tcpservicename,	NULL },
  { "TCPUserAccessFiles",    add_tcpuseraccessfiles,    NULL },
  { NULL }
};

static cmdtable wrap_cmdtab[] = {
  { PRE_CMD, C_PASS, G_NONE, wrap_handle_request, FALSE, FALSE },
  { 0, NULL }
};

module wrap_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "wrap",

  /* Mmodule configuration handler table */
  wrap_conftab,

  /* Module command handler table */
  wrap_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization */
  NULL,

  /* Session initialization */
  wrap_sess_init
};
