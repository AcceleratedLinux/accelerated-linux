/*
 * ProFTPD: mod_rewrite -- a module for rewriting FTP commands
 *
 * Copyright (c) 2001-2003 TJ Saunders
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
 * This is mod_rewrite, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_rewrite.c,v 1.22 2004/12/16 23:37:40 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

#define MOD_REWRITE_VERSION "mod_rewrite/0.6.8"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001021001
# error "ProFTPD 1.2.10rc1 or later required"
#endif

#ifdef HAVE_REGEX_H
# include <regex.h>

#define REWRITE_FIFO_MAXLEN		256
#define REWRITE_LOG_MODE		0640
#define REWRITE_MAX_MATCHES		10
#define REWRITE_U32_BITS		0xffffffff

/* RewriteCondition operations */
typedef enum {
  REWRITE_COND_OP_REGEX = 1,
  REWRITE_COND_OP_LEX_LT,
  REWRITE_COND_OP_LEX_GT,
  REWRITE_COND_OP_LEX_EQ,
  REWRITE_COND_OP_TEST_DIR,
  REWRITE_COND_OP_TEST_FILE,
  REWRITE_COND_OP_TEST_SYMLINK,
  REWRITE_COND_OP_TEST_SIZE
} rewrite_cond_op_t;

/* RewriteCondition flags */
#define REWRITE_COND_FLAG_NOCASE	0x001	/* nocase|NC */
#define REWRITE_COND_FLAG_ORNEXT	0x002	/* ornext|OR */

/* RewriteRule flags */
#define REWRITE_RULE_FLAG_NOCASE	0x001	/* nocase|NC */
#define REWRITE_RULE_FLAG_LAST		0x002	/* last|L */

/* Module structures */
typedef struct {
  pool *map_pool;
  char *map_name;
  char *map_lookup_key;
  char *map_default_value;
  void *map_string;
} rewrite_map_t;

typedef struct {
  char *match_string;
  regmatch_t match_groups[REWRITE_MAX_MATCHES];
} rewrite_match_t;

typedef struct {
  pool *txt_pool;
  char *txt_path;
  time_t txt_mtime;
  char **txt_keys;
  char **txt_values;
  unsigned int txt_nents; 
} rewrite_map_txt_t;

module rewrite_module;

/* Module variables */
static unsigned char rewrite_engine = FALSE;
static char *rewrite_logfile = NULL;
static int rewrite_logfd = -1;
static pool *rewrite_pool = NULL;
static pool *rewrite_cond_pool = NULL;

static unsigned int rewrite_nrules = 0;
static array_header *rewrite_conds = NULL, *rewrite_regexes = NULL;
static rewrite_match_t rewrite_cond_matches;
static rewrite_match_t rewrite_rule_matches;

#define REWRITE_MAX_VARS		14

static char rewrite_vars[REWRITE_MAX_VARS][3] = {
  "%a",		/* Remote IP address */
  "%c",		/* Session class */
  "%F",		/* Full path */
  "%f",		/* Filename */
  "%G",		/* Additional groups */
  "%g",		/* Primary group */
  "%h",		/* Remote DNS name */
  "%m",		/* FTP command (e.g. USER, RETR) */
  "%P",		/* Current PID */
  "%p",		/* Local port */
  "%t",		/* Unix time */
  "%U",		/* Original username */
  "%u",		/* Resolved/real username */
  "%v"		/* Server name */
};

/* Necessary prototypes
 */
static char *rewrite_argsep(char **);
static void rewrite_closelog(void);
static char *rewrite_expand_var(cmd_rec *, const char *, const char *);
static void rewrite_log(char *format, ...);
static unsigned char rewrite_match_cond(cmd_rec *, config_rec *);
static void rewrite_openlog(void);
static unsigned char rewrite_open_fifo(config_rec *);
static unsigned int rewrite_parse_cond_flags(pool *, const char *);
static unsigned char rewrite_parse_map_str(char *, rewrite_map_t *);
static unsigned char rewrite_parse_map_txt(rewrite_map_txt_t *);
static unsigned int rewrite_parse_rule_flags(pool *, const char *);
static int rewrite_read_fifo(int, char *, size_t);
static regex_t *rewrite_regalloc(void);
static unsigned char rewrite_regexec(const char *, regex_t *, unsigned char,
    rewrite_match_t *);
static char *rewrite_subst(cmd_rec *c, char *);
static char *rewrite_subst_backrefs(cmd_rec *, char *, rewrite_match_t *);
static char *rewrite_subst_maps(cmd_rec *, char *);
static char *rewrite_subst_maps_fifo(cmd_rec *, config_rec *, rewrite_map_t *);
static char *rewrite_subst_maps_int(cmd_rec *, config_rec *, rewrite_map_t *);
static char *rewrite_subst_maps_txt(cmd_rec *, config_rec *, rewrite_map_t *);
static char *rewrite_subst_vars(cmd_rec *, char *);
static void rewrite_wait_fifo(int);
static int rewrite_write_fifo(int, char *, size_t);

/* Support functions
 */

#define REWRITE_CHECK_VAR(p, m) \
    if (!p) rewrite_log("rewrite_expand_var(): %" m " expands to NULL")

static char *rewrite_expand_var(cmd_rec *cmd, const char *subst_pattern,
    const char *var) {
  if (!strcmp(var, "%c")) {
    REWRITE_CHECK_VAR(session.class, "%c");
    return (session.class ? session.class->cls_name : NULL);

  } else if (!strcmp(var, "%F")) {

    /* This variable is only valid for commands that operate on paths.
     * mod_log uses the session.xfer.xfer_path variable, but that is not yet
     * set at this stage in the command dispatch cycle.
     */
    if (!strcmp(cmd->argv[0], "APPE") || !strcmp(cmd->argv[0], "RETR") ||
        !strcmp(cmd->argv[0], "STOR") || !strcmp(cmd->argv[0], "DELE") ||
        !strcmp(cmd->argv[0], "MKD") || !strcmp(cmd->argv[0], "MDTM") ||
        !strcmp(cmd->argv[0], "RMD") || !strcmp(cmd->argv[0], "SIZE") ||
        !strcmp(cmd->argv[0], "STOU") || !strcmp(cmd->argv[0], "XMKD") ||
        !strcmp(cmd->argv[0], "XRMD")) {
      return dir_abs_path(cmd->tmp_pool, cmd->arg, FALSE);

    } else {
      rewrite_log("rewrite_expand_var(): %%F not valid for this command");
      return NULL;
    }

  } else if (!strcmp(var, "%f")) {
    REWRITE_CHECK_VAR(cmd->arg, "%f");
    return cmd->arg;

  } else if (!strcmp(var, "%m")) {
    return cmd->argv[0];

  } else if (!strcmp(var, "%p")) {
    char *port = pcalloc(cmd->tmp_pool, 8 * sizeof(char));
    snprintf(port, 8, "%d", main_server->ServerPort);
    port[7] = '\0';
    return port;

  } else if (!strcmp(var, "%U")) {
    return get_param_ptr(main_server->conf, C_USER, FALSE);

  } else if (!strcmp(var, "%P")) {
    char *pid = pcalloc(cmd->tmp_pool, 8 * sizeof(char));
    snprintf(pid, 8, "%u", getpid());
    pid[7] = '\0';
    return pid;

  } else if (!strcmp(var, "%g")) {
    REWRITE_CHECK_VAR(session.group, "%g");
    return session.group;

  } else if (!strcmp(var, "%u")) {
    REWRITE_CHECK_VAR(session.user, "%u");
    return session.user;

  } else if (!strcmp(var, "%a")) {
    return (char *) pr_netaddr_get_ipstr(session.c->remote_addr);

  } else if (!strcmp(var, "%h")) {
    return (char *) session.c->remote_name;

  } else if (!strcmp(var, "%v")) {
    return (char *) main_server->ServerName;

  } else if (!strcmp(var, "%G")) {

    if (session.groups) {
      register unsigned int i = 0;
      char *suppl_groups = pstrcat(cmd->tmp_pool, "", NULL);
      char **groups = (char **) session.groups->elts;

      for (i = 0; i < session.groups->nelts; i++)
        suppl_groups = pstrcat(cmd->tmp_pool, suppl_groups,
          i != 0 ? "," : "", groups[i], NULL);

      return suppl_groups;

    } else {
      REWRITE_CHECK_VAR(session.groups, "%G");
      return NULL;
    }

  } else if (!strcmp(var, "%t")) {
    char *timestr = pcalloc(cmd->tmp_pool, 80 * sizeof(char));
    snprintf(timestr, 80, "%lu", time(NULL));
    timestr[79] = '\0';
    return timestr;
  }

  rewrite_log("unknown variable: '%s'", var); 
  return NULL;
}

