/* pam_mail module */

/*
 * $Id: pam_mail.c,v 1.17 2005/12/12 12:47:38 t8m Exp $
 *
 * Written by Andrew Morgan <morgan@linux.kernel.org> 1996/3/11
 * $HOME additions by David Kinchlea <kinch@kinch.ark.com> 1997/1/7
 * mailhash additions by Chris Adams <cadams@ro.com> 1998/7/11
 */

#include "config.h"

#include <ctype.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#define DEFAULT_MAIL_DIRECTORY    PAM_PATH_MAILDIR
#define MAIL_FILE_FORMAT          "%s%s/%s"
#define MAIL_ENV_NAME             "MAIL"
#define MAIL_ENV_FORMAT           MAIL_ENV_NAME "=%s"

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_SESSION
#define PAM_SM_AUTH

#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_modutil.h>
#include <security/pam_ext.h>

/* argument parsing */

#define PAM_DEBUG_ARG		0x0001
#define PAM_NO_LOGIN		0x0002
#define PAM_LOGOUT_TOO		0x0004
#define PAM_NEW_MAIL_DIR	0x0010
#define PAM_MAIL_SILENT		0x0020
#define PAM_NO_ENV		0x0040
#define PAM_HOME_MAIL		0x0100
#define PAM_EMPTY_TOO		0x0200
#define PAM_STANDARD_MAIL	0x0400
#define PAM_QUIET_MAIL		0x1000

#define HAVE_NEW_MAIL           0x1
#define HAVE_OLD_MAIL           0x2
#define HAVE_NO_MAIL            0x3
#define HAVE_MAIL               0x4

static int
_pam_parse (const pam_handle_t *pamh, int flags, int argc,
	    const char **argv, const char **maildir, size_t *hashcount)
{
    int ctrl=0;

    if (flags & PAM_SILENT) {
	ctrl |= PAM_MAIL_SILENT;
    }

    *hashcount = 0;

    /* step through arguments */
    for (; argc-- > 0; ++argv) {

	/* generic options */

	if (!strcmp(*argv,"debug"))
	    ctrl |= PAM_DEBUG_ARG;
	else if (!strcmp(*argv,"quiet"))
	    ctrl |= PAM_QUIET_MAIL;
	else if (!strcmp(*argv,"standard"))
	    ctrl |= PAM_STANDARD_MAIL | PAM_EMPTY_TOO;
	else if (!strncmp(*argv,"dir=",4)) {
	    *maildir = 4 + *argv;
	    if (**maildir != '\0') {
		D(("new mail directory: %s", *maildir));
		ctrl |= PAM_NEW_MAIL_DIR;
	    } else {
		pam_syslog(pamh, LOG_ERR,
			   "dir= specification missing argument - ignored");
	    }
	} else if (!strncmp(*argv,"hash=",5)) {
	    char *ep = NULL;
	    *hashcount = strtoul(*argv+5,&ep,10);
	    if (!ep) {
		*hashcount = 0;
	    }
	} else if (!strcmp(*argv,"close")) {
	    ctrl |= PAM_LOGOUT_TOO;
	} else if (!strcmp(*argv,"nopen")) {
	    ctrl |= PAM_NO_LOGIN;
	} else if (!strcmp(*argv,"noenv")) {
	    ctrl |= PAM_NO_ENV;
	} else if (!strcmp(*argv,"empty")) {
	    ctrl |= PAM_EMPTY_TOO;
	} else {
	    pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
	}
    }

    if ((*hashcount != 0) && !(ctrl & PAM_NEW_MAIL_DIR)) {
	*maildir = DEFAULT_MAIL_DIRECTORY;
	ctrl |= PAM_NEW_MAIL_DIR;
    }

    return ctrl;
}

