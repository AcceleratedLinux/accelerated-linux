/*
 * pam_limits - impose resource limits when opening a user session
 *
 * 1.6 - modified for PLD (added process priority settings)
 *       by Marcin Korzonek <mkorz@shadow.eu.org>
 * 1.5 - Elliot Lee's "max system logins patch"
 * 1.4 - addressed bug in configuration file parser
 * 1.3 - modified the configuration file format
 * 1.2 - added 'debug' and 'conf=' arguments
 * 1.1 - added @group support
 * 1.0 - initial release - Linux ONLY
 *
 * See end for Copyright information
 */

#if !(defined(linux))
#error THIS CODE IS KNOWN TO WORK ONLY ON LINUX !!!
#endif

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <limits.h>

#include <utmp.h>
#ifndef UT_USER  /* some systems have ut_name instead of ut_user */
#define UT_USER ut_user
#endif

#include <grp.h>
#include <pwd.h>

/* Module defines */
#define LINE_LENGTH 1024

#define LIMITS_DEF_USER     0 /* limit was set by an user entry */
#define LIMITS_DEF_GROUP    1 /* limit was set by a group entry */
#define LIMITS_DEF_ALLGROUP 2 /* limit was set by a group entry */
#define LIMITS_DEF_ALL      3 /* limit was set by an default entry */
#define LIMITS_DEF_DEFAULT  4 /* limit was set by an default entry */
#define LIMITS_DEF_NONE     5 /* this limit was not set yet */

static const char *limits_def_names[] = {
       "USER",
       "GROUP",
       "ALLGROUP",
       "ALL",
       "DEFAULT",
       "NONE",
       NULL
};

struct user_limits_struct {
    int supported;
    int src_soft;
    int src_hard;
    struct rlimit limit;
};

/* internal data */
struct pam_limit_s {
    int login_limit;     /* the max logins limit */
    int login_limit_def; /* which entry set the login limit */
    int flag_numsyslogins; /* whether to limit logins only for a
			      specific user or to count all logins */
    int priority;	 /* the priority to run user process with */
    struct user_limits_struct limits[RLIM_NLIMITS];
    char conf_file[BUFSIZ];
    int utmp_after_pam_call;
    char login_group[LINE_LENGTH];
};

#define LIMIT_LOGIN RLIM_NLIMITS+1
#define LIMIT_NUMSYSLOGINS RLIM_NLIMITS+2

#define LIMIT_PRI RLIM_NLIMITS+3

#define LIMIT_SOFT  1
#define LIMIT_HARD  2

#define PAM_SM_SESSION

#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_modutil.h>
#include <security/pam_ext.h>

/* argument parsing */

#define PAM_DEBUG_ARG       0x0001
#define PAM_DO_SETREUID     0x0002
#define PAM_UTMP_EARLY      0x0004

static int
_pam_parse (const pam_handle_t *pamh, int argc, const char **argv,
	    struct pam_limit_s *pl)
{
    int ctrl=0;

    /* step through arguments */
    for (ctrl=0; argc-- > 0; ++argv) {

	/* generic options */

	if (!strcmp(*argv,"debug")) {
	    ctrl |= PAM_DEBUG_ARG;
	} else if (!strncmp(*argv,"conf=",5)) {
	    strncpy(pl->conf_file,*argv+5,sizeof(pl->conf_file)-1);
	} else if (!strncmp(*argv,"change_uid",10)) {
	    ctrl |= PAM_DO_SETREUID;
	} else if (!strcmp(*argv,"utmp_early")) {
	    ctrl |= PAM_UTMP_EARLY;
	} else {
	    pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
	}
    }
    pl->conf_file[sizeof(pl->conf_file) - 1] = '\0';

    return ctrl;
}


#define LIMITED_OK 0 /* limit setting appeared to work */
#define LIMIT_ERR  1 /* error setting a limit */
#define LOGIN_ERR  2 /* too many logins err */