static char *rewrite_argsep(char **arg) {
  char *res = NULL, *dst = NULL;
  char quote_mode = 0;

  if (!arg || !*arg || !**arg)
    return NULL;

  while (**arg && isspace((int) **arg))
    (*arg)++;

  if (!**arg)
    return NULL;

  res = dst = *arg;

  if (**arg == '\"') {
    quote_mode++;
    (*arg)++;
  }

  while (**arg && **arg != ',' &&
      (quote_mode ? (**arg != '\"') : (!isspace((int) **arg)))) {

    if (**arg == '\\' && quote_mode) {

      /* escaped char */
      if (*((*arg) + 1))
        *dst = *(++(*arg));
    }

    *dst++ = **arg;
    ++(*arg);
  }

  if (**arg)
    (*arg)++;

  *dst = '\0';
  return res;
}

static unsigned int rewrite_parse_cond_flags(pool *p, const char *flags_str) {
  char *opt = NULL, *str = NULL, **opts;
  array_header *opt_list = NULL;
  unsigned int flags = 0;
  register unsigned int i = 0;

  opt_list = make_array(p, 0, sizeof(char **));

  /* Make a duplicate of the given string, as the argsep() function consumes
   * the string.
   */
  str = pstrdup(p, flags_str);

  /* Skip past the first [ in the string, and trim the last ]. */
  str++;
  str[strlen(str)-1] = '\0';

  while ((opt = rewrite_argsep(&str)) != NULL)
    *((char **) push_array(opt_list)) = pstrdup(p, opt);

  opts = opt_list->elts;
  for (i = 0; i < opt_list->nelts; i++) {
    if (strcmp(opts[i], "nocase") == 0 || strcmp(opts[i], "NC") == 0)
      flags |= REWRITE_COND_FLAG_NOCASE;
  
    else if (strcmp(opts[i], "ornext") == 0 || strcmp(opts[i], "OR") == 0)
      flags |= REWRITE_COND_FLAG_ORNEXT;
  }

  return flags;
}

static unsigned int rewrite_parse_rule_flags(pool *p, const char *flags_str) {
  char *opt = NULL, *str = NULL, **opts;
  array_header *opt_list = NULL;
  unsigned int flags = 0;
  register unsigned int i = 0;

  opt_list = make_array(p, 0, sizeof(char **));

  /* Make a duplicate of the given string, as the argsep() function consumes
   * the string.
   */
  str = pstrdup(p, flags_str);

  /* Skip past the first [ in the string, and trim the last ]. */
  str++;
  str[strlen(str)-1] = '\0';

  while ((opt = rewrite_argsep(&str)) != NULL)
    *((char **) push_array(opt_list)) = pstrdup(p, opt);

  opts = opt_list->elts;
  for (i = 0; i < opt_list->nelts; i++) {
    if (strcmp(opts[i], "nocase") == 0 || strcmp(opts[i], "NC") == 0)
      flags |= REWRITE_RULE_FLAG_NOCASE;

    else if (strcmp(opts[i], "last") == 0 || strcmp(opts[i], "L") == 0)
      flags |= REWRITE_RULE_FLAG_LAST;
  }

  return flags;
}

static unsigned char rewrite_match_cond(cmd_rec *cmd, config_rec *cond) {
  char *cond_str = cond->argv[0];
  unsigned char negated = *((unsigned char *) cond->argv[2]);
  rewrite_cond_op_t cond_op = *((rewrite_cond_op_t *) cond->argv[3]);

  rewrite_log("rewrite_match_cond(): original cond: '%s'", cond_str);

  cond_str = rewrite_subst(cmd, cond->argv[0]);
  rewrite_log("rewrite_match_cond: subst'd cond: '%s'", cond_str);

  /* Check the condition */
  switch (cond_op) {
    case REWRITE_COND_OP_LEX_LT: {
      int res = strcmp(cond_str, (char *) cond->argv[1]);
      rewrite_log("rewrite_match_cond(): checking lexical LT cond");

      if (!negated)
        return (res < 0 ? TRUE : FALSE);
      else
        return (res < 0 ? FALSE : TRUE);
    }

    case REWRITE_COND_OP_LEX_GT: {
      int res = strcmp(cond_str, (char *) cond->argv[1]);
      rewrite_log("rewrite_match_cond(): checking lexical GT cond");

      if (!negated)
        return (res > 0 ? TRUE : FALSE);
      else
        return (res > 0 ? FALSE : TRUE);
    }

    case REWRITE_COND_OP_LEX_EQ: {
      int res = strcmp(cond_str, (char *) cond->argv[1]);
      rewrite_log("rewrite_match_cond(): checking lexical EQ cond");

      if (!negated)
        return (res == 0 ? TRUE : FALSE);
      else
        return (res == 0 ? FALSE : TRUE);
    }

    case REWRITE_COND_OP_REGEX: {
      rewrite_log("rewrite_match_cond(): checking regex cond");

      memset(&rewrite_cond_matches, '\0', sizeof(rewrite_cond_matches));
      rewrite_cond_matches.match_string = cond->argv[0];
      return rewrite_regexec(cond_str, (regex_t *) cond->argv[1], negated,
        &rewrite_cond_matches);
    }

    case REWRITE_COND_OP_TEST_DIR: {
      struct stat st;
      rewrite_log("rewrite_match_cond(): checking dir test cond");

      pr_fs_clear_cache();
      if (pr_fsio_lstat(cond_str, &st) >= 0 && S_ISDIR(st.st_mode))
        return TRUE;

      return FALSE;
    }

    case REWRITE_COND_OP_TEST_FILE: {
      struct stat st;
      rewrite_log("rewrite_match_cond(): checking file test cond");

      pr_fs_clear_cache();
      if (pr_fsio_lstat(cond_str, &st) >= 0 && S_ISREG(st.st_mode))
        return TRUE;

      return FALSE;
    }

    case REWRITE_COND_OP_TEST_SYMLINK: {
      struct stat st;
      rewrite_log("rewrite_match_cond(): checking symlink test cond");

      pr_fs_clear_cache();
      if (pr_fsio_lstat(cond_str, &st) >= 0 && S_ISLNK(st.st_mode))
        return TRUE;

      return FALSE;
    }

    case REWRITE_COND_OP_TEST_SIZE: {
      struct stat st;
      rewrite_log("rewrite_match_cond(): checking size test cond");

      pr_fs_clear_cache();
      if (pr_fsio_lstat(cond_str, &st) >= 0 && S_ISREG(st.st_mode) &&
          st.st_size > 0)
        return TRUE;

      return FALSE;
    }

    default:
      rewrite_log("rewrite_match_cond(): unknown cond op: %d", cond_op);
      break;
  }

  return FALSE;
}

static unsigned char rewrite_parse_map_str(char *str, rewrite_map_t *map) {
  static char *substr = NULL;
  char *tmp = NULL;

  /* A NULL string is used to set/reset this function. */
  if (!str) {
    substr = NULL;
    return FALSE;
  }

  if (!substr)
    substr = str;

  /* Format: ${map-name:lookup-key[|default-value]} */
  rewrite_log("rewrite_parse_map_str(): parsing '%s'", substr);
  if (substr && (tmp = strstr(substr, "${")) != NULL) {
    char twiddle;
    char *map_start = tmp + 2;
    char *map_end = strchr(map_start, '}');

    if (!map_end) {
      rewrite_log("rewrite_parse_mapstr(): error: badly formatted map string");
      return FALSE;
    }

    /* This fiddling is needed to preserve a copy of the complete map string. */
    twiddle = map_end[1];
    map_end[1] = '\0';
    map->map_string = pstrdup(map->map_pool, tmp);
    map_end[1] = twiddle;

    /* OK, now back to our regular schedule parsing... */
    *map_end = '\0';

    if ((tmp = strchr(map_start, ':')) == NULL) {
      rewrite_log("rewrite_parse_mapstr(): notice: badly formatted map string");
      return FALSE;
    }
    *tmp = '\0';

    /* We've teased out the map name. */
    map->map_name = map_start;

    /* Advance the pointer so that the rest of the components can be parsed. */
    map_start = ++tmp;

    map->map_lookup_key = map_start;

    if ((tmp = strchr(map_start, '|')) != NULL) {
      *tmp = '\0';

      /* We've got the default value. */
      map->map_default_value = ++tmp;

    } else
      map->map_default_value = "";

    substr = ++map_end;
    return TRUE;
  }
  
  return FALSE;
}

