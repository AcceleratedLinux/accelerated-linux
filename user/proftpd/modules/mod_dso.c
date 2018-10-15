/*
 * ProFTPD: mod_dso -- support for loading/unloading modules at run-time
 *
 * Copyright (c) 2004 TJ Saunders <tj@castaglia.org>
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_dso, contrib software for proftpd 1.2.x.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_dso.c,v 1.9 2004/12/17 21:56:00 castaglia Exp $
 */

#include "conf.h"
#include "mod_ctrls.h"
#include "ltdl.h"

#define MOD_DSO_VERSION		"mod_dso/0.4"

/* From modules/module_glue.c */
extern module *static_modules[];
extern module *loaded_modules;

module dso_module;
static const char *dso_module_path = PR_LIBEXEC_DIR;
static pool *dso_pool = NULL;

#ifdef PR_USE_CTRLS
static ctrls_acttab_t dso_acttab[];
#endif

static int dso_load_file(char *path) {

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  /* XXX Is this sufficient for loading an external library? */
  if (lt_dlopenext(path) == NULL) {
    pr_log_pri(PR_LOG_NOTICE, MOD_DSO_VERSION ": unable to open '%s': %s",
      path, lt_dlerror());
    errno = EPERM;
    return -1;
  }

  return 0;
}

static int dso_load_module(char *name) {
  int res;
  lt_ptr mh = NULL;
  char *symbol_name, *path, *tmp;
  module *m;

  if (!name) {
    errno = EINVAL;
    return -1;
  }

  if (strncmp(name, "mod_", 4) != 0 ||
      name[strlen(name)-2] != '.' ||
      name[strlen(name)-1] != 'c') {
    errno = EINVAL;
    return -1;
  }

  tmp = strrchr(name, '.');
  if (!tmp) {
    errno = EINVAL;
    return -1;
  }
  *tmp = '\0';

  /* Load file: $prefix/libexec/<module> */
  path = pdircat(dso_pool, dso_module_path, name, NULL);

  mh = lt_dlopenext(path);
  if (mh == NULL) {
    *tmp = '.';

    pr_log_debug(DEBUG3, MOD_DSO_VERSION ": unable to dlopen '%s': %s (%s)",
      name, lt_dlerror(), strerror(errno));
    pr_log_debug(DEBUG3, MOD_DSO_VERSION
      ": defaulting to 'self' for symbol resolution");

    mh = lt_dlopen(NULL);
    if (mh == NULL) {
      pr_log_debug(DEBUG0, MOD_DSO_VERSION ": error loading 'self': %s",
        lt_dlerror());
      errno = EPERM;
      return -1;
    }
  }

  /* Tease name of the module structure out of the given name:
   *  <module>.<ext> --> <module>_module
   */

  *tmp = '\0';
  symbol_name = pstrcat(dso_pool, name+4, "_module", NULL);

  /* Lookup module structure symbol by name. */
  m = (module *) lt_dlsym(mh, symbol_name);
  if (m == NULL) {
    *tmp = '.';
    pr_log_debug(DEBUG1, MOD_DSO_VERSION
      ": unable to find module symbol '%s' in '%s'", symbol_name,
        mh ? name : "self");
    lt_dlclose(mh);
    mh = NULL;

    errno = EACCES;
    return -1;
  }
  *tmp = '.';

  m->handle = mh;

  /* Add the module to the core structures */
  res = pr_module_load(m);
  if (res < 0) {
    if (errno == EEXIST)
      pr_log_pri(PR_LOG_INFO, MOD_DSO_VERSION
        ": module 'mod_%s.c' already loaded", m->name);

    else if (errno == EACCES)
      pr_log_pri(PR_LOG_ERR, MOD_DSO_VERSION
        ": module 'mod_%s.c' has wrong API version (0x%x), must be 0x%x",
        m->name, m->api_version, PR_MODULE_API_VERSION);

    else if (errno == EPERM)
      pr_log_pri(PR_LOG_ERR, MOD_DSO_VERSION
        ": module 'mod_%s.c' failed to initialize", m->name);

    lt_dlclose(mh);
    mh = NULL;
    return -1;
  }
  
  return 0;
}

static int dso_unload_module(module *m) {
  int res;
  char *name;

  /* Some modules cannot be unloaded. */
  if (strcmp(m->name, "dso") == 0) {
    errno = EPERM;
    return -1;
  }

  name = pstrdup(dso_pool, m->name);

  res = pr_module_unload(m);
  if (res < 0)
    pr_log_debug(DEBUG1, MOD_DSO_VERSION
      ": error unloading module 'mod_%s.c': %s", m->name, strerror(errno));

  if (lt_dlclose(m->handle) < 0) {
    pr_log_debug(DEBUG1, MOD_DSO_VERSION ": error closing '%s': %s",
      name, lt_dlerror());
    return -1;
  }

  return 0;
}

