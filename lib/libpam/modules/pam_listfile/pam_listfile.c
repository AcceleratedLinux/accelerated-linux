/*
 * $Id: pam_listfile.c,v 1.12 2005/09/21 10:01:01 t8m Exp $
 *
 */

/*
 * by Elliot Lee <sopwith@redhat.com>, Red Hat Software. July 25, 1996.
 * log refused access error christopher mccrory <chrismcc@netus.com> 1998/7/11
 *
 * This code began life as the pam_rootok module.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#ifdef DEBUG
#include <assert.h>
#endif

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT

#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_modutil.h>
#include <security/pam_ext.h>

/* some syslogging */

/* checks if a user is on a list of members */
static int is_on_list(char * const *list, const char *member)
{
    while (*list) {
        if (strcmp(*list, member) == 0)
            return 1;
        list++;
    }
    return 0;
}

/* --- authentication management functions (only) --- */

/* Extended Items that are not directly available via pam_get_item() */
#define EI_GROUP (1 << 0)
#define EI_SHELL (1 << 1)

/* Constants for apply= parameter */
#define APPLY_TYPE_NULL		0
#define APPLY_TYPE_NONE		1
#define APPLY_TYPE_USER		2
#define APPLY_TYPE_GROUP	3

#define LESSER(a, b) ((a) < (b) ? (a) : (b))

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t *pamh, int flags UNUSED,
		     int argc, const char **argv)
{
    int retval, i, citem=0, extitem=0, onerr=PAM_SERVICE_ERR, sense=2;
    const void *void_citemp;
    const char *citemp;
    char *ifname=NULL;
    char aline[256];
    char mybuf[256],myval[256];
    struct stat fileinfo;
    FILE *inf;
    char apply_val[256];
    int apply_type;

    /* Stuff for "extended" items */
    struct passwd *userinfo;
    struct group *grpinfo;
    char *itemlist[256]; /* Maximum of 256 items */

    apply_type=APPLY_TYPE_NULL;
    memset(apply_val,0,sizeof(apply_val));

    for(i=0; i < argc; i++) {
	{
	    const char *junk;

	    memset(mybuf,'\0',sizeof(mybuf));
	    memset(myval,'\0',sizeof(mybuf));
	    junk = strchr(argv[i], '=');
	    if((junk == NULL) || (junk - argv[i]) >= (int) sizeof(mybuf)) {
		pam_syslog(pamh,LOG_ERR, "Bad option: \"%s\"",
			 argv[i]);
		continue;
	    }
	    strncpy(mybuf, argv[i],
		    LESSER(junk - argv[i], (int)sizeof(mybuf) - 1));
	    strncpy(myval, junk + 1, sizeof(myval) - 1);
	}
	if(!strcmp(mybuf,"onerr"))
	    if(!strcmp(myval,"succeed"))
		onerr = PAM_SUCCESS;
	    else if(!strcmp(myval,"fail"))
		onerr = PAM_SERVICE_ERR;
	    else
		return PAM_SERVICE_ERR;
	else if(!strcmp(mybuf,"sense"))
	    if(!strcmp(myval,"allow"))
		sense=0;
	    else if(!strcmp(myval,"deny"))
		sense=1;
	    else
		return onerr;
	else if(!strcmp(mybuf,"file")) {
	    ifname = (char *)malloc(strlen(myval)+1);
	    if (!ifname)
		return PAM_BUF_ERR;
	    strcpy(ifname,myval);
	} else if(!strcmp(mybuf,"item"))
	    if(!strcmp(myval,"user"))
		citem = PAM_USER;
	    else if(!strcmp(myval,"tty"))
		citem = PAM_TTY;
	    else if(!strcmp(myval,"rhost"))
		citem = PAM_RHOST;
	    else if(!strcmp(myval,"ruser"))
		citem = PAM_RUSER;
	    else { /* These items are related to the user, but are not
		      directly gettable with pam_get_item */
		citem = PAM_USER;
		if(!strcmp(myval,"group"))
		    extitem = EI_GROUP;
		else if(!strcmp(myval,"shell"))
		    extitem = EI_SHELL;
		else
		    citem = 0;
	    } else if(!strcmp(mybuf,"apply")) {
		apply_type=APPLY_TYPE_NONE;
		memset(apply_val,'\0',sizeof(apply_val));
		if (myval[0]=='@') {
		    apply_type=APPLY_TYPE_GROUP;
		    strncpy(apply_val,myval+1,sizeof(apply_val)-1);
		} else {
		    apply_type=APPLY_TYPE_USER;
		    strncpy(apply_val,myval,sizeof(apply_val)-1);
		}
	    } else {
		free(ifname);
		pam_syslog(pamh,LOG_ERR, "Unknown option: %s",mybuf);
		return onerr;
	    }
    }

    if(!citem) {
	pam_syslog(pamh,LOG_ERR,
		  "Unknown item or item not specified");
	free(ifname);
	return onerr;
    } else if(!ifname) {
	pam_syslog(pamh,LOG_ERR, "List filename not specified");
	return onerr;
    } else if(sense == 2) {
	pam_syslog(pamh,LOG_ERR,
		  "Unknown sense or sense not specified");
	free(ifname);
	return onerr;
    } else if(
	      (apply_type==APPLY_TYPE_NONE) ||
	      ((apply_type!=APPLY_TYPE_NULL) && (*apply_val=='\0'))
              ) {
	pam_syslog(pamh,LOG_ERR,
		  "Invalid usage for apply= parameter");
	return onerr;
    }

    /* Check if it makes sense to use the apply= parameter */
    if (apply_type != APPLY_TYPE_NULL) {
	if((citem==PAM_USER) || (citem==PAM_RUSER)) {
	    pam_syslog(pamh,LOG_WARNING,
		      "Non-sense use for apply= parameter");
	    apply_type=APPLY_TYPE_NULL;
	}
	if(extitem && (extitem==EI_GROUP)) {
	    pam_syslog(pamh,LOG_WARNING,
		      "Non-sense use for apply= parameter");
	    apply_type=APPLY_TYPE_NULL;
	}
    }

    /* Short-circuit - test if this session apply for this user */
    {
	const char *user_name;
	int rval;

	rval=pam_get_user(pamh,&user_name,NULL);
	if((rval==PAM_SUCCESS) && user_name && user_name[0]) {
	    /* Got it ? Valid ? */
	    if(apply_type==APPLY_TYPE_USER) {
		if(strcmp(user_name, apply_val)) {
		    /* Does not apply to this user */
#ifdef DEBUG
		    pam_syslog(pamh,LOG_DEBUG,
			      "don't apply: apply=%s, user=%s",
			     apply_val,user_name);
#endif /* DEBUG */
		    free(ifname);
		    return PAM_IGNORE;
		}
	    } else if(apply_type==APPLY_TYPE_GROUP) {
		if(!pam_modutil_user_in_group_nam_nam(pamh,user_name,apply_val)) {
		    /* Not a member of apply= group */
#ifdef DEBUG
		    pam_syslog(pamh,LOG_DEBUG,
			     
			     "don't apply: %s not a member of group %s",
			     user_name,apply_val);
#endif /* DEBUG */
		    free(ifname);
		    return PAM_IGNORE;
		}
	    }
	}
    }

    retval = pam_get_item(pamh,citem,&void_citemp);
    citemp = void_citemp;
    if(retval != PAM_SUCCESS) {
	return onerr;
    }
    if((citem == PAM_USER) && !citemp) {
	retval = pam_get_user(pamh,&citemp,NULL);
	if (retval != PAM_SUCCESS || !citemp) {
	    free(ifname);
	    return PAM_SERVICE_ERR;
	}
    }
    if((citem == PAM_TTY) && citemp) {
        /* Normalize the TTY name. */
        if(strncmp(citemp, "/dev/", 5) == 0) {
            citemp += 5;
        }
    }

    if(!citemp || (strlen(citemp) == 0)) {
	free(ifname);
	/* The item was NULL - we are sure not to match */
	return sense?PAM_SUCCESS:PAM_AUTH_ERR;
    }

    if(extitem) {
	switch(extitem) {
	    case EI_GROUP:
		userinfo = pam_modutil_getpwnam(pamh, citemp);
		if (userinfo == NULL) {
		    pam_syslog(pamh,LOG_ERR, "getpwnam(%s) failed",
			     citemp);
		    free(ifname);
		    return onerr;
		}
		grpinfo = pam_modutil_getgrgid(pamh, userinfo->pw_gid);
		if (grpinfo == NULL) {
		    pam_syslog(pamh,LOG_ERR, "getgrgid(%d) failed",
			     (int)userinfo->pw_gid);
		    free(ifname);
		    return onerr;
		}
		itemlist[0] = x_strdup(grpinfo->gr_name);
		setgrent();
		for (i=1; (i < (int)(sizeof(itemlist)/sizeof(itemlist[0])-1)) &&
			 (grpinfo = getgrent()); ) {
		    if (is_on_list(grpinfo->gr_mem,citemp)) {
			itemlist[i++] = x_strdup(grpinfo->gr_name);
		    }
                }
		endgrent();
		itemlist[i] = NULL;
		break;
	    case EI_SHELL:
		/* Assume that we have already gotten PAM_USER in
		   pam_get_item() - a valid assumption since citem
		   gets set to PAM_USER in the extitem switch */
		userinfo = pam_modutil_getpwnam(pamh, citemp);
		if (userinfo == NULL) {
		    pam_syslog(pamh,LOG_ERR, "getpwnam(%s) failed",
			     citemp);
		    free(ifname);
		    return onerr;
		}
		citemp = userinfo->pw_shell;
		break;
	    default:
		pam_syslog(pamh,LOG_ERR,
			 
			 "Internal weirdness, unknown extended item %d",
			 extitem);
		free(ifname);
		return onerr;
	}
    }
#ifdef DEBUG
    pam_syslog(pamh,LOG_INFO,
	     
	     "Got file = %s, item = %d, value = %s, sense = %d",
	     ifname, citem, citemp, sense);
#endif
    if(lstat(ifname,&fileinfo)) {
	pam_syslog(pamh,LOG_ERR, "Couldn't open %s",ifname);
	free(ifname);
	return onerr;
    }

    if((fileinfo.st_mode & S_IWOTH)
       || !S_ISREG(fileinfo.st_mode)) {
	/* If the file is world writable or is not a
	   normal file, return error */
	pam_syslog(pamh,LOG_ERR,
		 "%s is either world writable or not a normal file",
		 ifname);
	free(ifname);
	return PAM_AUTH_ERR;
    }

    inf = fopen(ifname,"r");
    if(inf == NULL) { /* Check that we opened it successfully */
	if (onerr == PAM_SERVICE_ERR) {
	    /* Only report if it's an error... */
	    pam_syslog(pamh,LOG_ERR,  "Error opening %s", ifname);
	}
	free(ifname);
	return onerr;
    }
    /* There should be no more errors from here on */
    retval=PAM_AUTH_ERR;
    /* This loop assumes that PAM_SUCCESS == 0
       and PAM_AUTH_ERR != 0 */
#ifdef DEBUG
    assert(PAM_SUCCESS == 0);
    assert(PAM_AUTH_ERR != 0);
#endif
    if(extitem == EI_GROUP) {
	while((fgets(aline,sizeof(aline),inf) != NULL)
	      && retval) {
            if(strlen(aline) == 0)
		continue;
	    if(aline[strlen(aline) - 1] == '\n')
		aline[strlen(aline) - 1] = '\0';
	    for(i=0;itemlist[i];)
		/* If any of the items match, strcmp() == 0, and we get out
		   of this loop */
		retval = (strcmp(aline,itemlist[i++]) && retval);
	}
	for(i=0;itemlist[i];)
	    free(itemlist[i++]);
    } else {
	while((fgets(aline,sizeof(aline),inf) != NULL)
	      && retval) {
            char *a = aline;
            if(strlen(aline) == 0)
		continue;
	    if(aline[strlen(aline) - 1] == '\n')
		aline[strlen(aline) - 1] = '\0';
            if(strlen(aline) == 0)
		continue;
	    if(aline[strlen(aline) - 1] == '\r')
		aline[strlen(aline) - 1] = '\0';
	    if(citem == PAM_TTY)
	        if(strncmp(a, "/dev/", 5) == 0)
	            a += 5;
	    retval = strcmp(a,citemp);
	}
    }
    fclose(inf);
    free(ifname);
    if ((sense && retval) || (!sense && !retval)) {
#ifdef DEBUG
	pam_syslog(pamh,LOG_INFO, 
		 "Returning PAM_SUCCESS, retval = %d", retval);
#endif
	return PAM_SUCCESS;
    }
    else {
	const void *service;
	const char *user_name;
#ifdef DEBUG
	pam_syslog(pamh,LOG_INFO,
		 "Returning PAM_AUTH_ERR, retval = %d", retval);
#endif
	(void) pam_get_item(pamh, PAM_SERVICE, &service);
	(void) pam_get_user(pamh, &user_name, NULL);
	pam_syslog (pamh, LOG_ALERT, "Refused user %s for service %s",
		    user_name, (const char *)service);
	return PAM_AUTH_ERR;
    }
}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t *pamh UNUSED, int flags UNUSED,
		int argc UNUSED, const char **argv UNUSED)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_acct_mgmt (pam_handle_t *pamh, int flags UNUSED,
		  int argc, const char **argv)
{
    return pam_sm_authenticate(pamh, 0, argc, argv);
}

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_listfile_modstruct = {
    "pam_listfile",
    pam_sm_authenticate,
    pam_sm_setcred,
    pam_sm_acct_mgmt,
    NULL,
    NULL,
    NULL,
};

#endif /* PAM_STATIC */

/* end of module definition */