/* Counts the number of user logins and check against the limit*/
static int
check_logins (pam_handle_t *pamh, const char *name, int limit, int ctrl,
              struct pam_limit_s *pl)
{
    struct utmp *ut;
    int count;

    if (ctrl & PAM_DEBUG_ARG) {
        pam_syslog(pamh, LOG_DEBUG,
		   "checking logins for '%s' (maximum of %d)", name, limit);
    }

    if (limit < 0)
        return 0; /* no limits imposed */
    if (limit == 0) /* maximum 0 logins ? */ {
        pam_syslog(pamh, LOG_WARNING, "No logins allowed for '%s'", name);
        return LOGIN_ERR;
    }

    setutent();

    /* Because there is no definition about when an application
       actually adds a utmp entry, some applications bizarrely do the
       utmp call before the have PAM authenticate them to the system:
       you're logged it, sort of...? Anyway, you can use the
       "utmp_early" module argument in your PAM config file to make
       allowances for this sort of problem. (There should be a PAM
       standard for this, since if a module wants to actually map a
       username then any early utmp entry will be for the unmapped
       name = broken.) */

    if (ctrl & PAM_UTMP_EARLY) {
	count = 0;
    } else {
	count = 1;
    }

    while((ut = getutent())) {
#ifdef USER_PROCESS
        if (ut->ut_type != USER_PROCESS) {
            continue;
	}
#endif
        if (ut->UT_USER[0] == '\0') {
            continue;
	}
        if (!pl->flag_numsyslogins) {
	    if (((pl->login_limit_def == LIMITS_DEF_USER)
	         || (pl->login_limit_def == LIMITS_DEF_GROUP)
		 || (pl->login_limit_def == LIMITS_DEF_DEFAULT))
		&& strncmp(name, ut->UT_USER, sizeof(ut->UT_USER)) != 0) {
                continue;
	    }
	    if ((pl->login_limit_def == LIMITS_DEF_ALLGROUP)
		&& !pam_modutil_user_in_group_nam_nam(pamh, ut->UT_USER, pl->login_group)) {
                continue;
	    }
	}
	if (++count > limit) {
	    break;
	}
    }
    endutent();
    if (count > limit) {
	if (name) {
	    pam_syslog(pamh, LOG_WARNING,
		       "Too many logins (max %d) for %s", limit, name);
	} else {
	    pam_syslog(pamh, LOG_WARNING, "Too many system logins (max %d)", limit);
	}
        return LOGIN_ERR;
    }
    return 0;
}

static int init_limits(struct pam_limit_s *pl)
{
    int i;
    int retval = PAM_SUCCESS;

    D(("called."));

    for(i = 0; i < RLIM_NLIMITS; i++) {
	int r = getrlimit(i, &pl->limits[i].limit);
	if (r == -1) {
	    pl->limits[i].supported = 0;
	    if (errno != EINVAL) {
		retval = !PAM_SUCCESS;
	    }
	} else {
	    pl->limits[i].supported = 1;
	    pl->limits[i].src_soft = LIMITS_DEF_NONE;
	    pl->limits[i].src_hard = LIMITS_DEF_NONE;
	}
    }

    errno = 0;
    pl->priority = getpriority (PRIO_PROCESS, 0);
    if (pl->priority == -1 && errno != 0)
      retval = !PAM_SUCCESS;
    pl->login_limit = -2;
    pl->login_limit_def = LIMITS_DEF_NONE;

    return retval;
}