static unsigned char rewrite_parse_map_txt(rewrite_map_txt_t *txtmap) {
  struct stat st;
  pool *tmp_pool = NULL;
  char *linebuf = NULL;
  array_header *keys = NULL, *vals = NULL;
  unsigned int lineno = 0, i = 0;
  pr_fh_t *ftxt = NULL;

  /* Make sure the file exists. */
  if (pr_fsio_stat(txtmap->txt_path, &st) < 0) {
    rewrite_log("rewrite_parse_map_txt(): unable to stat %s: %s",
      txtmap->txt_path, strerror(errno));
    return FALSE;
  }

  /* Compare the modification time of the file against what's cached.  Unless
   * the file is newer, do not parse it in again.
   */
  if (st.st_mtime <= txtmap->txt_mtime) {
    rewrite_log("rewrite_parse_map_txt(): cached map cache up to date");
    return TRUE;
  }

  /* Open the file. */
  if ((ftxt = pr_fsio_open(txtmap->txt_path, O_RDONLY)) == NULL) {
    rewrite_log("rewrite_parse_map_txt(): unable to open %s: %s",
      txtmap->txt_path, strerror(errno));
    return FALSE;
  }
  txtmap->txt_mtime = st.st_mtime;

  tmp_pool = make_sub_pool(txtmap->txt_pool);
  linebuf = pcalloc(tmp_pool, PR_TUNABLE_BUFFER_SIZE * sizeof(char));
  keys = make_array(tmp_pool, 0, sizeof(char *));
  vals = make_array(tmp_pool, 0, sizeof(char *));

  while (pr_fsio_getline(linebuf, PR_TUNABLE_BUFFER_SIZE, ftxt, &i)) {
    register unsigned int pos = 0;
    size_t linelen = strlen(linebuf);
    unsigned int key_so = 0, key_eo = 0;
    unsigned int val_so = 0, val_eo = 0;

    pr_signals_handle();

    /* Skip leading whitespace. */
    for (pos = 0; pos < linelen && isspace(linebuf[pos]); pos++);

    /* Ignore comments and blank lines. */
    if (linebuf[pos] == '#')
      continue;

    if (pos == linelen)
      continue; 

    /* Only parse the first two non-whitespace strings.  Ignore everything
     * else.
     */
    key_so = pos;
    for (; pos < linelen; pos++) {
 
      if (isspace(linebuf[pos])) {
        if (!key_eo)
          key_eo = pos;

        else if (val_so && !val_eo) {
          val_eo = pos;
          break;
        }

      } else {
        if (key_eo && !val_so)
          val_so = pos;
      }
    }

    if (key_eo && val_eo) {
      linebuf[key_eo] = '\0';
      *((char **) push_array(keys)) = pstrdup(txtmap->txt_pool,
        &linebuf[key_so]);

      linebuf[val_eo] = '\0';
      *((char **) push_array(vals)) = pstrdup(txtmap->txt_pool,
        &linebuf[val_so]);

    } else {
      rewrite_log("rewrite_parse_map_txt(): error: %s, line %d",
        txtmap->txt_path, lineno);
      rewrite_log("rewrite_parse_map_txt(): bad line: '%s'", linebuf);
    }
  }

  txtmap->txt_keys = (char **) pcalloc(txtmap->txt_pool,
    keys->nelts * sizeof(char *));
  for (i = 0; i < keys->nelts; i++)
    txtmap->txt_keys[i] = ((char **) keys->elts)[i];

  txtmap->txt_values = (char **) pcalloc(txtmap->txt_pool,
    vals->nelts * sizeof(char *));
  for (i = 0; i < vals->nelts; i++)
    txtmap->txt_values[i] = ((char **) vals->elts)[i];

  txtmap->txt_nents = vals->nelts;

  destroy_pool(tmp_pool);
  pr_fsio_close(ftxt);
  return TRUE;
}

static regex_t *rewrite_regalloc(void) {
  regex_t *preg = NULL;

  /* If no regex-tracking array has been allocated, create one.  Register
   * a cleanup handler for this pool, to call regfree(3) on all
   * regex_t pointers in the array.
   */
  if (!rewrite_regexes)
    rewrite_regexes = make_array(rewrite_pool, 0, sizeof(regex_t *));

  if ((preg = calloc(1, sizeof(regex_t))) == NULL) {
    rewrite_log("fatal: memory exhausted during regex_t allocation");
    exit(1);
  }

  /* Add the regex_t pointer to the array. */
  *((regex_t **) push_array(rewrite_regexes)) = preg;

  return preg;
}

static unsigned char rewrite_regexec(const char *string, regex_t *regbuf,
    unsigned char negated, rewrite_match_t *matches) {
  int res = -1;
  char *tmpstr = (char *) string;
  unsigned char have_match = FALSE;

  /* Sanity checks */
  if (!string || !regbuf)
    return FALSE;

  /* Prepare the given match group array. */
  memset(matches->match_groups, '\0', sizeof(regmatch_t) * REWRITE_MAX_MATCHES);

  /* Execute the given regex. */
  while ((res = regexec(regbuf, tmpstr, REWRITE_MAX_MATCHES,
      matches->match_groups, 0)) == 0) {

    have_match = TRUE;
    break;

#if 0
    /* If negated, be done */
    if (negated)
      break;
#endif
  }

  /* Invert the return value if necessary. */
  if (negated)
    have_match = !have_match;

  return have_match;
}

static char *rewrite_subst(cmd_rec *cmd, char *pattern) {
  char *new_pattern = NULL;
  rewrite_log("rewrite_subst(): original pattern: '%s'", pattern);

  /* Expand any RewriteRule backreferences in the substitution pattern. */
  new_pattern = rewrite_subst_backrefs(cmd, pattern, &rewrite_rule_matches);
  rewrite_log("rewrite_subst(): rule backref subst'd pattern: '%s'",
    new_pattern);

  /* Expand any RewriteCondition backreferences in the substitution pattern. */
  new_pattern = rewrite_subst_backrefs(cmd, new_pattern, &rewrite_cond_matches);
  rewrite_log("rewrite_subst(): cond backref subst'd pattern: '%s'",
    new_pattern);

  /* Next, rewrite the arg, substituting in the values. */
  new_pattern = rewrite_subst_vars(cmd, new_pattern);
  rewrite_log("rewrite_subst(): var subst'd pattern: '%s'", new_pattern);

  /* Now, perform any map substitutions in the pattern. */
  new_pattern = rewrite_subst_maps(cmd, new_pattern);
  rewrite_log("rewrite_subst(): maps subst'd pattern: '%s'", new_pattern);

  return new_pattern;
}

static char *rewrite_subst_backrefs(cmd_rec *cmd, char *pattern,
    rewrite_match_t *matches) {
  register unsigned int i = 0;
  char *new_pattern = NULL;

  for (i = 0; i < REWRITE_MAX_MATCHES; i++) {
    if (matches->match_groups[i].rm_so != -1) {
      char buf[3] = {'\0'}, tmp;

      if (matches == &rewrite_rule_matches)
        /* Substitute "$N" backreferences for RewriteRule matches */
        snprintf(buf, sizeof(buf), "$%u", i);

      else if (matches == &rewrite_cond_matches)
        /* Substitute "%N" backreferences for RewriteCondition matches */
        snprintf(buf, sizeof(buf), "%%%u", i);

      if (!new_pattern)
        new_pattern = pstrdup(cmd->pool, pattern);

      /* Make sure there's a backreference for this in the substitution
       * pattern.  Otherwise, just continue on.
       */
      if (!strstr(new_pattern, buf))
        continue;

      tmp = (matches->match_string)[matches->match_groups[i].rm_eo];
      (matches->match_string)[matches->match_groups[i].rm_eo] = '\0';

      rewrite_log("rewrite_subst_backrefs(): replacing backref '%s' with '%s'",
        buf, &(matches->match_string)[matches->match_groups[i].rm_so]);
      new_pattern = sreplace(cmd->pool, new_pattern, buf,
        &(matches->match_string)[matches->match_groups[i].rm_so], NULL);
   
      /* Undo the twiddling of the NUL character. */ 
      (matches->match_string)[matches->match_groups[i].rm_eo] = tmp;
    }
  }

  return (new_pattern ? new_pattern : pattern);
}

static char *rewrite_subst_maps(cmd_rec *cmd, char *pattern) {
  rewrite_map_t map;
  char *tmp_pattern = pstrdup(cmd->pool, pattern), *new_pattern = NULL;

  map.map_pool = cmd->tmp_pool;

  while (rewrite_parse_map_str(tmp_pattern, &map)) {
    config_rec *c = NULL;
    unsigned char have_map = FALSE;

    rewrite_log("rewrite_subst_maps(): map name: '%s'",
      map.map_name);
    rewrite_log("rewrite_subst_maps(): lookup key: '%s'",
      map.map_lookup_key);
    rewrite_log("rewrite_subst_maps(): default value: '%s'",
      map.map_default_value);

    /* Check the configured maps for this server, to see if the given map
     * name is actually valid.
     */
    c = find_config(main_server->conf, CONF_PARAM, "RewriteMap", FALSE);

    while (c) {

      if (!strcmp(c->argv[0], map.map_name)) { 
        char *lookup_value = NULL;
        have_map = TRUE;

        rewrite_log("rewrite_subst_maps(): mapping '%s' using '%s'",
          map.map_lookup_key, map.map_name);

        /* Handle FIFO maps */
        if (!strcmp(c->argv[1], "fifo")) {
          lookup_value = rewrite_subst_maps_fifo(cmd, c, &map);
          rewrite_log("rewrite_subst_maps(): fifo map '%s' returned '%s'",
            map.map_name, lookup_value);

        /* Handle maps of internal functions */
        } else if (!strcmp(c->argv[1], "int")) {
          lookup_value = rewrite_subst_maps_int(cmd, c, &map);
          rewrite_log("rewrite_subst_maps(): internal map '%s' returned '%s'",
            map.map_name, lookup_value);

        /* Handle external file maps */
        } else if (!strcmp(c->argv[1], "txt")) {
          lookup_value = rewrite_subst_maps_txt(cmd, c, &map);
          rewrite_log("rewrite_subst_maps(): txt map '%s' returned '%s'",
            map.map_name, lookup_value);
        }

        /* Substitute the looked-up value into the substitution pattern,
         * if indeed a map (and value) have been found.
         */
        rewrite_log("rewrite_subst_maps(): substituting '%s' for '%s'",
          lookup_value, map.map_string);

        if (!new_pattern)
          new_pattern = pstrdup(cmd->pool, pattern);

        new_pattern = sreplace(cmd->pool, new_pattern, map.map_string,
          lookup_value, NULL);
      }

      c = find_config_next(c, c->next, CONF_PARAM, "RewriteMap", FALSE);
    }

    if (!have_map)
      rewrite_log("rewrite_subst_maps(): warning: no such RewriteMap '%s'",
        map.map_name);
  }

  /* Don't forget to reset the parsing function when done. */
  rewrite_parse_map_str(NULL, NULL);

  return (new_pattern ? new_pattern : pattern);
}

