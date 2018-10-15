/* pam_handlers.c -- pam config file parsing and module loading */

/*
 * created by Marc Ewing.
 * Currently maintained by Andrew G. Morgan <morgan@kernel.org>
 *
 * $Id: pam_handlers.c,v 1.21 2005/11/18 08:57:01 kukuk Exp $
 *
 */

#include "pam_private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef PAM_DYNAMIC
#include "pam_dynamic.h"
#endif /* PAM_DYNAMIC */

#define BUF_SIZE                  1024
#define MODULE_CHUNK              4
#define UNKNOWN_MODULE_PATH       "<*unknown module path*>"
#ifndef _PAM_ISA
#define _PAM_ISA "."
#endif

static int _pam_assemble_line(FILE *f, char *buf, int buf_len);

static void _pam_free_handlers_aux(struct handler **hp);

static int _pam_add_handler(pam_handle_t *pamh
		     , int must_fail, int other, int type
		     , int *actions, const char *mod_path
		     , int argc, char **argv, int argvlen);

/* Values for module type */

#define PAM_T_ANY     0
#define PAM_T_AUTH    1
#define PAM_T_SESS    2
#define PAM_T_ACCT    4
#define PAM_T_PASS    8

static int _pam_load_conf_file(pam_handle_t *pamh, const char *config_name
				, const char *service /* specific file */
				, int module_type /* specific type */
#ifdef PAM_READ_BOTH_CONFS
				, int not_other
#endif /* PAM_READ_BOTH_CONFS */
    );