#ifdef PR_USE_CTRLS
static int dso_unload_module_by_name(const char *name) {
  module *m;

  if (strncmp(name, "mod_", 4) != 0 ||
      name[strlen(name)-2] != '.' ||
      name[strlen(name)-1] != 'c') {
    errno = EINVAL;
    return -1;
  }

  /* Lookup the module pointer for the given module name. */
  m = pr_module_get(name);
  if (!m) {
    errno = ENOENT;
    return -1;
  }

  return dso_unload_module(m);
}
#endif /* PR_USE_CTRLS */

#ifdef PR_USE_CTRLS
/* Controls handlers
 */

static int dso_handle_insmod(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i;

  /* Check the ACL. */
  if (!ctrls_check_acl(ctrl, dso_acttab, "insmod")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc == 0 ||
      reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "missing required parameters");
    return -1;
  }

  for (i = 0; i < reqargc; i++) {
    if (dso_load_module(reqargv[i]) < 0) {

      /* Make the error messages a little more relevant. */
      switch (errno) {
        case EINVAL:
          pr_ctrls_add_response(ctrl, "error loading '%s': Bad module name",
            reqargv[i]);
          break;

        case EEXIST:
          pr_ctrls_add_response(ctrl, "error loading '%s': Already loaded",
            reqargv[i]);
          break;

        default:
          pr_ctrls_add_response(ctrl, "error loading '%s': %s", reqargv[i],
            strerror(errno));
          break;
      }

    } else
      pr_ctrls_add_response(ctrl, "'%s' loaded", reqargv[i]);
  }
  
  return 0;
}

static int dso_handle_lsmod(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  module *m;

  /* Check the ACL. */
  if (!ctrls_check_acl(ctrl, dso_acttab, "lsmod")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  if (reqargc != 0) {
    pr_ctrls_add_response(ctrl, "wrong number of parameters");
    return -1;
  }

  /* We want to show the modules as `proftpd -l` shows them, in module
   * load order.  So first we find the end of the loaded_modules list,
   * then walk it backwards.
   */
  for (m = loaded_modules; m && m->next; m = m->next);

  pr_ctrls_add_response(ctrl, "Loaded Modules:");
  for (; m; m = m->prev)
    pr_ctrls_add_response(ctrl, "  mod_%s.c", m->name);

  return 0;
}

static int dso_handle_rmmod(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i;

  /* Check the ACL. */
  if (!ctrls_check_acl(ctrl, dso_acttab, "rmmod")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc == 0 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "missing required parameters");
    return -1;
  }

  for (i = 0; i < reqargc; i++) {
    if (dso_unload_module_by_name(reqargv[i]) < 0) {
      switch (errno) {
        case EINVAL:
          pr_ctrls_add_response(ctrl, "error unloading '%s': Bad module name",
            reqargv[i]);
          break;

        case ENOENT:
          pr_ctrls_add_response(ctrl, "error unloading '%s': Module not loaded",
            reqargv[i]);
          break;

        default:
          pr_ctrls_add_response(ctrl, "error unloading '%s': %s",
            reqargv[i], strerror(errno));
          break;
      }

    } else
      pr_ctrls_add_response(ctrl, "'%s' unloaded", reqargv[i]);
  }

  return 0;
}
#endif /* PR_USE_CTRLS */

/* Configuration handlers
 */

/* usage: LoadFile path */
MODRET set_loadfile(cmd_rec *cmd) {

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "must be an absolute path");

  if (dso_load_file(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error loading '", cmd->argv[1],
      "': ", strerror(errno), NULL));

  return HANDLED(cmd);
}

/* usage: LoadModule module */
MODRET set_loadmodule(cmd_rec *cmd) {

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (dso_load_module(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error loading module '",
      cmd->argv[1], "': ", strerror(errno), NULL));

  return HANDLED(cmd);
}

/* usage: ModuleControlsACLs actions|all allow|deny user|group list */
MODRET set_modulectrlsacls(cmd_rec *cmd) {
#ifdef PR_USE_CTRLS
  char *bad_action = NULL, **actions = NULL;

  CHECK_ARGS(cmd, 4);
  CHECK_CONF(cmd, CONF_ROOT);

  actions = ctrls_parse_acl(cmd->tmp_pool, cmd->argv[1]);

  if (strcmp(cmd->argv[2], "allow") != 0 &&
      strcmp(cmd->argv[2], "deny") != 0)
    CONF_ERROR(cmd, "second parameter must be 'allow' or 'deny'");

  if (strcmp(cmd->argv[3], "user") != 0 &&
      strcmp(cmd->argv[3], "group") != 0)
    CONF_ERROR(cmd, "third parameter must be 'user' or 'group'");

  if ((bad_action = ctrls_set_module_acls(dso_acttab, dso_pool, actions,
      cmd->argv[2], cmd->argv[3], cmd->argv[4])) != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown action: '",
      bad_action, "'", NULL));

  return HANDLED(cmd);
