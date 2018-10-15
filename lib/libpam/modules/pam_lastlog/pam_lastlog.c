/* pam_lastlog module */

/*
 * Written by Andrew Morgan <morgan@linux.kernel.org> 1996/3/11
 *
 * This module does the necessary work to display the last login
 * time+date for this user, it then updates this entry for the
 * present (login) service.
 */

#include "config.h"

#include <fcntl.h>
#include <time.h>
#ifdef HAVE_UTMP_H
# include <utmp.h>
#else
# include <lastlog.h>
#endif
#include <pwd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <paths.h>

#if defined(hpux) || defined(sunos) || defined(solaris)
# ifndef _PATH_LASTLOG
#  define _PATH_LASTLOG "/usr/adm/lastlog"
# endif /* _PATH_LASTLOG */
# ifndef UT_HOSTSIZE
#  define UT_HOSTSIZE 16
# endif /* UT_HOSTSIZE */
# ifndef UT_LINESIZE
#  define UT_LINESIZE 12
# endif /* UT_LINESIZE */
#endif
#if defined(hpux)
struct lastlog {
    time_t  ll_time;
    char    ll_line[UT_LINESIZE];
    char    ll_host[UT_HOSTSIZE];            /* same as in utmp */
};
#endif /* hpux */

/* XXX - time before ignoring lock. Is 1 sec enough? */
#define LASTLOG_IGNORE_LOCK_TIME     1

#define DEFAULT_HOST     ""  /* "[no.where]" */
#define DEFAULT_TERM     ""  /* "tt???" */

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_SESSION

#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_modutil.h>
#include <security/pam_ext.h>

/* argument parsing */

#define LASTLOG_DATE        01  /* display the date of the last login */
#define LASTLOG_HOST        02  /* display the last host used (if set) */
#define LASTLOG_LINE        04  /* display the last terminal used */
#define LASTLOG_NEVER      010  /* display a welcome message for first login */
#define LASTLOG_DEBUG      020  /* send info to syslog(3) */
#define LASTLOG_QUIET      040  /* keep quiet about things */
#define LASTLOG_WTMP      0100  /* log to wtmp as well as lastlog */

static int
_pam_parse(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    int ctrl=(LASTLOG_DATE|LASTLOG_HOST|LASTLOG_LINE|LASTLOG_WTMP);

    /* does the appliction require quiet? */
    if (flags & PAM_SILENT) {
	ctrl |= LASTLOG_QUIET;
    }

    /* step through arguments */
    for (; argc-- > 0; ++argv) {

	/* generic options */

	if (!strcmp(*argv,"debug")) {
	    ctrl |= LASTLOG_DEBUG;
	} else if (!strcmp(*argv,"nodate")) {
	    ctrl &= ~LASTLOG_DATE;
	} else if (!strcmp(*argv,"noterm")) {
	    ctrl &= ~LASTLOG_LINE;
	} else if (!strcmp(*argv,"nohost")) {
	    ctrl &= ~LASTLOG_HOST;
	} else if (!strcmp(*argv,"silent")) {
	    ctrl |= LASTLOG_QUIET;
	} else if (!strcmp(*argv,"never")) {
	    ctrl |= LASTLOG_NEVER;
	} else if (!strcmp(*argv,"nowtmp")) {
	    ctrl &= ~LASTLOG_WTMP;
	} else {
	    pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
	}
    }

    D(("ctrl = %o", ctrl));
    return ctrl;
}

static const char *
get_tty(pam_handle_t *pamh)
{
    const void *void_terminal_line = NULL;
    const char *terminal_line;

    if (pam_get_item(pamh, PAM_TTY, &void_terminal_line) != PAM_SUCCESS
	|| void_terminal_line == NULL) {
	terminal_line = DEFAULT_TERM;
    } else {
	terminal_line = void_terminal_line;
    }
    if (!strncmp("/dev/", terminal_line, 5)) {
	/* strip leading "/dev/" from tty. */
	terminal_line += 5;
    }
    D(("terminal = %s", terminal_line));
    return terminal_line;
}