static void
process_limit (const pam_handle_t *pamh, int source, const char *lim_type,
	       const char *lim_item, const char *lim_value,
	       int ctrl, struct pam_limit_s *pl)
{
    int limit_item;
    int limit_type = 0;
    int int_value = 0;
    rlim_t rlimit_value = 0;
    char *endptr;
    const char *value_orig = lim_value;

    if (ctrl & PAM_DEBUG_ARG)
	 pam_syslog(pamh, LOG_DEBUG, "%s: processing %s %s %s for %s",
		    __FUNCTION__, lim_type, lim_item, lim_value,
		    limits_def_names[source]);

    if (strcmp(lim_item, "cpu") == 0)
        limit_item = RLIMIT_CPU;
    else if (strcmp(lim_item, "fsize") == 0)
        limit_item = RLIMIT_FSIZE;
    else if (strcmp(lim_item, "data") == 0)
	limit_item = RLIMIT_DATA;
    else if (strcmp(lim_item, "stack") == 0)
	limit_item = RLIMIT_STACK;
    else if (strcmp(lim_item, "core") == 0)
	limit_item = RLIMIT_CORE;
    else if (strcmp(lim_item, "rss") == 0)
	limit_item = RLIMIT_RSS;
    else if (strcmp(lim_item, "nproc") == 0)
	limit_item = RLIMIT_NPROC;
    else if (strcmp(lim_item, "nofile") == 0)
	limit_item = RLIMIT_NOFILE;
    else if (strcmp(lim_item, "memlock") == 0)
	limit_item = RLIMIT_MEMLOCK;
    else if (strcmp(lim_item, "as") == 0)
	limit_item = RLIMIT_AS;
#ifdef RLIMIT_LOCKS
    else if (strcmp(lim_item, "locks") == 0)
	limit_item = RLIMIT_LOCKS;
#endif
#ifdef RLIMIT_SIGPENDING
    else if (strcmp(lim_item, "sigpending") == 0)
	limit_item = RLIMIT_SIGPENDING;
#endif
#ifdef RLIMIT_MSGQUEUE
    else if (strcmp(lim_item, "msgqueue") == 0)
	limit_item = RLIMIT_MSGQUEUE;
#endif
#ifdef RLIMIT_NICE
    else if (strcmp(lim_item, "nice") == 0)
	limit_item = RLIMIT_NICE;
#endif
#ifdef RLIMIT_RTPRIO
    else if (strcmp(lim_item, "rtprio") == 0)
	limit_item = RLIMIT_RTPRIO;
#endif
    else if (strcmp(lim_item, "maxlogins") == 0) {
	limit_item = LIMIT_LOGIN;
	pl->flag_numsyslogins = 0;
    } else if (strcmp(lim_item, "maxsyslogins") == 0) {
	limit_item = LIMIT_NUMSYSLOGINS;
	pl->flag_numsyslogins = 1;
    } else if (strcmp(lim_item, "priority") == 0) {
	limit_item = LIMIT_PRI;
    } else {
        pam_syslog(pamh, LOG_DEBUG, "unknown limit item '%s'", lim_item);
        return;
    }

    if (strcmp(lim_type,"soft")==0)
	limit_type=LIMIT_SOFT;
    else if (strcmp(lim_type, "hard")==0)
	limit_type=LIMIT_HARD;
    else if (strcmp(lim_type,"-")==0)
	limit_type=LIMIT_SOFT | LIMIT_HARD;
    else if (limit_item != LIMIT_LOGIN && limit_item != LIMIT_NUMSYSLOGINS) {
        pam_syslog(pamh, LOG_DEBUG, "unknown limit type '%s'", lim_type);
        return;
    }
	if (limit_item != LIMIT_PRI
#ifdef RLIMIT_NICE
	    && limit_item != RLIMIT_NICE
#endif
	    && (strcmp(lim_value, "-1") == 0
		|| strcmp(lim_value, "-") == 0 || strcmp(lim_value, "unlimited") == 0
		|| strcmp(lim_value, "infinity") == 0)) {
		int_value = -1;
		rlimit_value = RLIM_INFINITY;
	} else if (limit_item == LIMIT_PRI || limit_item == LIMIT_LOGIN ||
#ifdef RLIMIT_NICE
		limit_item == RLIMIT_NICE ||
#endif
		limit_item == LIMIT_NUMSYSLOGINS) {
		long temp;
		temp = strtol (lim_value, &endptr, 10);
		temp = temp < INT_MAX ? temp : INT_MAX;
		int_value = temp > INT_MIN ? temp : INT_MIN;
		if (int_value == 0 && value_orig == endptr) {
			pam_syslog(pamh, LOG_DEBUG,
				   "wrong limit value '%s' for limit type '%s'",
				   lim_value, lim_type);
            return;
		}
	} else {
#ifdef __USE_FILE_OFFSET64
		rlimit_value = strtoull (lim_value, &endptr, 10);
#else
		rlimit_value = strtoul (lim_value, &endptr, 10);
#endif
		if (rlimit_value == 0 && value_orig == endptr) {
			pam_syslog(pamh, LOG_DEBUG,
				   "wrong limit value '%s' for limit type '%s'",
				   lim_value, lim_type);
			return;
		}
	}

    /* one more special case when limiting logins */
    if ((source == LIMITS_DEF_ALL || source == LIMITS_DEF_ALLGROUP)
		&& (limit_item != LIMIT_LOGIN)) {
	if (ctrl & PAM_DEBUG_ARG)
	    pam_syslog(pamh, LOG_DEBUG,
		       "'%%' domain valid for maxlogins type only");
	return;
    }

    switch(limit_item) {
        case RLIMIT_CPU:
         if (rlimit_value != RLIM_INFINITY)
            rlimit_value *= 60;
         break;
        case RLIMIT_FSIZE:
        case RLIMIT_DATA:
        case RLIMIT_STACK:
        case RLIMIT_CORE:
        case RLIMIT_RSS:
        case RLIMIT_MEMLOCK:
        case RLIMIT_AS:
         if (rlimit_value != RLIM_INFINITY)
            rlimit_value *= 1024;
    	 break;
#ifdef RLIMIT_NICE
	case RLIMIT_NICE:
	 if (int_value > 19)
	    int_value = 19;
	 rlimit_value = 19 - int_value;
#endif
         break;
    }

    if ( (limit_item != LIMIT_LOGIN)
	 && (limit_item != LIMIT_NUMSYSLOGINS)
	 && (limit_item != LIMIT_PRI) ) {
        if (limit_type & LIMIT_SOFT) {
	    if (pl->limits[limit_item].src_soft < source) {
                return;
	    } else {
                pl->limits[limit_item].limit.rlim_cur = rlimit_value;
                pl->limits[limit_item].src_soft = source;
            }
	}
        if (limit_type & LIMIT_HARD) {
	    if (pl->limits[limit_item].src_hard < source) {
                return;
            } else {
                pl->limits[limit_item].limit.rlim_max = rlimit_value;
                pl->limits[limit_item].src_hard = source;
            }
	}
    } else {
	/* recent kernels support negative priority limits (=raise priority) */

	if (limit_item == LIMIT_PRI) {
		pl->priority = int_value;
	} else {
	        if (pl->login_limit_def < source) {
	            return;
	        } else {
	            pl->login_limit = int_value;
	            pl->login_limit_def = source;
        	}
	}
    }
    return;
}