static char *rewrite_subst_maps_fifo(cmd_rec *cmd, config_rec *c,
    rewrite_map_t *map) {
  int fifo_fd = -1, fifo_lockfd = -1, res;
  char *value = NULL, *fifo_lockname = NULL;
  const char *fifo = (char *) c->argv[2];

#ifndef HAVE_FLOCK
  struct flock lock;
#endif /* HAVE_FLOCK */

  /* The FIFO file descriptor should already be open. */
  fifo_fd = *((int *) c->argv[3]);

  if (fifo_fd == -1) {
    rewrite_log("rewrite_subst_maps_fifo(): missing necessary FIFO file "
      "descriptor");
    return map->map_default_value;
  }

  /* No interruptions, please. */
  pr_signals_block();

  /* See if a RewriteLock has been configured. */
  if ((fifo_lockname = (char *) get_param_ptr(main_server->conf, "RewriteLock",
      FALSE)) != NULL) {

    /* Make sure the file exists. */
    if ((fifo_lockfd = open(fifo_lockname, O_CREAT)) < 0)
      rewrite_log("rewrite_subst_maps_fifo(): error creating '%s': %s",
        fifo_lockname, strerror(errno));
  }

  /* Obtain a write lock on the lock file, if configured */
  if (fifo_lockfd != -1) {
#ifdef HAVE_FLOCK
    if (flock(fifo_lockfd, LOCK_EX) < 0)
      rewrite_log("rewrite_subst_maps_fifo(): error obtaining lock: %s",
        strerror(errno));
#else
    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = lock.l_len = 0;

    while (fcntl(fifo_lockfd, F_SETLKW, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;
      }

      rewrite_log("rewrite_subst_maps_fifo(): error obtaining lock: %s",
        strerror(errno));
      break;
    }
#endif /* HAVE_FLOCK */
  }

  /* Write the lookup key to the FIFO.  There is no need to restrict the size
   * of data written to the FIFO to the PIPE_BUF length; the advisory lock on
   * the FIFO is used to coordinate I/O on the FIFO, which means that only one
   * child process will be talking to the FIFO at a given time, obviating the
   * possibility of interleaved reads/writes.
   *
   * Note that at present (1.2.6rc1), proftpd ignores SIGPIPE.  For the most
   * part this is not a concern.  However, when dealing with FIFOs, it can be:
   * a process that attempts to write to a FIFO that is not opened for reading
   * on the other side will receive a SIGPIPE from the kernel.  The above
   * open()s try to prevent this case, but it can happen that the FIFO-reading
   * process might end after the writing file descriptor has been opened.
   * Hmmm.
   */

  pr_signals_unblock();
  if (rewrite_write_fifo(fifo_fd,
      pstrcat(cmd->tmp_pool, map->map_lookup_key, "\n", NULL),
      strlen(map->map_lookup_key) + 1) != strlen(map->map_lookup_key) + 1) {

    rewrite_log("rewrite_subst_maps_fifo(): error writing lookup key '%s' to "
      "FIFO '%s': %s", map->map_lookup_key, fifo, strerror(errno));

    if (fifo_lockfd != -1) {
#ifdef HAVE_FLOCK
      if (flock(fifo_lockfd, LOCK_UN) < 0)
        rewrite_log("rewrite_subst_maps_fifo(): error releasing lock: %s",
          strerror(errno));
#else
      lock.l_type = F_UNLCK;
      lock.l_whence = 0;
      lock.l_start = lock.l_len = 0;

      while (fcntl(fifo_lockfd, F_SETLKW, &lock) < 0) {
        if (errno == EINTR) {
          pr_signals_handle();
          continue;
        }

        rewrite_log("rewrite_subst_maps_fifo(): error releasing lock: %s",
          strerror(errno));
        break;
      }
#endif /* HAVE_FLOCK */

      close(fifo_lockfd);
    }

    /* Return the default value */
    return map->map_default_value;
  }
  pr_signals_block();

  /* Make sure the data in the write buffer has been flushed into the FIFO. */
  fsync(fifo_fd);

  /* Allocate memory into which to read the lookup value. */
  value = pcalloc(cmd->pool, sizeof(char) * REWRITE_FIFO_MAXLEN);

  /* Read the value from the FIFO, if any. Unblock signals before doing so. */
  pr_signals_unblock();
  if ((res = rewrite_read_fifo(fifo_fd, value, REWRITE_FIFO_MAXLEN)) <= 0) {
    
    if (res < 0)
      rewrite_log("rewrite_subst_maps_fifo(): error reading value from FIFO "
        "'%s': %s", fifo, strerror(errno));

    /* Use the default value */
    value = map->map_default_value;

  } else {
    register unsigned int i = 0;

    /* Find the terminating newline in the returned value */
    for (i = 0; i < REWRITE_FIFO_MAXLEN; i++) {
      if (value[i] == '\n') {
        value[i] = '\0';
        break;
      }
    }

    if (i == REWRITE_FIFO_MAXLEN) {
      rewrite_log("rewrite_subst_maps_fifo(): FIFO returned too long value, "
        "using default value");
      value = map->map_default_value;
    }
  }
  pr_signals_block();

  rewrite_wait_fifo(fifo_fd);

  /* Make sure the data from the read buffer is completely flushed. */
  fsync(fifo_fd);

  if (fifo_lockfd != -1) {
#ifdef HAVE_FLOCK
    if (flock(fifo_lockfd, LOCK_UN) < 0)
      rewrite_log("rewrite_subst_maps_fifo(): error releasing lock: %s",
        strerror(errno));
#else
    lock.l_type = F_UNLCK;
    lock.l_whence = 0;
    lock.l_start = lock.l_len = 0;

    while (fcntl(fifo_lockfd, F_SETLKW, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;
      }

      rewrite_log("rewrite_subst_maps_fifo(): error releasing lock: %s",
        strerror(errno));
      break;
    }
#endif /* HAVE_FLOCK */

    close(fifo_lockfd);
  }

  pr_signals_unblock();

  return value;
}

static char *rewrite_subst_maps_int(cmd_rec *cmd, config_rec *c,
    rewrite_map_t *map) {
  char *value = NULL;
  char *(*map_func)(pool *, char *) = c->argv[2];
   
  if ((value = map_func(cmd->tmp_pool, map->map_lookup_key)) == NULL)
    value = map->map_default_value;

  return value;
}

static char *rewrite_subst_maps_txt(cmd_rec *cmd, config_rec *c,
    rewrite_map_t *map) {
  rewrite_map_txt_t *txtmap = c->argv[2];
  char **txt_keys = NULL, **txt_vals = NULL, *value = NULL;
  register unsigned int i = 0;

  /* Make sure this map is up-to-date. */
  if (!rewrite_parse_map_txt(txtmap))
    rewrite_log("rewrite_subst_maps_txt(): error parsing txt file");

  txt_keys = (char **) txtmap->txt_keys;
  txt_vals = (char **) txtmap->txt_values;

  for (i = 0; i < txtmap->txt_nents; i++)
    if (!strcmp(txtmap->txt_keys[i], map->map_lookup_key))
      value = txtmap->txt_values[i];

  if (!value)
    value = map->map_default_value;

  return value;
}

static char *rewrite_subst_vars(cmd_rec *cmd, char *pattern) {
  register unsigned int i = 0;
  char *new_pattern = NULL;

  for (i = 0; i < REWRITE_MAX_VARS; i++) {
    char *val = NULL;

    /* Does this variable occur in the substitution pattern? */
    if (!strstr(pattern, rewrite_vars[i]))
      continue;

    if ((val = rewrite_expand_var(cmd, pattern, rewrite_vars[i])) != NULL) {
      rewrite_log("rewrite_subst_vars(): replacing variable '%s' with '%s'",
        rewrite_vars[i], val);
      if (!new_pattern)
        new_pattern = pstrdup(cmd->pool, pattern);

      new_pattern = sreplace(cmd->pool, new_pattern, rewrite_vars[i], val,
        NULL);
    }
  }

  return (new_pattern ? new_pattern : pattern);
}