static int _pam_parse_conf_file(pam_handle_t *pamh, FILE *f
				, const char *known_service /* specific file */
				, int requested_module_type /* specific type */
#ifdef PAM_READ_BOTH_CONFS
				, int not_other
#endif /* PAM_READ_BOTH_CONFS */
    )
{
    char buf[BUF_SIZE];
    int x;                    /* read a line from the FILE *f ? */
    /*
     * read a line from the configuration (FILE *) f
     */
    while ((x = _pam_assemble_line(f, buf, BUF_SIZE)) > 0) {
	char *tok, *nexttok=NULL;
	const char *this_service;
	const char *mod_path;
	int module_type, actions[_PAM_RETURN_VALUES];
	int other;            /* set if module is for PAM_DEFAULT_SERVICE */
	int res;              /* module added successfully? */
	int must_fail=0;      /* a badly formatted line must fail when used */
	int argc;
	char **argv;
	int argvlen;

	D(("_pam_init_handler: LINE: %s", buf));
	if (known_service != NULL) {
	    nexttok = buf;
	    /* No service field: all lines are for the known service. */
	    this_service = known_service;
	} else {
	    this_service = tok = _pam_StrTok(buf, " \n\t", &nexttok);
	}

#ifdef PAM_READ_BOTH_CONFS
	if (not_other)
	    other = 0;
	else
#endif /* PAM_READ_BOTH_CONFS */
	other = !_pam_strCMP(this_service, PAM_DEFAULT_SERVICE);

	/* accept "service name" or PAM_DEFAULT_SERVICE modules */
	if (!_pam_strCMP(this_service, pamh->service_name) || other) {
	    int pam_include = 0;

	    /* This is a service we are looking for */
	    D(("_pam_init_handlers: Found PAM config entry for: %s"
	       , this_service));

	    tok = _pam_StrTok(NULL, " \n\t", &nexttok);
	    if (tok == NULL) {
	        /* module type does not exist */
	        D(("_pam_init_handlers: empty module type for %s", this_service));
	        pam_syslog(pamh, LOG_ERR,
			   "(%s) empty module type", this_service);
	        module_type = (requested_module_type != PAM_T_ANY) ?
		  requested_module_type : PAM_T_AUTH;	/* most sensitive */
	        must_fail = 1; /* install as normal but fail when dispatched */
	    } else if (!_pam_strCMP("auth", tok)) {
		module_type = PAM_T_AUTH;
	    } else if (!_pam_strCMP("session", tok)) {
		module_type = PAM_T_SESS;
	    } else if (!_pam_strCMP("account", tok)) {
		module_type = PAM_T_ACCT;
	    } else if (!_pam_strCMP("password", tok)) {
		module_type = PAM_T_PASS;
	    } else {
		/* Illegal module type */
		D(("_pam_init_handlers: bad module type: %s", tok));
		pam_syslog(pamh, LOG_ERR, "(%s) illegal module type: %s",
			   this_service, tok);
		module_type = (requested_module_type != PAM_T_ANY) ?
			      requested_module_type : PAM_T_AUTH;	/* most sensitive */
		must_fail = 1; /* install as normal but fail when dispatched */
	    }
	    D(("Using %s config entry: %s", must_fail?"BAD ":"", tok));
	    if (requested_module_type != PAM_T_ANY &&
	        module_type != requested_module_type) {
		D(("Skipping config entry: %s (requested=%d, found=%d)",
		   tok, requested_module_type, module_type));
		continue;
	    }

	    /* reset the actions to .._UNDEF's -- this is so that
               we can work out which entries are not yet set (for default). */
	    {
		int i;
		for (i=0; i<_PAM_RETURN_VALUES;
		     actions[i++] = _PAM_ACTION_UNDEF);
	    }
	    tok = _pam_StrTok(NULL, " \n\t", &nexttok);
	    if (tok == NULL) {
		/* no module name given */
		D(("_pam_init_handlers: no control flag supplied"));
		pam_syslog(pamh, LOG_ERR,
			   "(%s) no control flag supplied", this_service);
		_pam_set_default_control(actions, _PAM_ACTION_BAD);
		must_fail = 1;
	    } else if (!_pam_strCMP("required", tok)) {
		D(("*PAM_F_REQUIRED*"));
		actions[PAM_SUCCESS] = _PAM_ACTION_OK;
		actions[PAM_NEW_AUTHTOK_REQD] = _PAM_ACTION_OK;
                actions[PAM_IGNORE] = _PAM_ACTION_IGNORE;
		_pam_set_default_control(actions, _PAM_ACTION_BAD);
	    } else if (!_pam_strCMP("requisite", tok)) {
		D(("*PAM_F_REQUISITE*"));
		actions[PAM_SUCCESS] = _PAM_ACTION_OK;
		actions[PAM_NEW_AUTHTOK_REQD] = _PAM_ACTION_OK;
                actions[PAM_IGNORE] = _PAM_ACTION_IGNORE;
		_pam_set_default_control(actions, _PAM_ACTION_DIE);
	    } else if (!_pam_strCMP("optional", tok)) {
		D(("*PAM_F_OPTIONAL*"));
		actions[PAM_SUCCESS] = _PAM_ACTION_OK;
		actions[PAM_NEW_AUTHTOK_REQD] = _PAM_ACTION_OK;
		_pam_set_default_control(actions, _PAM_ACTION_IGNORE);
	    } else if (!_pam_strCMP("sufficient", tok)) {
		D(("*PAM_F_SUFFICIENT*"));
		actions[PAM_SUCCESS] = _PAM_ACTION_DONE;
		actions[PAM_NEW_AUTHTOK_REQD] = _PAM_ACTION_DONE;
		_pam_set_default_control(actions, _PAM_ACTION_IGNORE);
	    } else if (!_pam_strCMP("include", tok)) {
		D(("*PAM_F_INCLUDE*"));
		pam_include = 1;
	    } else {
		D(("will need to parse %s", tok));
		_pam_parse_control(actions, tok);
		/* by default the default is to treat as failure */
		_pam_set_default_control(actions, _PAM_ACTION_BAD);
	    }

	    tok = _pam_StrTok(NULL, " \n\t", &nexttok);
	    if (pam_include) {
		if (_pam_load_conf_file(pamh, tok, this_service, module_type
#ifdef PAM_READ_BOTH_CONFS
					      , !other
#endif /* PAM_READ_BOTH_CONFS */
		    ) == PAM_SUCCESS)
		    continue;
		_pam_set_default_control(actions, _PAM_ACTION_BAD);
		mod_path = NULL;
		must_fail = 1;
		nexttok = NULL;
	    } else if (tok != NULL) {
		mod_path = tok;
		D(("mod_path = %s",mod_path));
	    } else {
		/* no module name given */
		D(("_pam_init_handlers: no module name supplied"));
		pam_syslog(pamh, LOG_ERR,
		           "(%s) no module name supplied", this_service);
		mod_path = NULL;
		must_fail = 1;
	    }

	    /* nexttok points to remaining arguments... */

	    if (nexttok != NULL) {
		D(("list: %s",nexttok));
	        argvlen = _pam_mkargv(nexttok, &argv, &argc);
		D(("argvlen = %d",argvlen));
	    } else {               /* there are no arguments so fix by hand */
		D(("_pam_init_handlers: empty argument list"));
		argvlen = argc = 0;
		argv = NULL;
	    }

#ifdef DEBUG
	    {
		int y;

		D(("CONF%s: %s%s %d %s %d"
		   , must_fail?"<*will fail*>":""
		   , this_service, other ? "(backup)":""
		   , module_type
		   , mod_path, argc));
		for (y = 0; y < argc; y++) {
		    D(("CONF: %s", argv[y]));
		}
		for (y = 0; y<_PAM_RETURN_VALUES; ++y) {
		    D(("RETURN %s(%d) -> %d %s",
		       _pam_token_returns[y], y, actions[y],
		       actions[y]>0 ? "jump":
			_pam_token_actions[-actions[y]]));
		}
	    }
#endif

	    res = _pam_add_handler(pamh, must_fail, other
				   , module_type, actions, mod_path
				   , argc, argv, argvlen);
	    if (res != PAM_SUCCESS) {
		pam_syslog(pamh, LOG_ERR, "error loading %s", mod_path);
		D(("failed to load module - aborting"));
		return PAM_ABORT;
	    }
	}
    }

    return ( (x < 0) ? PAM_ABORT:PAM_SUCCESS );
}