static int parse_config_file(pam_handle_t *pamh, const char *uname, int ctrl,
			     struct pam_limit_s *pl)
{
    FILE *fil;
    char buf[LINE_LENGTH];

#define CONF_FILE (pl->conf_file[0])?pl->conf_file:LIMITS_FILE
    /* check for the LIMITS_FILE */
    if (ctrl & PAM_DEBUG_ARG)
        pam_syslog(pamh, LOG_DEBUG, "reading settings from '%s'", CONF_FILE);
    fil = fopen(CONF_FILE, "r");
    if (fil == NULL) {
        pam_syslog (pamh, LOG_WARNING,
		    "cannot read settings from %s: %m", CONF_FILE);
        return PAM_SERVICE_ERR;
    }
#undef CONF_FILE

    /* init things */
    memset(buf, 0, sizeof(buf));
    /* start the show */
    while (fgets(buf, LINE_LENGTH, fil) != NULL) {
        char domain[LINE_LENGTH];
        char ltype[LINE_LENGTH];
        char item[LINE_LENGTH];
        char value[LINE_LENGTH];
        int i;
        size_t j;
        char *tptr;

        tptr = buf;
        /* skip the leading white space */
        while (*tptr && isspace(*tptr))
            tptr++;
        strncpy(buf, tptr, sizeof(buf)-1);
	buf[sizeof(buf)-1] = '\0';

        /* Rip off the comments */
        tptr = strchr(buf,'#');
        if (tptr)
            *tptr = '\0';
        /* Rip off the newline char */
        tptr = strchr(buf,'\n');
        if (tptr)
            *tptr = '\0';
        /* Anything left ? */
        if (!strlen(buf)) {
            memset(buf, 0, sizeof(buf));
            continue;
        }

        memset(domain, 0, sizeof(domain));
        memset(ltype, 0, sizeof(ltype));
        memset(item, 0, sizeof(item));
        memset(value, 0, sizeof(value));

        i = sscanf(buf,"%s%s%s%s", domain, ltype, item, value);
	D(("scanned line[%d]: domain[%s], ltype[%s], item[%s], value[%s]",
	   i, domain, ltype, item, value));

        for(j=0; j < strlen(ltype); j++)
            ltype[j]=tolower(ltype[j]);
        for(j=0; j < strlen(item); j++)
            item[j]=tolower(item[j]);
        for(j=0; j < strlen(value); j++)
            value[j]=tolower(value[j]);

        if (i == 4) { /* a complete line */
            if (strcmp(uname, domain) == 0) /* this user have a limit */
                process_limit(pamh, LIMITS_DEF_USER, ltype, item, value, ctrl, pl);
            else if (domain[0]=='@') {
		    if (ctrl & PAM_DEBUG_ARG) {
			pam_syslog(pamh, LOG_DEBUG,
				   "checking if %s is in group %s",
				   uname, domain + 1);
		    }
                if (pam_modutil_user_in_group_nam_nam(pamh, uname, domain+1))
                    process_limit(pamh, LIMITS_DEF_GROUP, ltype, item, value, ctrl,
				  pl);
            } else if (domain[0]=='%') {
		    if (ctrl & PAM_DEBUG_ARG) {
			pam_syslog(pamh, LOG_DEBUG,
				   "checking if %s is in group %s",
				   uname, domain + 1);
		    }
		if (strcmp(domain,"%") == 0)
		    process_limit(pamh, LIMITS_DEF_ALL, ltype, item, value, ctrl,
				  pl);
		else if (pam_modutil_user_in_group_nam_nam(pamh, uname, domain+1)) {
		    strcpy(pl->login_group, domain+1);
                    process_limit(pamh, LIMITS_DEF_ALLGROUP, ltype, item, value, ctrl,
				  pl);
		}
            } else if (strcmp(domain, "*") == 0)
                process_limit(pamh, LIMITS_DEF_DEFAULT, ltype, item, value, ctrl,
			      pl);
	} else if (i == 2 && ltype[0] == '-') { /* Probably a no-limit line */
	    if (strcmp(uname, domain) == 0) {
		if (ctrl & PAM_DEBUG_ARG) {
		    pam_syslog(pamh, LOG_DEBUG, "no limits for '%s'", uname);
		}
		fclose(fil);
		return PAM_IGNORE;
	    } else if (domain[0] == '@' && pam_modutil_user_in_group_nam_nam(pamh, uname, domain+1)) {
		if (ctrl & PAM_DEBUG_ARG) {
		    pam_syslog(pamh, LOG_DEBUG,
			       "no limits for '%s' in group '%s'",
			       uname, domain+1);
		}
		fclose(fil);
		return PAM_IGNORE;
	    }
        } else {
            pam_syslog(pamh, LOG_WARNING, "invalid line '%s' - skipped", buf);
	}
    }
    fclose(fil);
    return PAM_SUCCESS;
}