static char rewrite_hex_to_char(const char *what) {
  register char digit;

  /* NOTE: this assumes a non-EBCDIC system... */
  digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));

  return digit;
}

#define REWRITE_VALID_UTF8_BYTE(b) (((b) & 0x80) ? TRUE : FALSE)

/* Converts a UTF8 encoded string to a UCS4 encoded string (which just
 * happens to coincide with ISO-8859-1).  On success, the length of the
 * populated long array (which must be allocated by the caller) will be
 * returned.  -1 will be returned on error, signalling an incorrectly
 * formatted UTF8 string.  The majority of this code is contained
 * in RFC 2640 "Internationalization of the File Transfer Protocol".
 */
static int rewrite_utf8_to_ucs4(unsigned long *ucs4_buf,
    size_t utf8_len, unsigned char *utf8_buf) {
  const unsigned char *utf8_endbuf = utf8_buf + utf8_len;
  int ucs_len = 0;

  while (utf8_buf != utf8_endbuf) {

    /* ASCII chars - no conversion needed */
    if ((*utf8_buf & 0x80) == 0x00) {
      *ucs4_buf++ = (unsigned long) *utf8_buf;
      utf8_buf++;
      ucs_len++;

    /* In the 2-byte UTF-8 range... */
    } else if ((*utf8_buf & 0xE0)== 0xC0) {

      /* Make sure the next byte is a valid UTF8 byte. */
      if (!REWRITE_VALID_UTF8_BYTE(*(utf8_buf + 1)))
        return -1;

      *ucs4_buf++ = (unsigned long) (((*utf8_buf - 0xC0) * 0x40)
                    + (*(utf8_buf+1) - 0x80));
      utf8_buf += 2;
      ucs_len++;

    /* In the 3-byte UTF-8 range... */
    } else if ((*utf8_buf & 0xF0) == 0xE0) {
      register unsigned int i;

      /* Make sure the next 2 bytes are valid UTF8 bytes. */
      for (i = 1; i <= 2; i++)
        if (!REWRITE_VALID_UTF8_BYTE(*(utf8_buf + i)))
          return -1;

      *ucs4_buf++ = (unsigned long) (((*utf8_buf - 0xE0) * 0x1000)
                  + ((*(utf8_buf+1) - 0x80) * 0x40)
                  + (*(utf8_buf+2) - 0x80));
      utf8_buf+=3;
      ucs_len++;

    /* In the 4-byte UTF-8 range... */
    } else if ((*utf8_buf & 0xF8) == 0xF0) {
      register unsigned int i;

      /* Make sure the next 3 bytes are valid UTF8 bytes. */
      for (i = 1; i <= 3; i++)
        if (!REWRITE_VALID_UTF8_BYTE(*(utf8_buf + i)))
          return -1;

      *ucs4_buf++ = (unsigned long)
                   (((*utf8_buf - 0xF0) * 0x040000)
                 + ((*(utf8_buf+1) - 0x80) * 0x1000)
                 + ((*(utf8_buf+2) - 0x80) * 0x40)
                 + (*(utf8_buf+3) - 0x80));
      utf8_buf+=4;
      ucs_len++;

    /* In the 5-byte UTF-8 range... */
    } else if ((*utf8_buf & 0xFC) == 0xF8) {
      register unsigned int i;

      /* Make sure the next 4 bytes are valid UTF8 bytes. */
      for (i = 1; i <= 4; i++)
        if (!REWRITE_VALID_UTF8_BYTE(*(utf8_buf + i)))
          return -1;

      *ucs4_buf++ = (unsigned long)
                    (((*utf8_buf - 0xF8) * 0x01000000)
                  + ((*(utf8_buf+1) - 0x80) * 0x040000)
                  + ((*(utf8_buf+2) - 0x80) * 0x1000)
                  + ((*(utf8_buf+3) - 0x80) * 0x40)
                  + (*(utf8_buf+4) - 0x80));
       utf8_buf+=5;
       ucs_len++;

    /* In the 6-byte UTF-8 range... */
    } else if ((*utf8_buf & 0xFE) == 0xFC) {
      register unsigned int i;

      /* make sure the next 5 bytes are valid UTF8 bytes */
      for (i = 1; i <= 5; i++)
        if (!REWRITE_VALID_UTF8_BYTE(*(utf8_buf + i)))
          return -1;

      *ucs4_buf++ = (unsigned long)
                    (((*utf8_buf - 0xFC) * 0x40000000)
                  + ((*(utf8_buf+1) - 0x80) * 0x010000000)
                  + ((*(utf8_buf+2) - 0x80) * 0x040000)
                  + ((*(utf8_buf+3) - 0x80) * 0x1000)
                  + ((*(utf8_buf+4) - 0x80) * 0x40)
                  + (*(utf8_buf+5) - 0x80));
      utf8_buf+=6;
      ucs_len++;

    /* Badly formatted UTF8 string, with escape sequences that are interpreted
     * ambiguously.  If this is the case, just copy the non-ASCII, non-UTF8
     * char into the UCS4 buffer, and assume the string creator knew what they
     * were doing...
     */
    } else {
      *ucs4_buf++ = (unsigned long) *utf8_buf;
      utf8_buf++;
      ucs_len++;
    }
  }

  return ucs_len;
}

/* RewriteMap internal functions.  Note that these functions may (and
 * probably will) modify their key arguments.
 */

static char *rewrite_map_int_replaceall(pool *map_pool, char *key) {
  char sep = *key;
  char *value = NULL, *src = NULL, *dst = NULL;
  char *tmp = NULL;

  /* Due to the way in which this internal function works, the first
   * character of the given key is used as a delimiter separating
   * the given key, and the sequences to replace for this function.
   */
  char *str = pstrdup(map_pool, key + 1);
  
  tmp = strchr(str , sep);
  if (tmp == NULL) {
    rewrite_log("rewrite_map_int_replace(): badly formatted input key");
    return NULL;
  }

  *tmp = '\0';
  value = str;
  rewrite_log("rewrite_map_int_replace(): actual key: '%s'", value); 
 
  str = tmp + 1;

  tmp = strchr(str, sep);
  if (tmp == NULL) {
    rewrite_log("rewrite_map_int_replace(): badly formatted input key");
    return NULL;
  }

  *tmp = '\0';
  src = str;
  dst = tmp + 1;
  
  rewrite_log("rewrite_map_int_replace(): replacing '%s' with '%s'", src,
    dst);

  /* Make sure the source sequence is present in the given key. */
  if (strstr(value, src) == NULL) {
    rewrite_log("rewrite_map_int_replace(): '%s' does not occur in given "
      "key '%s'", src, value);
    return NULL;
  }

  return sreplace(map_pool, value, src, dst, NULL);
}

static char *rewrite_map_int_tolower(pool *map_pool, char *key) {
  register unsigned int i = 0;
  char *value = pstrdup(map_pool, key);
  size_t valuelen = strlen(value);

  for (i = 0; i < valuelen; i++)
    value[i] = tolower(value[i]);

  return value;
}

static char *rewrite_map_int_toupper(pool *map_pool, char *key) {
  register unsigned int i = 0;
  char *value = pstrdup(map_pool, key);
  size_t valuelen = strlen(value);

  for (i = 0; i < valuelen; i++)
    value[i] = toupper(value[i]);

  return value;
}

#define REWRITE_IS_XDIGIT(c) (isxdigit(((unsigned char)(c))))

/* Unescapes the hex escape sequences in the given string (typically a URL-like
 * path).  Returns the escaped string on success, NULL on error; failures can
 * be caused by: bad % escape sequences, decoding %00, or a special character.
 */
static char *rewrite_map_int_unescape(pool *map_pool, char *key) {
  register int i, j;
  char *value = pcalloc(map_pool, sizeof(char) * strlen(key));

  for (i = 0, j = 0; key[j]; ++i, ++j) {
    if (key[j] != '%')
      value[i] = key[j];

    else {

      if (!REWRITE_IS_XDIGIT(key[j+1]) || !REWRITE_IS_XDIGIT(key[j+2])) {
        rewrite_log("rewrite_map_int_unescape(): bad escape sequence '%c%c%c'",
          key[j], key[j+1], key[j+2]);
        return NULL;

      } else {

        value[i] = rewrite_hex_to_char(&key[j+1]);
        j += 2;
        if (key[i] == '/' || key[i] == '\0') {
          rewrite_log("rewrite_map_int_unescape(): bad path");
          return NULL;
        }
      }
    }
  }
  value[i] = '\0';

  return value;
}

static unsigned char rewrite_open_fifo(config_rec *c) {
  int fd = -1;
  char *fifo = c->argv[2];

  /* No interruptions, please. */
  pr_signals_block();

  if ((fd = open(fifo, O_RDWR|O_NONBLOCK)) < 0) {
    rewrite_log("rewrite_rdopen_fifo(): unable to open FIFO '%s': %s", fifo,
      strerror(errno));
    pr_signals_unblock();
    return FALSE;

  } else {
    int flags = -1;

    /* Set this descriptor for blocking. */
    flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags & (REWRITE_U32_BITS^O_NONBLOCK)) < 0)
      rewrite_log("rewrite_open_fifo(): error setting FIFO "
        "blocking mode: %s", strerror(errno));
  }

  /* Add the file descriptor into the config_rec. */
  *((int *) c->argv[3]) = fd;

  return TRUE;
}