static int _pam_load_conf_file(pam_handle_t *pamh, const char *config_name
				, const char *service /* specific file */
				, int module_type /* specific type */
#ifdef PAM_READ_BOTH_CONFS
				, int not_other
#endif /* PAM_READ_BOTH_CONFS */
    )
{
    FILE *f;
    char *config_path = NULL;
    int retval = PAM_ABORT;

    D(("_pam_load_conf_file called"));

    if (config_name == NULL) {
	D(("no config file supplied"));
	pam_syslog(pamh, LOG_ERR, "(%s) no config file supplied", service);
	return PAM_ABORT;
    }

    if (config_name[0] != '/') {
	if (asprintf (&config_path, PAM_CONFIG_DF, config_name) < 0) {
	    pam_syslog(pamh, LOG_CRIT, "asprintf failed");
	    return PAM_BUF_ERR;
	}
	config_name = config_path;
    }

    D(("opening %s", config_name));
    f = fopen(config_name, "r");
    if (f != NULL) {
	retval = _pam_parse_conf_file(pamh, f, service, module_type
#ifdef PAM_READ_BOTH_CONFS
					      , not_other
#endif /* PAM_READ_BOTH_CONFS */
	    );
	fclose(f);
	if (retval != PAM_SUCCESS)
	    pam_syslog(pamh, LOG_ERR,
		       "_pam_load_conf_file: error reading %s: %s",
		       config_name, pam_strerror(pamh, retval));
    } else {
	D(("unable to open %s", config_name));
	pam_syslog(pamh, LOG_ERR,
		   "_pam_load_conf_file: unable to open %s",
		   config_name);
    }

    _pam_drop(config_path);
    return retval;
}