static int
get_folder(pam_handle_t *pamh, int ctrl,
	   const char *path_mail, char **folder_p, size_t hashcount)
{
    int retval;
    const char *user, *path;
    char *folder = NULL;
    const struct passwd *pwd = NULL;

    retval = pam_get_user(pamh, &user, NULL);
    if (retval != PAM_SUCCESS || user == NULL) {
	pam_syslog(pamh, LOG_ERR, "cannot determine username");
	retval = PAM_USER_UNKNOWN;
	goto get_folder_cleanup;
    }

    if (ctrl & PAM_NEW_MAIL_DIR) {
	path = path_mail;
	if (*path == '~') {	/* support for $HOME delivery */
	    pwd = pam_modutil_getpwnam(pamh, user);
	    if (pwd == NULL) {
		pam_syslog(pamh, LOG_ERR, "user unknown");
		retval = PAM_USER_UNKNOWN;
		goto get_folder_cleanup;
	    }
	    /*
	     * "~/xxx" and "~xxx" are treated as same
	     */
	    if (!*++path || (*path == '/' && !*++path)) {
		pam_syslog(pamh, LOG_ERR,
			   "badly formed mail path [%s]", path_mail);
		retval = PAM_SERVICE_ERR;
		goto get_folder_cleanup;
	    }
	    ctrl |= PAM_HOME_MAIL;
	    if (hashcount != 0) {
		pam_syslog(pamh, LOG_ERR,
			   "cannot do hash= and home directory mail");
	    }
	}
    } else {
	path = DEFAULT_MAIL_DIRECTORY;
    }

    /* put folder together */

    hashcount = hashcount < strlen(user) ? hashcount : strlen(user);

    retval = PAM_BUF_ERR;
    if (ctrl & PAM_HOME_MAIL) {
	if (asprintf(&folder, MAIL_FILE_FORMAT, pwd->pw_dir, "", path) < 0)
	    goto get_folder_cleanup;
    } else {
	int rc;
	size_t i;
	char *hash;

	if ((hash = malloc(2 * hashcount + 1)) == NULL)
	    goto get_folder_cleanup;

	for (i = 0; i < hashcount; i++) {
	    hash[2 * i] = '/';
	    hash[2 * i + 1] = user[i];
	}
	hash[2 * i] = '\0';

	rc = asprintf(&folder, MAIL_FILE_FORMAT, path, hash, user);
	_pam_overwrite(hash);
	_pam_drop(hash);
	if (rc < 0)
	    goto get_folder_cleanup;
    }
    D(("folder=[%s]", folder));
    retval = PAM_SUCCESS;

    /* tidy up */

  get_folder_cleanup:
    user = NULL;
    path = NULL;

    *folder_p = folder;
    folder = NULL;

    if (retval == PAM_BUF_ERR)
	pam_syslog(pamh, LOG_CRIT, "out of memory for mail folder");

    return retval;
}

static int
get_mail_status(pam_handle_t *pamh, int ctrl, const char *folder)
{
    int type = 0;
    struct stat mail_st;

    if (stat(folder, &mail_st) < 0)
	return 0;

    if (S_ISDIR(mail_st.st_mode)) {	/* Assume Maildir format */
	int i, save_errno;
	char *dir;
	struct dirent **namelist;

	if (asprintf(&dir, "%s/new", folder) < 0) {
	    pam_syslog(pamh, LOG_CRIT, "out of memory");
	    goto get_mail_status_cleanup;
	}
	i = scandir(dir, &namelist, 0, alphasort);
	save_errno = errno;
	_pam_overwrite(dir);
	_pam_drop(dir);
	if (i < 0) {
	    type = 0;
	    namelist = NULL;
	    if (save_errno == ENOMEM) {
		pam_syslog(pamh, LOG_CRIT, "out of memory");
		goto get_mail_status_cleanup;
	    }
	}
	type = (i > 2) ? HAVE_NEW_MAIL : 0;
	while (--i >= 0)
	    _pam_drop(namelist[i]);
	_pam_drop(namelist);
	if (type == 0) {
	    if (asprintf(&dir, "%s/cur", folder) < 0) {
		pam_syslog(pamh, LOG_CRIT, "out of memory");
		goto get_mail_status_cleanup;
	    }
	    i = scandir(dir, &namelist, 0, alphasort);
	    save_errno = errno;
	    _pam_overwrite(dir);
	    _pam_drop(dir);
	    if (i < 0) {
		type = 0;
		namelist = NULL;
		if (save_errno == ENOMEM) {
		    pam_syslog(pamh, LOG_CRIT, "out of memory");
		    goto get_mail_status_cleanup;
		}
	    }
	    if (i > 2)
	        type = HAVE_OLD_MAIL;
	    else
	        type = (ctrl & PAM_EMPTY_TOO) ? HAVE_NO_MAIL : 0;
	    while (--i >= 0)
		_pam_drop(namelist[i]);
	    _pam_drop(namelist);
	}
    } else {
	if (mail_st.st_size > 0) {
	    if (mail_st.st_atime < mail_st.st_mtime)	/* new */
	        type = HAVE_NEW_MAIL;
	    else		/* old */
	        type = (ctrl & PAM_STANDARD_MAIL) ? HAVE_MAIL : HAVE_OLD_MAIL;
	} else if (ctrl & PAM_EMPTY_TOO) {
	    type = HAVE_NO_MAIL;
	} else {
	    type = 0;
	}
    }

  get_mail_status_cleanup:
    memset(&mail_st, 0, sizeof(mail_st));
    D(("user has %d mail in %s folder", type, folder));
    return type;
}