static int rewrite_read_fifo(int fd, char *buf, size_t buflen) {
  int res = 0;
  fd_set rset;

  FD_ZERO(&rset);
  FD_SET(fd, &rset);

  /* Blocking select for reading, handling interruptions appropriately. */
  while ((res = select(fd + 1, &rset, NULL, NULL, NULL)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return res;
  }

  /* Now, read from the FIFO, again handling interruptions. */
  while ((res = read(fd, buf, buflen)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    } 

    break;
  }

  return res;
}

static void rewrite_wait_fifo(int fd) {
  int size = 0;
  struct timeval tv;

  /* Wait for the data written to the FIFO buffer to be read.  Use
   * ioctl(2) (yuck) to poll the buffer, waiting for the number of bytes
   * to be read to drop to zero.  When that happens, we'll know that the
   * process on the other end of the FIFO has read the data, and has
   * hopefully written a response back.  We select(2) when reading from
   * the FIFO, so we won't need to poll the buffer similarly there.
   */

  if (ioctl(fd, FIONREAD, &size) < 0)
    rewrite_log("rewrite_wait_fifo(): ioctl error: %s", strerror(errno));

  while (size != 0) {
    rewrite_log("rewrite_wait_fifo(): waiting for buffer to be read");

    /* Handling signals is always a Good Thing in a while() loop. */
    pr_signals_handle();

    /* Poll every half second. */
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
 
    select(0, NULL, NULL, NULL, &tv);

    if (ioctl(fd, FIONREAD, &size) < 0)
      rewrite_log("rewrite_wait_fifo(): ioctl error: %s", strerror(errno));
  }
}

static int rewrite_write_fifo(int fd, char *buf, size_t buflen) {
  int res = 0;

  while ((res = write(fd, buf, buflen)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    break;
  }

  return res;
}

static char *rewrite_map_int_utf8trans(pool *map_pool, char *key) {
  int ucs4strlen = 0;
  static unsigned char utf8_val[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  static unsigned long ucs4_longs[PR_TUNABLE_BUFFER_SIZE] = {0L};

  /* If the key is NULL or empty, do nothing. */
  if (!key || !strlen(key))
    return NULL;

  /* Always make sure the buffers are clear for this run. */
  memset(utf8_val, '\0', PR_TUNABLE_BUFFER_SIZE);
  memset(ucs4_longs, 0, PR_TUNABLE_BUFFER_SIZE);

  if ((ucs4strlen = rewrite_utf8_to_ucs4(ucs4_longs, strlen(key), key)) < 0) {

    /* The key is not a properly formatted UTF-8 string. */
    rewrite_log("rewrite_map_int_utf8trans(): not a proper UTF-8 string: '%s'",
      key);  
    return NULL;

  } else if (ucs4strlen > 1) {
    register unsigned int i = 0;

    /* Cast the UTF-8 longs to unsigned chars.  NOTE: this is an assumption
     * about casts; it just so happens, quite nicely, that UCS4 maps one-to-one
     * to ISO-8859-1 (Latin-1).
     */
    for (i = 0; i < ucs4strlen; i++)
      utf8_val[i] = (unsigned char) ucs4_longs[i];

    return pstrdup(map_pool, utf8_val);
  }

  return NULL;
}

/* Rewrite logging functions */

static void rewrite_openlog(void) {
  int res = 0;

  /* Sanity checks */
  if (rewrite_logfd >= 0)
    return;

  if ((rewrite_logfile = get_param_ptr(main_server->conf, "RewriteLog",
      FALSE)) == NULL) {
    rewrite_logfd = -2;
    return;
  }

  if (!strcasecmp(rewrite_logfile, "none")) {
    rewrite_logfd = -1;
    rewrite_logfile = NULL;
    return;
  }

  pr_signals_block();
  PRIVS_ROOT
  res = pr_log_openfile(rewrite_logfile, &rewrite_logfd, REWRITE_LOG_MODE);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (res < 0)
    pr_log_pri(PR_LOG_NOTICE, MOD_REWRITE_VERSION
      ": error: unable to open log file '%s': %s", rewrite_logfile,
      strerror(errno));

  return;
}

static void rewrite_closelog(void) {

  /* Sanity check */
  if (rewrite_logfd < 0)
    return;

  if (close(rewrite_logfd) < 0) {
    pr_log_pri(PR_LOG_ERR, MOD_REWRITE_VERSION
      ": error closing RewriteLog '%s': %s", rewrite_logfile, strerror(errno));
    return;
  }

  rewrite_logfile = NULL;
  rewrite_logfd = -1;

  return;
}

static void rewrite_log(char *format, ...) {
  char mesgbuf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  char prefix[80] = {'\0'};
  char entry[1156] = {'\0'};
  time_t logtime = time(NULL);
  struct tm *timestamp = localtime(&logtime);
  va_list mesg;

  /* Format the log message */
  va_start(mesg, format);
  vsnprintf(mesgbuf, sizeof(mesgbuf), format, mesg);
  va_end(mesg);
  mesgbuf[sizeof(mesgbuf)-1] = '\0';

  /* Format a message prefix */
  strftime(prefix, sizeof(prefix), "%b %d %H:%M:%S:", timestamp);
  snprintf(prefix + strlen(prefix), sizeof(prefix) - strlen(prefix),
    " " MOD_REWRITE_VERSION ":");

  prefix[sizeof(prefix)-1] = '\0';

  snprintf(entry, sizeof(entry), "%s %s\n", prefix, mesgbuf);
  entry[sizeof(entry)-1] = '\0';

  if (rewrite_logfd > 0) {
    if (write(rewrite_logfd, entry, strlen(entry)) < strlen(entry))
      pr_log_pri(PR_LOG_ERR, "error writing to RewriteLog '%s': %s",
        rewrite_logfile, strerror(errno));

  } else
    pr_log_debug(DEBUG3, MOD_REWRITE_VERSION ": %s", mesgbuf);

  return;
}

/* Configuration directive handlers
 */

/* usage: RewriteCondition condition pattern [flags] */
MODRET set_rewritecondition(cmd_rec *cmd) {
  config_rec *c = NULL;
  pool *cond_pool = NULL;
  void *cond_data = NULL;
  unsigned int cond_flags = 0;
  char *var = NULL;
  unsigned char negated = FALSE;
  rewrite_cond_op_t cond_op = 0;
  int regex_flags = REG_EXTENDED, res = -1;

  if (cmd->argc-1 < 2 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "bad number of parameters");
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  /* The following variables are not allowed in RewriteConditions:
   *  %P (PID), and %t (Unix epoch).  Check for them.
   */
  if (strstr(cmd->argv[2], "%P") || strstr(cmd->argv[2], "%t"))
    CONF_ERROR(cmd, "illegal RewriteCondition variable used");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  /* Make sure that, if present, the flags parameter is correctly formatted. */
  if (cmd->argc-1 == 3) {
    if (cmd->argv[3][0] != '[' || cmd->argv[3][strlen(cmd->argv[3])-1] != ']')
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        ": badly formatted flags parameter: '", cmd->argv[3], "'", NULL));

    /* We need to parse the flags parameter here, to see if any flags which
     * affect the compilation of the regex (e.g. NC) are present.
     */
    cond_flags = rewrite_parse_cond_flags(cmd->tmp_pool, cmd->argv[3]);

    if (cond_flags & REWRITE_COND_FLAG_NOCASE)
      regex_flags |= REG_ICASE;
  }

  if (!rewrite_conds) {
    if (rewrite_cond_pool)
      destroy_pool(rewrite_cond_pool);

    rewrite_cond_pool = make_sub_pool(rewrite_pool);
    rewrite_conds = make_array(rewrite_cond_pool, 0, sizeof(config_rec *));
  }

  /* Check for a leading '!' negation prefix to the regex pattern */
  if (*cmd->argv[2] == '!') {
    cmd->argv[2]++;
    negated = TRUE;
  }

  /* Check the next character in the given pattern.  It may be a lexical
   * or a file test pattern...
   */
  if (*cmd->argv[2] == '>') {
    cond_op = REWRITE_COND_OP_LEX_LT;
    cond_data = pstrdup(rewrite_pool, ++cmd->argv[2]);

  } else if (*cmd->argv[2] == '<') {
    cond_op = REWRITE_COND_OP_LEX_GT;
    cond_data = pstrdup(rewrite_pool, ++cmd->argv[2]);

  } else if (*cmd->argv[2] == '=') {
    cond_op = REWRITE_COND_OP_LEX_EQ;
    cond_data = pstrdup(rewrite_pool, ++cmd->argv[2]);

  } else if (strcmp(cmd->argv[2], "-d") == 0) {
    cond_op = REWRITE_COND_OP_TEST_DIR;

  } else if (strcmp(cmd->argv[2], "-f") == 0) {
    cond_op = REWRITE_COND_OP_TEST_FILE;

  } else if (strcmp(cmd->argv[2], "-l") == 0) {
    cond_op = REWRITE_COND_OP_TEST_SYMLINK;

  } else if (strcmp(cmd->argv[2], "-s") == 0) {
    cond_op = REWRITE_COND_OP_TEST_SIZE;

  } else {
    cond_op = REWRITE_COND_OP_REGEX;
    cond_data = rewrite_regalloc();

    if ((res = regcomp(cond_data, cmd->argv[2], regex_flags)) != 0) {
      char errstr[200] = {'\0'};

      regerror(res, cond_data, errstr, sizeof(errstr));
      regfree(cond_data);

      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to compile '",
        cmd->argv[2], "' regex: ", errstr, NULL));
    }
  }

  /* Make sure the variables used, if any, are valid. */
  var = cmd->argv[1];
  while (*var != '\0' && (var = strchr(var, '%')) != NULL &&
         strlen(var) > 1 && !isdigit(*(var+1))) {
    register unsigned int i = 0;
    unsigned char is_valid_var = FALSE;

    for (i = 0; i < REWRITE_MAX_VARS; i++) {
      if (!strncmp(var, rewrite_vars[i], 2)) {
        is_valid_var = TRUE;
        break;
      }
    }
    if (!is_valid_var)
      pr_log_debug(DEBUG1, "invalid RewriteCondition variable used");

    var += 2;
  }

  /* Do this manually -- no need to clutter up the configuration tree
   * with config_recs that really don't belong in that contextualized
   * arrangement.  These config_recs will be tracked/retrieved via
   * a RewriteRule config_rec, not via the normal configuration tree
   * retrieval functions.
   */

  cond_pool = make_sub_pool(rewrite_pool);
  c = pcalloc(cond_pool, sizeof(config_rec));
  c->pool = cond_pool;
  c->name = pstrdup(c->pool, cmd->argv[0]);
  c->config_type = CONF_PARAM;
  c->argc = 5;
  c->argv = pcalloc(c->pool, (c->argc+1) * sizeof(void *));
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]); 
  c->argv[1] = (void *) cond_data;

  c->argv[2] = palloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[2]) = negated;

  c->argv[3] = pcalloc(c->pool, sizeof(rewrite_cond_op_t));
  *((rewrite_cond_op_t *) c->argv[3]) = cond_op;

  c->argv[4] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[4]) = cond_flags;

  /* Add this config_rec to an array, to be added to the RewriteRule when
   * (if?) it appears.
   */
  *((config_rec **) push_array(rewrite_conds)) = c;

  return HANDLED(cmd);
}