static int setup_limits(pam_handle_t *pamh,
			const char *uname, uid_t uid, int ctrl,
			struct pam_limit_s *pl)
{
    int i;
    int status;
    int retval = LIMITED_OK;

    if (uid == 0) {
	/* do not impose limits (+ve limits anyway) on the superuser */
	if (pl->priority > 0) {
	    if (ctrl & PAM_DEBUG_ARG) {
		pam_syslog(pamh, LOG_DEBUG,
			   "user '%s' has UID 0 - no limits imposed", uname);
	    }
            pl->priority = 0;
	}
    }

    for (i=0, status=LIMITED_OK; i<RLIM_NLIMITS; i++) {
	if (!pl->limits[i].supported) {
	    /* skip it if its not known to the system */
	    continue;
	}
	if (pl->limits[i].src_soft == LIMITS_DEF_NONE &&
	    pl->limits[i].src_hard == LIMITS_DEF_NONE) {
	    /* skip it if its not initialized */
	    continue;
	}
        if (pl->limits[i].limit.rlim_cur > pl->limits[i].limit.rlim_max)
            pl->limits[i].limit.rlim_cur = pl->limits[i].limit.rlim_max;
	status |= setrlimit(i, &pl->limits[i].limit);
    }

    if (status) {
        retval = LIMIT_ERR;
    }