/* Parse config file, allocate handler structures, dlopen() */
int _pam_init_handlers(pam_handle_t *pamh)
{
    FILE *f;
    int retval;

    D(("_pam_init_handlers called"));
    IF_NO_PAMH("_pam_init_handlers",pamh,PAM_SYSTEM_ERR);

    /* Return immediately if everything is already loaded */
    if (pamh->handlers.handlers_loaded) {
	return PAM_SUCCESS;
    }

    D(("_pam_init_handlers: initializing"));

    /* First clean the service structure */

    _pam_free_handlers(pamh);
    if (! pamh->handlers.module) {
	if ((pamh->handlers.module =
	     malloc(MODULE_CHUNK * sizeof(struct loaded_module))) == NULL) {
	    pam_syslog(pamh, LOG_CRIT,
		       "_pam_init_handlers: no memory loading module");
	    return PAM_BUF_ERR;
	}
	pamh->handlers.modules_allocated = MODULE_CHUNK;
	pamh->handlers.modules_used = 0;
    }

    if (pamh->service_name == NULL) {
	return PAM_BAD_ITEM;                /* XXX - better error? */
    }

#ifdef PAM_LOCKING
    /* Is the PAM subsystem locked? */
    {
	 int fd_tmp;

	 if ((fd_tmp = open( PAM_LOCK_FILE, O_RDONLY )) != -1) {
	     pam_syslog(pamh, LOG_ERR,
			"_pam_init_handlers: PAM lockfile ("
			PAM_LOCK_FILE ") exists - aborting");
	      (void) close(fd_tmp);
	      /*
	       * to avoid swamping the system with requests
	       */
	      _pam_start_timer(pamh);
	      pam_fail_delay(pamh, 5000000);
	      _pam_await_timer(pamh, PAM_ABORT);

	      return PAM_ABORT;
	 }
    }
#endif /* PAM_LOCKING */

    /*
     * Now parse the config file(s) and add handlers
     */
    {
	struct stat test_d;

	/* Is there a PAM_CONFIG_D directory? */
	if ( stat(PAM_CONFIG_D, &test_d) == 0 && S_ISDIR(test_d.st_mode) ) {
	    char *filename;
	    int read_something=0;

	    D(("searching " PAM_CONFIG_D " for config files"));
	    if (asprintf(&filename, PAM_CONFIG_DF, pamh->service_name) < 0) {
		pam_syslog(pamh, LOG_ERR,
				"_pam_init_handlers: no memory; service %s",
				pamh->service_name);
		return PAM_BUF_ERR;
	    }
	    D(("opening %s", filename));
	    f = fopen(filename, "r");
	    if (f != NULL) {
		/* would test magic here? */
		retval = _pam_parse_conf_file(pamh, f, pamh->service_name, PAM_T_ANY
#ifdef PAM_READ_BOTH_CONFS
					      , 0
#endif /* PAM_READ_BOTH_CONFS */
		    );
		fclose(f);
		if (retval != PAM_SUCCESS) {
		    pam_syslog(pamh, LOG_ERR,
				    "_pam_init_handlers: error reading %s",
				    filename);
		    pam_syslog(pamh, LOG_ERR, "_pam_init_handlers: [%s]",
				    pam_strerror(pamh, retval));
		} else {
		    read_something = 1;
		}
	    } else {
		D(("unable to open %s", filename));
#ifdef PAM_READ_BOTH_CONFS
		D(("checking %s", PAM_CONFIG));

		if ((f = fopen(PAM_CONFIG,"r")) != NULL) {
		    retval = _pam_parse_conf_file(pamh, f, NULL, PAM_T_ANY, 1);
		    fclose(f);
		} else
#endif /* PAM_READ_BOTH_CONFS */
		retval = PAM_SUCCESS;
		/*
		 * XXX - should we log an error? Some people want to always
		 * use "other"
		 */
	    }
	    _pam_drop(filename);

	    if (retval == PAM_SUCCESS) {
		/* now parse the PAM_DEFAULT_SERVICE_FILE */

		D(("opening %s", PAM_DEFAULT_SERVICE_FILE));
		f = fopen(PAM_DEFAULT_SERVICE_FILE, "r");
		if (f != NULL) {
		    /* would test magic here? */
		    retval = _pam_parse_conf_file(pamh, f
						  , PAM_DEFAULT_SERVICE
						  , PAM_T_ANY
#ifdef PAM_READ_BOTH_CONFS
						  , 0
#endif /* PAM_READ_BOTH_CONFS */
			);
		    fclose(f);
		    if (retval != PAM_SUCCESS) {
			pam_syslog(pamh, LOG_ERR,
					"_pam_init_handlers: error reading %s",
					PAM_DEFAULT_SERVICE_FILE);
			pam_syslog(pamh, LOG_ERR,
					"_pam_init_handlers: [%s]",
					pam_strerror(pamh, retval));
		    } else {
			read_something = 1;
		    }
		} else {
		    D(("unable to open %s", PAM_DEFAULT_SERVICE_FILE));
		    pam_syslog(pamh, LOG_ERR,
				    "_pam_init_handlers: no default config %s",
				    PAM_DEFAULT_SERVICE_FILE);
		}
		if (!read_something) {          /* nothing read successfully */
		    retval = PAM_ABORT;
		}
	    }
	} else {
	    if ((f = fopen(PAM_CONFIG, "r")) == NULL) {
		pam_syslog(pamh, LOG_ERR, "_pam_init_handlers: could not open "
				PAM_CONFIG );
		return PAM_ABORT;
	    }

	    retval = _pam_parse_conf_file(pamh, f, NULL, PAM_T_ANY
#ifdef PAM_READ_BOTH_CONFS
					  , 0
#endif /* PAM_READ_BOTH_CONFS */
		);

	    D(("closing configuration file"));
	    fclose(f);
	}
    }

    if (retval != PAM_SUCCESS) {
	/* Read error */
	pam_syslog(pamh, LOG_ERR, "error reading PAM configuration file");
	return PAM_ABORT;
    }

    pamh->handlers.handlers_loaded = 1;

    D(("_pam_init_handlers exiting"));
    return PAM_SUCCESS;
}