#else
  CONF_ERROR(cmd, "requires Controls support (--enable-ctrls)");
#endif
}

/* usage: ModuleOrder mod1 mod2 ... modN */
MODRET set_moduleorder(cmd_rec *cmd) {
  register unsigned int i;
  module *m, *mn, *module_list = NULL;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT);

  /* What about duplicate names in the list?
   *
   * What if the given list is longer than the one already in loaded_modules?
   * This will be caught by the existence check.  Otherwise, the only way for
   * the list to be longer is if there are duplicates, which will be caught
   * by the duplicate check.
   */

  /* Make sure the given module names exist. */
  for (i = 1; i < cmd->argc; i++) {
    if (pr_module_get(cmd->argv[i]) == NULL)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "no such module '", cmd->argv[i],
      "' loaded", NULL));
  }

  /* Make sure there are no duplicate module names in the list. */
  for (i = 1; i < cmd->argc; i++) {
    register unsigned int j;

    for (j = i + 1; j < cmd->argc; j++) {
      if (strcmp(cmd->argv[i], cmd->argv[j]) == 0) {
        char ibuf[4], jbuf[4];

        snprintf(ibuf, sizeof(ibuf), "%u", i);
        ibuf[sizeof(ibuf)-1] = '\0';

        snprintf(jbuf, sizeof(jbuf), "%u", j);
        jbuf[sizeof(jbuf)-1] = '\0';

        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
          "duplicate module name '", cmd->argv[i], "' as parameters ",
          ibuf, " and ", jbuf, NULL));
      }
    }
  }

  pr_log_debug(DEBUG4, "%s: reordering modules", cmd->argv[0]);
  for (i = 1; i < cmd->argc; i++) {
    m = pr_module_get(cmd->argv[i]);

    if (module_list) {
      m->next = module_list;
      module_list->prev = m;
      module_list = m;

    } else
      module_list = m;
  }

  /* Now, unload all the modules in the loaded_modules list, then load
   * the modules in our module_list.
   */
  for (m = loaded_modules; m;) {
    mn = m->next;

    if (pr_module_unload(m) < 0)
      pr_log_debug(DEBUG0, "%s: error unloading module 'mod_%s.c': %s",
        cmd->argv[0], m->name, strerror(errno));

    m = mn;
  }

  for (m = module_list; m; m = m->next) {
    if (pr_module_load(m) < 0) {
      pr_log_debug(DEBUG0, "%s: error loading module 'mod_%s.c': %s",
        cmd->argv[0], m->name, strerror(errno));
      exit(1);
    }
  }

  pr_log_pri(PR_LOG_NOTICE, "module order is now:");
  for (m = loaded_modules; m; m = m->next)
    pr_log_pri(PR_LOG_NOTICE, " mod_%s.c", m->name);

  return HANDLED(cmd);
}

/* usage: ModulePath path */
MODRET set_modulepath(cmd_rec *cmd) {
  int res;
  struct stat st;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "must be an absolute path");

  /* Make sure that the configured path is not world-writeable. */
  res = pr_fsio_stat(cmd->argv[1], &st);
  if (res < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error checking '",
      cmd->argv[1], "': ", strerror(errno), NULL)); 

  if (!S_ISDIR(st.st_mode))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[1], " is not a directory",
      NULL));

  if (st.st_mode & S_IWOTH)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[1], " is world-writeable",
      NULL));

  if (lt_dlsetsearchpath(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error setting module path: ",
      lt_dlerror(), NULL));

  dso_module_path = pstrdup(dso_pool, cmd->argv[1]);
  return HANDLED(cmd);
}

/* Event handlers
 */

