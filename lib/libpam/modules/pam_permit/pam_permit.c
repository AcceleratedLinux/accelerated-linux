/* pam_permit module */

/*
 * $Id: pam_permit.c,v 1.4 2005/09/17 08:50:29 t8m Exp $
 *
 * Written by Andrew Morgan <morgan@parc.power.net> 1996/3/11
 *
 */

#include "config.h"

#define DEFAULT_USER "nobody"

#include <stdio.h>

/*
 * here, we make definitions for the externally accessible functions
 * in this file (these definitions are required for static modules
 * but strongly encouraged generally) they are used to instruct the
 * modules include file to define their prototypes.
 */

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <security/pam_modules.h>
#include <security/_pam_macros.h>

/* --- authentication management functions --- */

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags UNUSED,
		    int argc UNUSED, const char **argv UNUSED)
{
    int retval;
    const char *user=NULL;

    /*
     * authentication requires we know who the user wants to be
     */
    retval = pam_get_user(pamh, &user, NULL);
    if (retval != PAM_SUCCESS) {
	D(("get user returned error: %s", pam_strerror(pamh,retval)));
	return retval;
    }
    if (user == NULL || *user == '\0') {
	D(("username not known"));
	retval = pam_set_item(pamh, PAM_USER, (const void *) DEFAULT_USER);
	if (retval != PAM_SUCCESS)
	    return PAM_USER_UNKNOWN;
    }
    user = NULL;                                            /* clean up */

    return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh UNUSED, int flags UNUSED,
	       int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- account management functions --- */

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh UNUSED, int flags UNUSED,
		 int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- password management --- */

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh UNUSED, int flags UNUSED,
		 int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- session management --- */

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh UNUSED, int flags UNUSED,
		    int argc UNUSED, const char **argv UNUSED)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh UNUSED, int flags UNUSED,
		     int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* end of module definition */

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_permit_modstruct = {
    "pam_permit",
    pam_sm_authenticate,
    pam_sm_setcred,
    pam_sm_acct_mgmt,
    pam_sm_open_session,
    pam_sm_close_session,
    pam_sm_chauthtok
};

#endif