/*
 * This is where we read a line of the PAM config file. The line may be
 * preceeded by lines of comments and also extended with "\\\n"
 */

static int _pam_assemble_line(FILE *f, char *buffer, int buf_len)
{
    char *p = buffer;
    char *s, *os;
    int used = 0;

    /* loop broken with a 'break' when a non-'\\n' ended line is read */

    D(("called."));
    for (;;) {
	if (used >= buf_len) {
	    /* Overflow */
	    D(("_pam_assemble_line: overflow"));
	    return -1;
	}
	if (fgets(p, buf_len - used, f) == NULL) {
	    if (used) {
		/* Incomplete read */
		return -1;
	    } else {
		/* EOF */
		return 0;
	    }
	}

	/* skip leading spaces --- line may be blank */

	s = p + strspn(p, " \n\t");
	if (*s && (*s != '#')) {
	    os = s;

	    /*
	     * we are only interested in characters before the first '#'
	     * character
	     */

	    while (*s && *s != '#')
		 ++s;
	    if (*s == '#') {
		 *s = '\0';
		 used += strlen(os);
		 break;                /* the line has been read */
	    }

	    s = os;

	    /*
	     * Check for backslash by scanning back from the end of
	     * the entered line, the '\n' has been included since
	     * normally a line is terminated with this
	     * character. fgets() should only return one though!
	     */

	    s += strlen(s);
	    while (s > os && ((*--s == ' ') || (*s == '\t')
			      || (*s == '\n')));

	    /* check if it ends with a backslash */
	    if (*s == '\\') {
		*s++ = ' ';             /* replace backslash with ' ' */
		*s = '\0';              /* truncate the line here */
		used += strlen(os);
		p = s;                  /* there is more ... */
	    } else {
		/* End of the line! */
		used += strlen(os);
		break;                  /* this is the complete line */
	    }

	} else {
	    /* Nothing in this line */
	    /* Don't move p         */
	}
    }

    return used;
}

static char *
extract_modulename(const char *mod_path)
{
  const char *p = strrchr (mod_path, '/');
  char *dot, *retval;

  if (p == NULL)
    p = mod_path;
  else
    p++;

  if ((retval = strdup (p)) == NULL)
    return NULL;

  dot = strrchr (retval, '.');
  if (dot)
    *dot = '\0';

  return retval;
}