static void dso_restart_ev(const void *event_data, void *user_data) {
  module *m, *mi;
#ifdef PR_USE_CTRLS
  register unsigned int i = 0;
#endif /* PR_USE_CTRLS */

  if (dso_pool)
    destroy_pool(dso_pool);

  dso_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(dso_pool, MOD_DSO_VERSION);

#ifdef PR_USE_CTRLS
  /* Re-register the control handlers */
  for (i = 0; dso_acttab[i].act_action; i++) {
    pool *sub_pool = make_sub_pool(dso_pool);

    /* Allocate and initialize the ACL for this control. */
    dso_acttab[i].act_acl = pcalloc(sub_pool, sizeof(ctrls_acl_t));
    dso_acttab[i].act_acl->acl_pool = sub_pool;
    ctrls_init_acl(dso_acttab[i].act_acl);
  }
#endif /* PR_USE_CTRLS */

  /* Unload all shared modules. */
  for (mi = loaded_modules; mi; mi = m) {
    register unsigned int i;
    int is_static = FALSE;

    m = mi->next;

    for (i = 0; static_modules[i]; i++) {
      if (strcmp(mi->name, static_modules[i]->name) == 0) {
        is_static = TRUE;
        break;
      }
    }

    if (!is_static) {
      pr_log_debug(DEBUG7, "unloading 'mod_%s.c'", mi->name);
      if (dso_unload_module(mi) < 0)
        pr_log_pri(PR_LOG_INFO, "error unloading 'mod_%s.c': %s",
          mi->name, strerror(errno));
    }
  }

  return;
}

/* Initialization routines
 */

static int dso_init(void) {
#ifdef PR_USE_CTRLS
  register unsigned int i = 0;
#endif /* PR_USE_CTRLS */

  /* Allocate the pool for this module's use. */
  dso_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(dso_pool, MOD_DSO_VERSION);

  LTDL_SET_PRELOADED_SYMBOLS();

  /* Initialize libltdl. */
  if (lt_dlinit() < 0) {
    pr_log_pri(PR_LOG_ERR, MOD_DSO_VERSION ": error initializing libltdl: %s",
      lt_dlerror());
    return -1;
  }

  /* Explicitly set the search path used for opening modules. */
  if (lt_dlsetsearchpath(dso_module_path) < 0) {
    pr_log_pri(PR_LOG_ERR, MOD_DSO_VERSION ": error setting module path: %s",
      lt_dlerror());
    return -1;
  }

#ifdef PR_USE_CTRLS
  /* Register ctrls handlers. */
  for (i = 0; dso_acttab[i].act_action; i++) {
    pool *sub_pool = make_sub_pool(dso_pool);

    /* Allocate and initialize the ACL for this control. */
    dso_acttab[i].act_acl = pcalloc(sub_pool, sizeof(ctrls_acl_t));
    dso_acttab[i].act_acl->acl_pool = sub_pool;
    ctrls_init_acl(dso_acttab[i].act_acl);

    if (pr_ctrls_register(&dso_module, dso_acttab[i].act_action,
        dso_acttab[i].act_desc, dso_acttab[i].act_cb) < 0)
      pr_log_pri(PR_LOG_INFO, MOD_DSO_VERSION
        ": error registering '%s' control: %s", dso_acttab[i].act_action,
        strerror(errno));
  }
#endif /* PR_USE_CTRLS */

  /* Ideally, we'd call register a listener for the 'core.exit' event
   * and call lt_dlexit() there, politely freeing up any resources allocated
   * by the ltdl library.  However, it's possible that other modules, later in
   * the dispatch cycles, may need to use pointers to memory in shared modules
   * that would become invalid by such finalization.  So we skip it, for now.
   *
   * If there was a way to schedule this handler, to happen after all other
   * exit handlers, that'd be best.
   */
  pr_event_register(&dso_module, "core.restart", dso_restart_ev, NULL);

  return 0;
}

static int dso_sess_init(void) {
  pr_event_unregister(&dso_module, "core.restart", dso_restart_ev);
  return 0;
}

#ifdef PR_USE_CTRLS
static ctrls_acttab_t dso_acttab[] = {
  { "insmod",	"load modules",		NULL,	dso_handle_insmod },
  { "lsmod",	"list modules",		NULL, 	dso_handle_lsmod },
  { "rmmod",	"unload modules",	NULL,	dso_handle_rmmod },
  { NULL, NULL, NULL, NULL }
};
#endif /* PR_USE_CTRLS */

/* Module API tables
 */

static conftable dso_conftab[] = {
  { "LoadFile",			set_loadfile,		NULL },
  { "LoadModule",		set_loadmodule,		NULL },
  { "ModuleControlsACLs",	set_modulectrlsacls,	NULL },
  { "ModuleOrder",		set_moduleorder,	NULL },
  { "ModulePath",		set_modulepath,		NULL },
  { NULL }
};

module dso_module = {
  /* Always NULL */
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "dso",

  /* Module configuration handler table */
  dso_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  dso_init,

  /* Session initialization function */
  dso_sess_init,

  /* Module version */
  MOD_DSO_VERSION
};