/* usage: RewriteEngine on|off */
MODRET set_rewriteengine(cmd_rec *cmd) {
  int bool = 0;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expecting boolean argument");

  /* Check for duplicates */
  if (get_param_ptr(cmd->server->conf, cmd->argv[0], FALSE) != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0], ": multiple "     
     "instances not allowed for same server", NULL));

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return HANDLED(cmd);
}

/* usage: RewriteLock file */
MODRET set_rewritelock(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check for non-absolute paths */
  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "absolute path required");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return HANDLED(cmd);
}

/* usage: RewriteLog file|"none" */
MODRET set_rewritelog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check for non-absolute paths */
  if (strcasecmp(cmd->argv[1], "none") != 0 && *(cmd->argv[1]) != '/')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0], ": absolute path "
      "required", NULL));

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return HANDLED(cmd);
}

/* usage: RewriteMap map-name map-type:map-source */ 
MODRET set_rewritemap(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *mapsrc = NULL;
  void *map = NULL;
  
  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* Check the configured map types */
  if ((mapsrc = strchr(cmd->argv[2], ':')) == NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid RewriteMap parameter: '",
      cmd->argv[2], "'", NULL));

  *mapsrc = '\0';
  mapsrc++;

  if (!strcmp(cmd->argv[2], "int")) {
    c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);

    /* Check that the given function is a valid internal mapping function. */
    if (strcmp(mapsrc, "replaceall") == 0) {
      map = (void *) rewrite_map_int_replaceall;

    } else if (strcmp(mapsrc, "tolower") == 0) {
      map = (void *) rewrite_map_int_tolower;

    } else if (strcmp(mapsrc, "toupper") == 0) {
      map = (void *) rewrite_map_int_toupper;

    } else if (strcmp(mapsrc, "unescape") == 0) {
      map = (void *) rewrite_map_int_unescape;

    } else if (strcmp(mapsrc, "utf8trans") == 0) {
      map = (void *) rewrite_map_int_utf8trans;

    } else
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        "unknown internal map function requested: '", mapsrc, "'", NULL));

  } else if (!strcmp(cmd->argv[2], "fifo")) {
    struct stat st;

    c = add_config_param(cmd->argv[0], 4, NULL, NULL, NULL, NULL);

    /* Make sure the given path is absolute. */
    if (*mapsrc != '/')
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0],
        ": fifo: absolute path required", NULL));

    /* Stat the path, to make sure it is indeed a FIFO. */
    if (pr_fsio_stat(mapsrc, &st) < 0)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0],
        ": fifo: error stat'ing '", mapsrc, "': ", strerror(errno), NULL));

    if (!S_ISFIFO(st.st_mode))
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0],
        ": fifo: error: '", mapsrc, "' is not a FIFO", NULL));

    map = (void *) pstrdup(c->pool, mapsrc);

    /* Initialize the FIFO file descriptor slot. */
    c->argv[3] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[3]) = -1;

  } else if (!strcmp(cmd->argv[2], "txt")) {
    pool *txt_pool = NULL;
    rewrite_map_txt_t *txtmap = NULL;

    c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);
    txt_pool = make_sub_pool(c->pool);
    txtmap = pcalloc(txt_pool, sizeof(rewrite_map_txt_t));

    /* Make sure the given path is absolute. */
    if (*mapsrc != '/')
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0],
        ": txt: absolute path required", NULL));

    txtmap->txt_pool = txt_pool;
    txtmap->txt_path = pstrdup(txt_pool, mapsrc);    

    if (!rewrite_parse_map_txt(txtmap)) {
      pr_log_debug(DEBUG3, "%s: error parsing map file", cmd->argv[0]);
      pr_log_debug(DEBUG3, "%s: check the RewriteLog for details",
        cmd->argv[0]);
    }

    map = (void *) txtmap;

  } else
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid RewriteMap map type: '",
      cmd->argv[2], "'", NULL));
 
  /* A defined map name is available within the scope of the server in
   * which it was defined.
   */

  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
  c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  c->argv[2] = map;

  return HANDLED(cmd);
}

/* usage: RewriteRule pattern substitution [flags] */
MODRET set_rewriterule(cmd_rec *cmd) {
  config_rec *c = NULL;
  regex_t *preg = NULL;
  unsigned int rule_flags = 0;
  unsigned char negated = FALSE;
  int regex_flags = REG_EXTENDED, res = -1;
  char *tmp = NULL;
  register unsigned int i = 0;

  if (cmd->argc-1 < 2 || cmd->argc-1 > 3)
    CONF_ERROR(cmd, "bad number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  /* Make sure that, if present, the flags parameter is correctly formatted. */
  if (cmd->argc-1 == 3) {
    if (cmd->argv[3][0] != '[' || cmd->argv[3][strlen(cmd->argv[3])-1] != ']')
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        ": badly formatted flags parameter: '", cmd->argv[3], "'", NULL));

    /* We need to parse the flags parameter here, to see if any flags which
     * affect the compilation of the regex (e.g. NC) are present.
     */
    rule_flags = rewrite_parse_rule_flags(cmd->tmp_pool, cmd->argv[3]);

    if (rule_flags & REWRITE_RULE_FLAG_NOCASE)
      regex_flags |= REG_ICASE;
  }

  preg = rewrite_regalloc();

  /* Check for a leading '!' prefix, signifying regex negation */
  if (*cmd->argv[1] == '!') {
    negated = TRUE;
    cmd->argv[1]++;
  }

  if ((res = regcomp(preg, cmd->argv[1], regex_flags)) != 0) {
    char errstr[200] = {'\0'};

    regerror(res, preg, errstr, sizeof(errstr));
    regfree(preg);

    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to compile '",
      cmd->argv[1], "' regex: ", errstr, NULL));
  }

  /* Note: scan through the substitution parameter, checking any given
   * variables, and noting if any invalid sequences are found
   */
  tmp = NULL;

  c = add_config_param(cmd->argv[0], 6, preg, NULL, NULL, NULL, NULL, NULL);

  /* Note: how to handle the substitution expression? Later? */
  c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  c->argv[2] = palloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[2]) = negated;

  /* Attach the list of conditions to the config_rec.  Don't forget to
   * clear/reset the list when done.
   */
  if (rewrite_conds) {
    config_rec **arg_conds = NULL, **conf_conds = NULL;

    /* Allocate space for an array of rewrite_conds->nelts + 1.  The extra
     * pointer is for NULL-terminating the array
     */
    c->argv[3] = pcalloc(c->pool,
      (rewrite_conds->nelts + 1) * sizeof(config_rec *));

    arg_conds = (config_rec **) c->argv[3];
    conf_conds = (config_rec **) rewrite_conds->elts;

    for (i = 0; i <= rewrite_conds->nelts; i++)
      arg_conds[i] = conf_conds[i];

    arg_conds[rewrite_conds->nelts] = NULL;

    destroy_pool(rewrite_cond_pool);
    rewrite_cond_pool = NULL;
    rewrite_conds = NULL;

  } else
    c->argv[3] = NULL;

  c->argv[4] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[4]) = rule_flags;

  /* The very last slot is to be filled by a unique ID (just a counter
   * value).
   */
  c->argv[5] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[5]) = rewrite_nrules++;

  c->flags |= CF_MERGEDOWN_MULTI;
  return HANDLED(cmd);
}