    status = setpriority(PRIO_PROCESS, 0, pl->priority);
    if (status != 0) {
        retval = LIMIT_ERR;
    }

    if (uid == 0) {
	D(("skip login limit check for uid=0"));
    } else if (pl->login_limit > 0) {
        if (check_logins(pamh, uname, pl->login_limit, ctrl, pl) == LOGIN_ERR) {
            retval |= LOGIN_ERR;
	}
    } else if (pl->login_limit == 0) {
        retval |= LOGIN_ERR;
    }

    return retval;
}

/* now the session stuff */
PAM_EXTERN int
pam_sm_open_session (pam_handle_t *pamh, int flags UNUSED,
		     int argc, const char **argv)
{
    int retval;
    char *user_name;
    struct passwd *pwd;
    int ctrl;
    struct pam_limit_s pl;

    D(("called."));

    memset(&pl, 0, sizeof(pl));

    ctrl = _pam_parse(pamh, argc, argv, &pl);
    retval = pam_get_item( pamh, PAM_USER, (void*) &user_name );
    if ( user_name == NULL || retval != PAM_SUCCESS ) {
        pam_syslog(pamh, LOG_CRIT, "open_session - error recovering username");
        return PAM_SESSION_ERR;
     }

    pwd = getpwnam(user_name);
    if (!pwd) {
        if (ctrl & PAM_DEBUG_ARG)
            pam_syslog(pamh, LOG_WARNING,
		       "open_session username '%s' does not exist", user_name);
        return PAM_SESSION_ERR;
    }

    retval = init_limits(&pl);
    if (retval != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_WARNING, "cannot initialize");
        return PAM_ABORT;
    }

    retval = parse_config_file(pamh, pwd->pw_name, ctrl, &pl);
    if (retval == PAM_IGNORE) {
	D(("the configuration file has an applicable '<domain> -' entry"));
	return PAM_SUCCESS;
    }
    if (retval != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_WARNING, "error parsing the configuration file");
        return retval;
    }

    if (ctrl & PAM_DO_SETREUID) {
	setreuid(pwd->pw_uid, -1);
    }
    retval = setup_limits(pamh, pwd->pw_name, pwd->pw_uid, ctrl, &pl);
    if (retval & LOGIN_ERR)
	pam_error(pamh, _("Too many logins for '%s'."), pwd->pw_name);
    if (retval != LIMITED_OK) {
        return PAM_PERM_DENIED;
    }

    return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_close_session (pam_handle_t *pamh UNUSED, int flags UNUSED,
		      int argc UNUSED, const char **argv UNUSED)
{
     /* nothing to do */
     return PAM_SUCCESS;
}

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_limits_modstruct = {
     "pam_limits",
     NULL,
     NULL,
     NULL,
     pam_sm_open_session,
     pam_sm_close_session,
     NULL
};
#endif

/*
 * Copyright (c) Cristian Gafton, 1996-1997, <gafton@redhat.com>
 *                                              All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