int _pam_add_handler(pam_handle_t *pamh
		     , int must_fail, int other, int type
		     , int *actions, const char *mod_path
		     , int argc, char **argv, int argvlen)
{
    struct loaded_module *mod;
    int x = 0;
    struct handler **handler_p;
    struct handler **handler_p2;
    struct handlers *the_handlers;
    const char *sym, *sym2;
    char *mod_full_path=NULL;
#ifdef PAM_DYNAMIC
    char *mod_full_isa_path=NULL, *isa=NULL;
#endif
    servicefn func, func2;
    int success;

    D(("called."));
    IF_NO_PAMH("_pam_add_handler",pamh,PAM_SYSTEM_ERR);

    /* if NULL set to something that can be searched for */
    switch (mod_path != NULL) {
    default:
	if (mod_path[0] == '/') {
	    break;
	}
	if (asprintf(&mod_full_path, "%s%s",
		     DEFAULT_MODULE_PATH, mod_path) >= 0) {
	    mod_path = mod_full_path;
	    break;
	}
	mod_full_path = NULL;
	pam_syslog(pamh, LOG_CRIT, "cannot malloc full mod path");
    case 0:
	mod_path = UNKNOWN_MODULE_PATH;
    }

    D(("_pam_add_handler: adding type %d, module `%s'",type,mod_path));
    mod  = pamh->handlers.module;

    /* First, ensure the module is loaded */
    while (x < pamh->handlers.modules_used) {
	if (!strcmp(mod[x].name, mod_path)) {  /* case sensitive ! */
	    break;
	}
	x++;
    }
    if (x == pamh->handlers.modules_used) {
	/* Not found */
	if (pamh->handlers.modules_allocated == pamh->handlers.modules_used) {
	    /* will need more memory */
	    void *tmp = realloc(pamh->handlers.module,
                               (pamh->handlers.modules_allocated+MODULE_CHUNK)
                               *sizeof(struct loaded_module));
	    if (tmp == NULL) {
		D(("cannot enlarge module pointer memory"));
		pam_syslog(pamh, LOG_ERR,
				"realloc returned NULL in _pam_add_handler");
		_pam_drop(mod_full_path);
		return PAM_ABORT;
	    }
	    pamh->handlers.module = tmp;
	    pamh->handlers.modules_allocated += MODULE_CHUNK;
	}
	mod = &(pamh->handlers.module[x]);
	/* Be pessimistic... */
	success = PAM_ABORT;

#ifdef PAM_DYNAMIC
	D(("_pam_add_handler: _pam_dlopen(%s)", mod_path));
	mod->dl_handle = _pam_dlopen(mod_path);
	D(("_pam_add_handler: _pam_dlopen'ed"));
	D(("_pam_add_handler: dlopen'ed"));
	if (mod->dl_handle == NULL) {
	    if (strstr(mod_path, "$ISA")) {
		mod_full_isa_path = malloc(strlen(mod_path) + strlen(_PAM_ISA) + 1);
		if (mod_full_isa_path == NULL) {
		    D(("_pam_handler: couldn't get memory for mod_path"));
		    pam_syslog(pamh, LOG_ERR, "no memory for module path");
		    success = PAM_ABORT;
		} else {
		    strcpy(mod_full_isa_path, mod_path);
                    isa = strstr(mod_full_isa_path, "$ISA");
		    if (isa) {
		        memmove(isa + strlen(_PAM_ISA), isa + 4, strlen(isa + 4) + 1);
		        memmove(isa, _PAM_ISA, strlen(_PAM_ISA));
		    }
		    mod->dl_handle = _pam_dlopen(mod_full_isa_path);
		    _pam_drop(mod_full_isa_path);
		}
	    }
	}
	if (mod->dl_handle == NULL) {
	    D(("_pam_add_handler: _pam_dlopen(%s) failed", mod_path));
	    pam_syslog(pamh, LOG_ERR, "unable to dlopen(%s)", mod_path);
	    pam_syslog(pamh, LOG_ERR, "[error: %s]", _pam_dlerror());
	    /* Don't abort yet; static code may be able to find function.
	     * But defaults to abort if nothing found below... */
	} else {
	    D(("module added successfully"));
	    success = PAM_SUCCESS;
	    mod->type = PAM_MT_DYNAMIC_MOD;
	    pamh->handlers.modules_used++;
	}
#endif
#ifdef PAM_STATIC
	/* Only load static function if function was not found dynamically.
	 * This code should work even if no dynamic loading is available. */
	if (success != PAM_SUCCESS) {
	    D(("_pam_add_handler: open static handler %s", mod_path));
	    mod->dl_handle = _pam_open_static_handler(mod_path);
	    if (mod->dl_handle == NULL) {
	        D(("_pam_add_handler: unable to find static handler %s",
		   mod_path));
		pam_syslog(pamh, LOG_ERR,
				"unable to open static handler %s", mod_path);
		/* Didn't find module in dynamic or static..will mark bad */
	    } else {
	        D(("static module added successfully"));
		success = PAM_SUCCESS;
		mod->type = PAM_MT_STATIC_MOD;
		pamh->handlers.modules_used++;
	    }
	}
#endif

	if (success != PAM_SUCCESS) {	         /* add a malformed module */
	    mod->dl_handle = NULL;
	    mod->type = PAM_MT_FAULTY_MOD;
	    pamh->handlers.modules_used++;
	    pam_syslog(pamh, LOG_ERR, "adding faulty module: %s", mod_path);
	    success = PAM_SUCCESS;  /* We have successfully added a module */
	}

	/* indicate its name - later we will search for it by this */
	if ((mod->name = _pam_strdup(mod_path)) == NULL) {
	    D(("_pam_handler: couldn't get memory for mod_path"));
	    pam_syslog(pamh, LOG_ERR, "no memory for module path");
	    success = PAM_ABORT;
	}

    } else {                           /* x != pamh->handlers.modules_used */
	mod += x;                                    /* the located module */
	success = PAM_SUCCESS;
    }

    _pam_drop(mod_full_path);
    mod_path = NULL;                        /* no longer needed or trusted */

    /* Now return error if necessary after trying all possible ways... */
    if (success != PAM_SUCCESS)
	return(success);

    /*
     * At this point 'mod' points to the stored/loaded module. If its
     * dl_handle is unknown, then we must be able to indicate dispatch
     * failure with 'must_fail'
     */

    /* Now define the handler(s) based on mod->dlhandle and type */

    /* decide which list of handlers to use */
    the_handlers = (other) ? &pamh->handlers.other : &pamh->handlers.conf;

    handler_p = handler_p2 = NULL;
    func = func2 = NULL;
    sym2 = NULL;

    /* point handler_p's at the root addresses of the function stacks */
    switch (type) {
    case PAM_T_AUTH:
	handler_p = &the_handlers->authenticate;
	sym = "pam_sm_authenticate";
	handler_p2 = &the_handlers->setcred;
	sym2 = "pam_sm_setcred";
	break;
    case PAM_T_SESS:
	handler_p = &the_handlers->open_session;
	sym = "pam_sm_open_session";
	handler_p2 = &the_handlers->close_session;
	sym2 = "pam_sm_close_session";
	break;
    case PAM_T_ACCT:
	handler_p = &the_handlers->acct_mgmt;
	sym = "pam_sm_acct_mgmt";
	break;
    case PAM_T_PASS:
	handler_p = &the_handlers->chauthtok;
	sym = "pam_sm_chauthtok";
	break;
    default:
	/* Illegal module type */
	D(("_pam_add_handler: illegal module type %d", type));
	return PAM_ABORT;
    }

    /* are the modules reliable? */
    if (
#ifdef PAM_DYNAMIC
	 mod->type != PAM_MT_DYNAMIC_MOD
	 &&
#endif /* PAM_DYNAMIC */
#ifdef PAM_STATIC
	 mod->type != PAM_MT_STATIC_MOD
	 &&
#endif /* PAM_STATIC */
	 mod->type != PAM_MT_FAULTY_MOD
	) {
	D(("_pam_add_handlers: illegal module library type; %d", mod->type));
	pam_syslog(pamh, LOG_ERR,
			"internal error: module library type not known: %s;%d",
			sym, mod->type);
	return PAM_ABORT;
    }

    /* now identify this module's functions - for non-faulty modules */

#ifdef PAM_DYNAMIC
    if ((mod->type == PAM_MT_DYNAMIC_MOD) && 
        !(func = _pam_dlsym(mod->dl_handle, sym)) ) {
	pam_syslog(pamh, LOG_ERR, "unable to resolve symbol: %s", sym);
    }
#endif
#ifdef PAM_STATIC
    if ((mod->type == PAM_MT_STATIC_MOD) &&
        (func = (servicefn)_pam_get_static_sym(mod->dl_handle, sym)) == NULL) {
	pam_syslog(pamh, LOG_ERR, "unable to resolve static symbol: %s", sym);
    }
#endif
    if (sym2) {
#ifdef PAM_DYNAMIC
	if ((mod->type == PAM_MT_DYNAMIC_MOD) &&
	    !(func2 = _pam_dlsym(mod->dl_handle, sym2)) ) {
	    pam_syslog(pamh, LOG_ERR, "unable to resolve symbol: %s", sym2);
	}
#endif
#ifdef PAM_STATIC
	if ((mod->type == PAM_MT_STATIC_MOD) &&
	    (func2 = (servicefn)_pam_get_static_sym(mod->dl_handle, sym2))
	    == NULL) {
	    pam_syslog(pamh, LOG_ERR, "unable to resolve symbol: %s", sym2);
	}
#endif
    }

    /* here func (and perhaps func2) point to the appropriate functions */

    /* add new handler to end of existing list */
    while (*handler_p != NULL) {
	handler_p = &((*handler_p)->next);
    }

    if ((*handler_p = malloc(sizeof(struct handler))) == NULL) {
	pam_syslog(pamh, LOG_CRIT, "cannot malloc struct handler #1");
	return (PAM_ABORT);
    }

    (*handler_p)->must_fail = must_fail;        /* failure forced? */
    (*handler_p)->func = func;
    memcpy((*handler_p)->actions,actions,sizeof((*handler_p)->actions));
    (*handler_p)->cached_retval = _PAM_INVALID_RETVAL;
    (*handler_p)->cached_retval_p = &((*handler_p)->cached_retval);
    (*handler_p)->argc = argc;
    (*handler_p)->argv = argv;                       /* not a copy */
    (*handler_p)->mod_name = extract_modulename(mod->name);
    (*handler_p)->next = NULL;

    /* some of the modules have a second calling function */
    if (handler_p2) {
	/* add new handler to end of existing list */
	while (*handler_p2) {
	    handler_p2 = &((*handler_p2)->next);
	}

	if ((*handler_p2 = malloc(sizeof(struct handler))) == NULL) {
	    pam_syslog(pamh, LOG_CRIT, "cannot malloc struct handler #2");
	    return (PAM_ABORT);
	}

	(*handler_p2)->must_fail = must_fail;        /* failure forced? */
	(*handler_p2)->func = func2;
	memcpy((*handler_p2)->actions,actions,sizeof((*handler_p2)->actions));
	(*handler_p2)->cached_retval =  _PAM_INVALID_RETVAL;     /* ignored */
	/* Note, this next entry points to the handler_p value! */
	(*handler_p2)->cached_retval_p = &((*handler_p)->cached_retval);
	(*handler_p2)->argc = argc;
	if (argv) {
	    if (((*handler_p2)->argv = malloc(argvlen)) == NULL) {
		pam_syslog(pamh, LOG_CRIT, "cannot malloc argv for handler #2");
		return (PAM_ABORT);
	    }
	    memcpy((*handler_p2)->argv, argv, argvlen);
	} else {
	    (*handler_p2)->argv = NULL;              /* no arguments */
	}
	(*handler_p2)->mod_name = extract_modulename(mod->name);
	(*handler_p2)->next = NULL;
    }

    D(("_pam_add_handler: returning successfully"));

    return PAM_SUCCESS;
}