static int
last_login_read(pam_handle_t *pamh, int announce, int last_fd, uid_t uid)
{
    struct flock last_lock;
    struct lastlog last_login;
    int retval = PAM_SUCCESS;
    char the_time[256];
    char *date = NULL;
    char *host = NULL;
    char *line = NULL;

    memset(&last_lock, 0, sizeof(last_lock));
    last_lock.l_type = F_RDLCK;
    last_lock.l_whence = SEEK_SET;
    last_lock.l_start = sizeof(last_login) * (off_t) uid;
    last_lock.l_len = sizeof(last_login);

    if (fcntl(last_fd, F_SETLK, &last_lock) < 0) {
        D(("locking %s failed..(waiting a little)", _PATH_LASTLOG));
	pam_syslog(pamh, LOG_WARNING,
		   "file %s is locked/read", _PATH_LASTLOG);
	sleep(LASTLOG_IGNORE_LOCK_TIME);
    }

    if (pam_modutil_read(last_fd, (char *) &last_login,
			 sizeof(last_login)) != sizeof(last_login)) {
        memset(&last_login, 0, sizeof(last_login));
    }

    last_lock.l_type = F_UNLCK;
    (void) fcntl(last_fd, F_SETLK, &last_lock);        /* unlock */

    if (!last_login.ll_time) {
        if (announce & LASTLOG_DEBUG) {
	    pam_syslog(pamh, LOG_DEBUG, "first login for user with uid %d", uid);
	}
    }

    if (!(announce & LASTLOG_QUIET)) {

	if (last_login.ll_time) {

	    /* we want the date? */
	    if (announce & LASTLOG_DATE) {
	        struct tm *tm, tm_buf;
		time_t ll_time;

		ll_time = last_login.ll_time;
		tm = localtime_r (&ll_time, &tm_buf);
		strftime (the_time, sizeof (the_time),
			  " %a %b %e %H:%M:%S %Z %Y", tm);

		date = the_time;
	    }

	    /* we want & have the host? */
	    if ((announce & LASTLOG_HOST)
		&& (last_login.ll_host[0] != '\0')) {
		/* TRANSLATORS: " from <host>" */
		if (asprintf(&host, _(" from %.*s"), UT_HOSTSIZE,
			     last_login.ll_host) < 0) {
		    pam_syslog(pamh, LOG_ERR, "out of memory");
		    retval = PAM_BUF_ERR;
		    goto cleanup;
		}
	    }

	    /* we want and have the terminal? */
	    if ((announce & LASTLOG_LINE)
		&& (last_login.ll_line[0] != '\0')) {
		/* TRANSLATORS: " on <terminal>" */
		if (asprintf(&line, _(" on %.*s"), UT_LINESIZE,
			     last_login.ll_line) < 0) {
		    pam_syslog(pamh, LOG_ERR, "out of memory");
		    retval = PAM_BUF_ERR;
		    goto cleanup;
		}
	    }

		/* TRANSLATORS: "Last login: <date> from <host> on <terminal>" */
	    retval = pam_info(pamh, _("Last login:%s%s%s"),
			      date ? date : "",
			      host ? host : "",
			      line ? line : "");
	} else if (announce & LASTLOG_NEVER) {
		D(("this is the first time this user has logged in"));
		retval = pam_info(pamh, "%s", _("Welcome to your new account!"));
	}
    }

    /* cleanup */
 cleanup:
    memset(&last_login, 0, sizeof(last_login));
    _pam_overwrite(date);
    _pam_overwrite(host);
    _pam_drop(host);
    _pam_overwrite(line);
    _pam_drop(line);

    return retval;
}

static int
last_login_write(pam_handle_t *pamh, int announce, int last_fd,
		 uid_t uid, const char *user)
{
    struct flock last_lock;
    struct lastlog last_login;
    time_t ll_time;
    const void *void_remote_host = NULL;
    const char *remote_host;
    const char *terminal_line;
    int retval = PAM_SUCCESS;

    /* rewind */
    if (lseek(last_fd, sizeof(last_login) * (off_t) uid, SEEK_SET) < 0) {
	pam_syslog(pamh, LOG_ERR, "failed to lseek %s: %m", _PATH_LASTLOG);
	return PAM_SERVICE_ERR;
    }

    /* set this login date */
    D(("set the most recent login time"));
    (void) time(&ll_time);    /* set the time */
    last_login.ll_time = ll_time;

    /* set the remote host */
    if (pam_get_item(pamh, PAM_RHOST, &void_remote_host) != PAM_SUCCESS
	|| void_remote_host == NULL) {
	remote_host = DEFAULT_HOST;
    } else {
	remote_host = void_remote_host;
    }

    /* copy to last_login */
    last_login.ll_host[0] = '\0';
    strncat(last_login.ll_host, remote_host, sizeof(last_login.ll_host)-1);

    /* set the terminal line */
    terminal_line = get_tty(pamh);

    /* copy to last_login */
    last_login.ll_host[0] = '\0';
    strncat(last_login.ll_line, terminal_line, sizeof(last_login.ll_line)-1);
    terminal_line = NULL;

    D(("locking lastlog file"));

    /* now we try to lock this file-record exclusively; non-blocking */
    memset(&last_lock, 0, sizeof(last_lock));
    last_lock.l_type = F_WRLCK;
    last_lock.l_whence = SEEK_SET;
    last_lock.l_start = sizeof(last_login) * (off_t) uid;
    last_lock.l_len = sizeof(last_login);

    if (fcntl(last_fd, F_SETLK, &last_lock) < 0) {
	D(("locking %s failed..(waiting a little)", _PATH_LASTLOG));
	pam_syslog(pamh, LOG_WARNING, "file %s is locked/write", _PATH_LASTLOG);
        sleep(LASTLOG_IGNORE_LOCK_TIME);
    }

    D(("writing to the lastlog file"));
    if (pam_modutil_write (last_fd, (char *) &last_login,
			   sizeof (last_login)) != sizeof(last_login)) {
	pam_syslog(pamh, LOG_ERR, "failed to write %s: %m", _PATH_LASTLOG);
	retval = PAM_SERVICE_ERR;
    }

    last_lock.l_type = F_UNLCK;
    (void) fcntl(last_fd, F_SETLK, &last_lock);        /* unlock */
    D(("unlocked"));

    if (announce & LASTLOG_WTMP) {
	/* write wtmp entry for user */
	logwtmp(last_login.ll_line, user, remote_host);
    }

    /* cleanup */
    memset(&last_login, 0, sizeof(last_login));

    return retval;
}