static int
report_mail(pam_handle_t *pamh, int ctrl, int type, const char *folder)
{
    int retval;

    if (!(ctrl & PAM_MAIL_SILENT) ||
	((ctrl & PAM_QUIET_MAIL) && type == HAVE_NEW_MAIL))
      {
	if (ctrl & PAM_STANDARD_MAIL)
	  switch (type)
	    {
	    case HAVE_NO_MAIL:
	      retval = pam_info (pamh, "%s", _("No mail."));
	      break;
	    case HAVE_NEW_MAIL:
	      retval = pam_info (pamh, "%s", _("You have new mail."));
	      break;
	    case HAVE_OLD_MAIL:
	      retval = pam_info (pamh, "%s", _("You have old mail."));
	      break;
	    case HAVE_MAIL:
	    default:
	      retval = pam_info (pamh, "%s", _("You have mail."));
	      break;
	    }
	else
	  switch (type)
	    {
	    case HAVE_NO_MAIL:
	      retval = pam_info (pamh, _("You have no mail in folder %s."),
				 folder);
	      break;
	    case HAVE_NEW_MAIL:
	      retval = pam_info (pamh, _("You have new mail in folder %s."),
				 folder);
	      break;
	    case HAVE_OLD_MAIL:
	      retval = pam_info (pamh, _("You have old mail in folder %s."),
				 folder);
	      break;
	    case HAVE_MAIL:
	    default:
	      retval = pam_info (pamh, _("You have mail in folder %s."),
				 folder);
	      break;
	    }
      }
    else
      {
	D(("keeping quiet"));
	retval = PAM_SUCCESS;
      }

    D(("returning %s", pam_strerror(pamh, retval)));
    return retval;
}

static int _do_mail(pam_handle_t *, int, int, const char **, int);

/* --- authentication functions --- */

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t *pamh UNUSED, int flags UNUSED,
		     int argc UNUSED, const char **argv UNUSED)
{
    return PAM_IGNORE;
}

/* Checking mail as part of authentication */
PAM_EXTERN
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc,
    const char **argv)
{
    if (!(flags & (PAM_ESTABLISH_CRED|PAM_DELETE_CRED)))
      return PAM_IGNORE;
    return _do_mail(pamh,flags,argc,argv,(flags & PAM_ESTABLISH_CRED));
}

/* --- session management functions --- */

PAM_EXTERN
int pam_sm_close_session(pam_handle_t *pamh,int flags,int argc
			 ,const char **argv)
{
    return _do_mail(pamh,flags,argc,argv,0);
}

/* Checking mail as part of the session management */
PAM_EXTERN
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc,
    const char **argv)
{
    return _do_mail(pamh,flags,argc,argv,1);
}


/* --- The Beaf (Tm) --- */

static int _do_mail(pam_handle_t *pamh, int flags, int argc,
    const char **argv, int est)
{
    int retval, ctrl, type;
    size_t hashcount;
    char *folder = NULL;
    const char *path_mail = NULL;

    /*
     * this module (un)sets the MAIL environment variable, and checks if
     * the user has any new mail.
     */

    ctrl = _pam_parse(pamh, flags, argc, argv, &path_mail, &hashcount);

    /* Do we have anything to do? */

    if (flags & PAM_SILENT)
	return PAM_SUCCESS;

    /* which folder? */

    retval = get_folder(pamh, ctrl, path_mail, &folder, hashcount);
    if (retval != PAM_SUCCESS) {
	D(("failed to find folder"));
	return retval;
    }

    /* set the MAIL variable? */

    if (!(ctrl & PAM_NO_ENV) && est) {
	char *tmp;

	if (asprintf(&tmp, MAIL_ENV_FORMAT, folder) < 0) {
	    pam_syslog(pamh, LOG_CRIT,
		       "no memory for " MAIL_ENV_NAME " variable");
	    retval = PAM_BUF_ERR;
	    goto do_mail_cleanup;
	}
	D(("setting env: %s", tmp));
	retval = pam_putenv(pamh, tmp);
	_pam_overwrite(tmp);
	_pam_drop(tmp);
	if (retval != PAM_SUCCESS) {
	    pam_syslog(pamh, LOG_CRIT,
		       "unable to set " MAIL_ENV_NAME " variable");
	    retval = PAM_BUF_ERR;
	    goto do_mail_cleanup;
	}
    } else {
	D(("not setting " MAIL_ENV_NAME " variable"));
    }

    /*
     * OK. we've got the mail folder... what about its status?
     */

    if ((est && !(ctrl & PAM_NO_LOGIN))
	|| (!est && (ctrl & PAM_LOGOUT_TOO))) {
	type = get_mail_status(pamh, ctrl, folder);
	if (type != 0) {
	    retval = report_mail(pamh, ctrl, type, folder);
	    type = 0;
	}
    }

    /* Delete environment variable? */
    if ( ! est && ! (ctrl & PAM_NO_ENV) )
	(void) pam_putenv(pamh, MAIL_ENV_NAME);

  do_mail_cleanup:
    _pam_overwrite(folder);
    _pam_drop(folder);

    /* indicate success or failure */

    return retval;
}

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_mail_modstruct = {
     "pam_mail",
     pam_sm_authenticate,
     pam_sm_setcred,
     NULL,
     pam_sm_open_session,
     pam_sm_close_session,
     NULL,
};

#endif

/* end of module definition */
