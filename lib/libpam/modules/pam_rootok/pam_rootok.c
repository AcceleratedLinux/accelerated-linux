/* pam_rootok module */

/*
 * $Id: pam_rootok.c,v 1.7 2005/12/12 14:45:02 ldv Exp $
 *
 * Written by Andrew Morgan <morgan@linux.kernel.org> 1996/3/11
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_AUTH

#include <security/pam_modules.h>
#include <security/pam_ext.h>

#ifdef WITH_SELINUX
#include <selinux/selinux.h>
#include <selinux/av_permissions.h>
#endif

/* argument parsing */

#define PAM_DEBUG_ARG       01

static int
_pam_parse (const pam_handle_t *pamh, int argc, const char **argv)
{
    int ctrl=0;

    /* step through arguments */
    for (ctrl=0; argc-- > 0; ++argv) {

	/* generic options */

	if (!strcmp(*argv,"debug"))
	    ctrl |= PAM_DEBUG_ARG;
	else {
	    pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
	}
    }

    return ctrl;
}

/* --- authentication management functions (only) --- */

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t *pamh, int flags UNUSED,
		     int argc, const char **argv)
{
    int ctrl;
    int retval = PAM_AUTH_ERR;

    ctrl = _pam_parse(pamh, argc, argv);
    if (getuid() == 0)
#ifdef WITH_SELINUX
      if (is_selinux_enabled()<1 || checkPasswdAccess(PASSWD__ROOTOK)==0)
#endif
	retval = PAM_SUCCESS;

    if (ctrl & PAM_DEBUG_ARG) {
	pam_syslog(pamh, LOG_DEBUG, "authentication %s",
		   (retval==PAM_SUCCESS) ? "succeeded" : "failed");
    }

    return retval;
}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t *pamh UNUSED, int flags UNUSED,
		int argc UNUSED, const char **argv UNUSED)
{
    return PAM_SUCCESS;
}


#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_rootok_modstruct = {
    "pam_rootok",
    pam_sm_authenticate,
    pam_sm_setcred,
    NULL,
    NULL,
    NULL,
    NULL,
};

#endif

/* end of module definition */