static int
last_login_date(pam_handle_t *pamh, int announce, uid_t uid, const char *user)
{
    int retval;
    int last_fd;

    /* obtain the last login date and all the relevant info */
    last_fd = open(_PATH_LASTLOG, O_RDWR);
    if (last_fd < 0) {
	pam_syslog(pamh, LOG_ERR, "unable to open %s: %m", _PATH_LASTLOG);
	D(("unable to open %s file", _PATH_LASTLOG));
	return PAM_SERVICE_ERR;
    }

    if (lseek(last_fd, sizeof(struct lastlog) * (off_t) uid, SEEK_SET) < 0) {
	pam_syslog(pamh, LOG_ERR, "failed to lseek %s: %m", _PATH_LASTLOG);
	D(("unable to lseek %s file", _PATH_LASTLOG));
	return PAM_SERVICE_ERR;
    }

    retval = last_login_read(pamh, announce, last_fd, uid);
    if (retval != PAM_SUCCESS)
      {
	close(last_fd);
	D(("error while reading lastlog file"));
	return retval;
      }

    retval = last_login_write(pamh, announce, last_fd, uid, user);

    close(last_fd);
    D(("all done with last login"));

    return retval;
}

/* --- authentication management functions (only) --- */

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
		    int argc, const char **argv)
{
    int retval, ctrl;
    const void *user;
    const struct passwd *pwd;
    uid_t uid;

    /*
     * this module gets the uid of the PAM_USER. Uses it to display
     * last login info and then updates the lastlog for that user.
     */

    ctrl = _pam_parse(pamh, flags, argc, argv);

    /* which user? */

    retval = pam_get_item(pamh, PAM_USER, &user);
    if (retval != PAM_SUCCESS || user == NULL || *(const char *)user == '\0') {
	pam_syslog(pamh, LOG_NOTICE, "user unknown");
	return PAM_USER_UNKNOWN;
    }

    /* what uid? */

    pwd = pam_modutil_getpwnam (pamh, user);
    if (pwd == NULL) {
	D(("couldn't identify user %s", user));
	return PAM_USER_UNKNOWN;
    }
    uid = pwd->pw_uid;
    pwd = NULL;                                         /* tidy up */

    /* process the current login attempt (indicate last) */

    retval = last_login_date(pamh, ctrl, uid, user);

    /* indicate success or failure */

    uid = -1;                                           /* forget this */

    return retval;
}

PAM_EXTERN int
pam_sm_close_session (pam_handle_t *pamh, int flags,
		      int argc, const char **argv)
{
    const char *terminal_line;

    if (!(_pam_parse(pamh, flags, argc, argv) & LASTLOG_WTMP))
	return PAM_SUCCESS;

    terminal_line = get_tty(pamh);

    /* Wipe out utmp logout entry */
    logwtmp(terminal_line, "", "");

    return PAM_SUCCESS;
}

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_lastlog_modstruct = {
     "pam_lastlog",
     NULL,
     NULL,
     NULL,
     pam_sm_open_session,
     pam_sm_close_session,
     NULL,
};

#endif

/* end of module definition */