/* Free various allocated structures and dlclose() the libs */
int _pam_free_handlers(pam_handle_t *pamh)
{
    struct loaded_module *mod;

    D(("called."));
    IF_NO_PAMH("_pam_free_handlers",pamh,PAM_SYSTEM_ERR);

    mod = pamh->handlers.module;

    /* Close all loaded modules */

    while (pamh->handlers.modules_used) {
	D(("_pam_free_handlers: dlclose(%s)", mod->name));
	free(mod->name);
#ifdef PAM_DYNAMIC
	if (mod->type == PAM_MT_DYNAMIC_MOD) {
	    _pam_dlclose(mod->dl_handle);
	}
#endif
	mod++;
	pamh->handlers.modules_used--;
    }

    /* Free all the handlers */

    _pam_free_handlers_aux(&(pamh->handlers.conf.authenticate));
    _pam_free_handlers_aux(&(pamh->handlers.conf.setcred));
    _pam_free_handlers_aux(&(pamh->handlers.conf.acct_mgmt));
    _pam_free_handlers_aux(&(pamh->handlers.conf.open_session));
    _pam_free_handlers_aux(&(pamh->handlers.conf.close_session));
    _pam_free_handlers_aux(&(pamh->handlers.conf.chauthtok));

    _pam_free_handlers_aux(&(pamh->handlers.other.authenticate));
    _pam_free_handlers_aux(&(pamh->handlers.other.setcred));
    _pam_free_handlers_aux(&(pamh->handlers.other.acct_mgmt));
    _pam_free_handlers_aux(&(pamh->handlers.other.open_session));
    _pam_free_handlers_aux(&(pamh->handlers.other.close_session));
    _pam_free_handlers_aux(&(pamh->handlers.other.chauthtok));

    /* no more loaded modules */

    _pam_drop(pamh->handlers.module);

    /* Indicate that handlers are not initialized for this pamh */

    pamh->handlers.handlers_loaded = 0;

    return PAM_SUCCESS;
}