/* Command handlers
 */

MODRET rewrite_fixup(cmd_rec *cmd) {
  config_rec *c = NULL;
  array_header *seen_rules = NULL;

  /* Is RewriteEngine on? */
  if (!rewrite_engine)
    return DECLINED(cmd);

  /* If this command has no argument(s), the module has nothing on which to
   * operate.
   */
  if (cmd->argc == 1) {
    rewrite_log("rewrite_fixup(): skipping %s (no arg)", cmd->argv[0]);
    return DECLINED(cmd);
  }

  /* Create an array that will contain the IDs of the RewriteRules we've
   * already processed.
   */
  seen_rules = make_array(cmd->tmp_pool, 0, sizeof(unsigned int));

  /* Find all RewriteRules in effect. */
  c = find_config(CURRENT_CONF, CONF_PARAM, "RewriteRule", FALSE);

  while (c) {
    unsigned char exec_rule = FALSE;
    rewrite_log("rewrite_fixup(): found RewriteRule");

    /* If we've already seen this Rule, skip on to the next Rule. */
    if (seen_rules->nelts > 0) {
      register unsigned int i = 0;
      unsigned char saw_rule = FALSE;
      unsigned int id = *((unsigned int *) c->argv[5]), *ids = seen_rules->elts;

      for (i = 0; i < seen_rules->nelts; i++) {
        if (ids[i] == id) {
          saw_rule = TRUE;
          break;
        }
      }

      if (saw_rule) {
        rewrite_log("rewrite_fixup(): already saw this RewriteRule, skipping");
        c = find_config_next(c, c->next, CONF_PARAM, "RewriteRule", FALSE);
        continue;
      }
    }

    /* Add this Rule's ID to the list of seen Rules. */
    *((unsigned int *) push_array(seen_rules)) = *((unsigned int *) c->argv[5]);

    /* Make sure the given RewriteRule regex matches the command argument. */
    memset(&rewrite_rule_matches, '\0', sizeof(rewrite_rule_matches));
    rewrite_rule_matches.match_string = cmd->arg;
    if (!rewrite_regexec(cmd->arg, (regex_t *) c->argv[0],
        *((unsigned char *) c->argv[2]), &rewrite_rule_matches)) {
      rewrite_log("rewrite_fixup(): %s arg '%s' does not match RewriteRule "
        "regex", cmd->argv[0], cmd->arg);
      c = find_config_next(c, c->next, CONF_PARAM, "RewriteRule", FALSE);
      continue;

    } else {

      /* The command matches the RewriteRule's regex.  If there are conditions
       * attached to the RewriteRule, make sure those are met as well.
       */
      if (c->argv[3]) {
        register unsigned int i = 0;
        config_rec **conds = (config_rec **) c->argv[3];

        rewrite_log("rewrite_fixup(): examining RewriteRule conditions");
        exec_rule = TRUE;

        for (i = 0; conds[i] != NULL; i++) {
          unsigned int cond_flags = *((unsigned int *) conds[i]->argv[4]);

          if (!rewrite_match_cond(cmd, conds[i])) {

            /* If this condition is not to be OR'd with the next condition,
             * or there is no next condition, fail the Rule.
             */
            if (!(cond_flags & REWRITE_COND_FLAG_ORNEXT) ||
                conds[i+1] == NULL) {
              exec_rule = FALSE;
              rewrite_log("rewrite_fixup(): condition not met, skipping this "
                "Rule");
              break;
            }

          } else
            rewrite_log("rewrite_fixup(): condition met");
        }

      } else
        /* There are no conditions. */
        exec_rule = TRUE;
    } 

    if (exec_rule) {
      char *new_arg = NULL;
      unsigned int rule_flags = *((unsigned int *) c->argv[4]);

      rewrite_log("rewrite_fixup(): executing RewriteRule");
      new_arg = rewrite_subst(cmd, (char *) c->argv[1]);

      if (strlen(new_arg) > 0) {
        cmd->arg = new_arg;
        rewrite_log("rewrite_fixup(): %s arg now '%s'", cmd->argv[0], cmd->arg);

      } else
        rewrite_log("rewrite_fixup(): error processing RewriteRule");

      /* If this Rule is marked as "last", break out of the loop. */
      if (rule_flags & REWRITE_RULE_FLAG_LAST) {
        rewrite_log("rewrite_fixup(): Rule marked as 'last', done processing "
          "Rules");
        break;
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RewriteRule", FALSE);
  }

  return DECLINED(cmd);
}

/* Events handlers
 */

static void rewrite_exit_ev(const void *event_data, void *user_data) {
  rewrite_closelog();
  return;
}

#if defined(PR_SHARED_MODULE)
static void rewrite_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_rewrite.c", (const char *) event_data) == 0) {
    pr_event_unregister(&rewrite_module, NULL, NULL);

    if (rewrite_pool) {
      destroy_pool(rewrite_pool);
      rewrite_pool = NULL;
    }
  }
}
#endif /* PR_SHARED_MODULE */

static void rewrite_restart_ev(const void *event_data, void *user_data) {
  if (rewrite_regexes) {
    register unsigned int i = 0;
    regex_t **regexes = (regex_t **) rewrite_regexes->elts;

    for (i = 0; i < rewrite_regexes->nelts && regexes[i]; i++) {
      regfree(regexes[i]);
      free(regexes[i]);
    }
  }

  if (rewrite_pool) {
    destroy_pool(rewrite_pool);
    rewrite_cond_pool = NULL;
    rewrite_conds = NULL;
    rewrite_regexes = NULL;

    /* Re-allocate a pool for this module's use. */
    rewrite_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(rewrite_pool, MOD_REWRITE_VERSION);
  }
}

/* Initialization functions
 */

static int rewrite_sess_init(void) {
  config_rec *c = NULL;
  unsigned char *engine = get_param_ptr(main_server->conf, "RewriteEngine",
    FALSE);

  /* Is RewriteEngine on? */
  if (!engine || *engine == FALSE) {
    rewrite_engine = FALSE;
    return 0;
  }

  rewrite_engine = TRUE;

  /* Open the RewriteLog, if present. */
  rewrite_openlog();

  /* Make sure proper cleanup is done when a child exits. */
  pr_event_register(&rewrite_module, "core.exit", rewrite_exit_ev, NULL);

  /* Loop through all the RewriteMap config_recs for this server, and for
   * all FIFO maps, open FIFO file descriptors.  This has to be done here,
   * before any possible chroot occurs.
   */

  c = find_config(main_server->conf, CONF_PARAM, "RewriteMap", FALSE);

  while (c) {
    pr_signals_handle();

    if (!strcmp(c->argv[1], "fifo")) {
      PRIVS_ROOT
      if (!rewrite_open_fifo(c))
        rewrite_log("error preparing FIFO RewriteMap");
      PRIVS_RELINQUISH
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RewriteMap", FALSE);
  }

  return 0;
}

static int rewrite_init(void) {

  /* Allocate a pool for this module's use. */
  if (!rewrite_pool) {
    rewrite_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(rewrite_pool, MOD_REWRITE_VERSION);
  }

#if defined(PR_SHARED_MODULE)
  pr_event_register(&rewrite_module, "core.module-unload",
    rewrite_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */

  /* Add a restart handler. */
  pr_event_register(&rewrite_module, "core.restart", rewrite_restart_ev,
    NULL);

  return 0;
}

/* Module API Tables
 */

static conftable rewrite_conftab[] = {
  { "RewriteCondition",		set_rewritecondition,	NULL },
  { "RewriteEngine",		set_rewriteengine,	NULL },
  { "RewriteLock",		set_rewritelock,	NULL },
  { "RewriteLog",		set_rewritelog,		NULL },
  { "RewriteMap",		set_rewritemap,		NULL },
  { "RewriteRule",		set_rewriterule,	NULL },
  { NULL }
};

static cmdtable rewrite_cmdtab[] = {
  { PRE_CMD,	C_ANY,	G_NONE,	rewrite_fixup,	FALSE,	FALSE },
  { 0, NULL }
};

module rewrite_module = {

  /* Always NULL */
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "rewrite",

  /* Module configuration handler table */
  rewrite_conftab,

  /* Module command handler table */
  rewrite_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  rewrite_init,

  /* Session initialization function */
  rewrite_sess_init
};

#endif /* HAVE_REGEX_H */