void _pam_start_handlers(pam_handle_t *pamh)
{
    D(("called."));
    /* NB. There is no check for a NULL pamh here, since no return
     * value to communicate the fact!  */

    /* Indicate that handlers are not initialized for this pamh */
    pamh->handlers.handlers_loaded = 0;

    pamh->handlers.modules_allocated = 0;
    pamh->handlers.modules_used = 0;
    pamh->handlers.module = NULL;

    /* initialize the .conf and .other entries */

    pamh->handlers.conf.authenticate = NULL;
    pamh->handlers.conf.setcred = NULL;
    pamh->handlers.conf.acct_mgmt = NULL;
    pamh->handlers.conf.open_session = NULL;
    pamh->handlers.conf.close_session = NULL;
    pamh->handlers.conf.chauthtok = NULL;

    pamh->handlers.other.authenticate = NULL;
    pamh->handlers.other.setcred = NULL;
    pamh->handlers.other.acct_mgmt = NULL;
    pamh->handlers.other.open_session = NULL;
    pamh->handlers.other.close_session = NULL;
    pamh->handlers.other.chauthtok = NULL;
}

void _pam_free_handlers_aux(struct handler **hp)
{
    struct handler *h = *hp;
    struct handler *last;

    D(("called."));
    while (h) {
	last = h;
	_pam_drop(h->argv);  /* This is all alocated in a single chunk */
	_pam_drop(h->mod_name);
	h = h->next;
	memset(last, 0, sizeof(*last));
	free(last);
    }

    *hp = NULL;
}
