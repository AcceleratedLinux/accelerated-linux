/*
 * lib/kdb/kdb_ldap/ldap_service_rights.c
 *
 * Copyright (c) 2004-2005, Novell, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *   * The copyright holder's name is not used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ldap_main.h"
#include "ldap_services.h"
#include "ldap_err.h"

/* NOTE: add appropriate rights for krbpasswordexpiration attribute */

#ifdef HAVE_EDIRECTORY

static char *kdcrights_subtree[][2] = {
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#ObjectClass"},
    {"2#subtree#","#krbTicketPolicyReference"},
    {"2#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbHostServer"},
    {"2#subtree#","#krbRealmReferences"},
    {"2#subtree#","#krbTicketFlags"},
    {"2#subtree#","#krbMaxTicketLife"},
    {"2#subtree#","#krbMaxRenewableAge"},
    {"2#subtree#","#krbPrincipalName"},
    {"2#subtree#","#krbPrincipalKey"},
    {"2#subtree#","#krbPrincipalExpiration"},
    {"2#subtree#","#krbPwdPolicyReference"},
    {"2#subtree#","#krbMaxPwdLife"},
    {"2#subtree#","#krbObjectReferences"},
    {"2#subtree#","#krbLastPwdChange"},
    {"6#subtree#","#krbExtraData"},
    {"2#subtree#","#krbPasswordExpiration"},
    {"6#subtree#","#krbLastFailedAuth"},
    {"6#subtree#","#krbLoginFailedCount"},
    {"6#subtree#","#krbLastSuccessfulAuth"},
    { "", "" }
};

static char *adminrights_subtree[][2]={
    {"15#subtree#","#[Entry Rights]"},
    {"6#subtree#","#ObjectClass"},
    {"6#subtree#","#krbTicketPolicyReference"},
    {"6#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbHostServer"},
    {"2#subtree#","#krbRealmReferences"},
    {"6#subtree#","#krbTicketFlags"},
    {"6#subtree#","#krbMaxTicketLife"},
    {"6#subtree#","#krbMaxRenewableAge"},
    {"6#subtree#","#krbPrincipalName"},
    {"6#subtree#","#krbPrincipalKey"},
    {"6#subtree#","#krbPrincipalExpiration"},
    {"6#subtree#","#krbPwdHistoryLength"},
    {"6#subtree#","#krbMinPwdLife"},
    {"6#subtree#","#krbMaxPwdLife"},
    {"6#subtree#","#krbPwdMinDiffChars"},
    {"6#subtree#","#krbPwdMinLength"},
    {"6#subtree#","#krbPwdPolicyReference"},
    {"6#subtree#","#krbLastPwdChange"},
    {"6#subtree#","#krbObjectReferences"},
    {"6#subtree#","#krbExtraData"},
    {"6#subtree#","#krbPasswordExpiration"},
    {"2#subtree#","#krbLastFailedAuth"},
    {"2#subtree#","#krbLoginFailedCount"},
    {"2#subtree#","#krbLastSuccessfulAuth"},
    { "","" }
};

static char *pwdrights_subtree[][2] = {
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#ObjectClass"},
    {"2#subtree#","#krbTicketPolicyReference"},
    {"2#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbHostServer"},
    {"2#subtree#","#krbRealmReferences"},
    {"6#subtree#","#krbTicketFlags"},
    {"2#subtree#","#krbMaxTicketLife"},
    {"2#subtree#","#krbMaxRenewableAge"},
    {"2#subtree#","#krbPrincipalName"},
    {"6#subtree#","#krbPrincipalKey"},
    {"2#subtree#","#krbPrincipalExpiration"},
    {"2#subtree#","#krbPwdHistoryLength"},
    {"2#subtree#","#krbMinPwdLife"},
    {"2#subtree#","#krbMaxPwdLife"},
    {"2#subtree#","#krbPwdMinDiffChars"},
    {"2#subtree#","#krbPwdMinLength"},
    {"2#subtree#","#krbPwdPolicyReference"},
    {"6#subtree#","#krbLastPwdChange"},
    {"2#subtree#","#krbObjectReferences"},
    {"6#subtree#","#krbExtraData"},
    {"6#subtree#","#krbPasswordExpiration"},
    {"2#subtree#","#krbLastFailedAuth"},
    {"2#subtree#","#krbLoginFailedCount"},
    {"2#subtree#","#krbLastSuccessfulAuth"},
    { "", "" }
};

static char *kdcrights_realmcontainer[][2]={
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#CN"},
    {"2#subtree#","#ObjectClass"},
    {"2#subtree#","#krbTicketPolicyReference"},
    {"2#subtree#","#krbMKey"},
    {"2#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbSubTrees"},
    {"2#subtree#","#krbPrincContainerRef"}, 
    {"2#subtree#","#krbSearchScope"},
    {"2#subtree#","#krbLdapServers"},
    {"2#subtree#","#krbKdcServers"},
    {"2#subtree#","#krbAdmServers"},
    {"2#subtree#","#krbPwdServers"},
    {"2#subtree#","#krbTicketFlags"},
    {"2#subtree#","#krbMaxTicketLife"},
    {"2#subtree#","#krbMaxRenewableAge"},
    {"2#subtree#","#krbPrincipalName"},
    {"2#subtree#","#krbPrincipalKey"},
    {"2#subtree#","#krbPrincipalExpiration"},
    {"2#subtree#","#krbPwdPolicyReference"},
    {"2#subtree#","#krbMaxPwdLife"},
    {"2#subtree#","#krbObjectReferences"},
    {"2#subtree#","#krbLastPwdChange"},
    {"6#subtree#","#krbExtraData"},
    {"2#subtree#","#krbPasswordExpiration"},
    {"2#subtree#","#krbDefaultEncSaltTypes"},
    {"6#subtree#","#krbLastFailedAuth"},
    {"6#subtree#","#krbLoginFailedCount"},
    {"6#subtree#","#krbLastSuccessfulAuth"},
    { "", "" }
};


static char *adminrights_realmcontainer[][2]={
    {"15#subtree#","#[Entry Rights]"},
    {"6#subtree#","#CN"},
    {"6#subtree#","#ObjectClass"},
    {"6#subtree#","#krbTicketPolicyReference"},
    {"2#subtree#","#krbMKey"},
    {"6#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbSubTrees"},
    {"2#subtree#","#krbPrincContainerRef"}, 
    {"2#subtree#","#krbSearchScope"},
    {"2#subtree#","#krbLdapServers"},
    {"2#subtree#","#krbKdcServers"},
    {"2#subtree#","#krbAdmServers"},
    {"2#subtree#","#krbPwdServers"},
    {"6#subtree#","#krbTicketFlags"},
    {"6#subtree#","#krbMaxTicketLife"},
    {"6#subtree#","#krbMaxRenewableAge"},
    {"6#subtree#","#krbPrincipalName"},
    {"6#subtree#","#krbPrincipalKey"},
    {"6#subtree#","#krbPrincipalExpiration"},
    {"6#subtree#","#krbPwdHistoryLength"},
    {"6#subtree#","#krbMinPwdLife"},
    {"6#subtree#","#krbMaxPwdLife"},
    {"6#subtree#","#krbPwdMinDiffChars"},
    {"6#subtree#","#krbPwdMinLength"},
    {"6#subtree#","#krbPwdPolicyReference"},
    {"6#subtree#","#krbLastPwdChange"},
    {"6#subtree#","#krbObjectReferences"},
    {"6#subtree#","#krbExtraData"},
    {"6#subtree#","#krbPasswordExpiration"},
    {"6#subtree#","#krbDefaultEncSaltTypes"},
    {"2#subtree#","#krbLastFailedAuth"},
    {"2#subtree#","#krbLoginFailedCount"},
    {"2#subtree#","#krbLastSuccessfulAuth"},
    { "","" }
};


static char *pwdrights_realmcontainer[][2]={
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#CN"},
    {"2#subtree#","#ObjectClass"},
    {"2#subtree#","#krbTicketPolicyReference"},
    {"2#subtree#","#krbMKey"},
    {"2#subtree#","#krbUPEnabled"},
    {"2#subtree#","#krbSubTrees"},
    {"2#subtree#","#krbPrincContainerRef"}, 
    {"2#subtree#","#krbSearchScope"},
    {"2#subtree#","#krbLdapServers"},
    {"2#subtree#","#krbKdcServers"},
    {"2#subtree#","#krbAdmServers"},
    {"2#subtree#","#krbPwdServers"},
    {"6#subtree#","#krbTicketFlags"},
    {"2#subtree#","#krbMaxTicketLife"},
    {"2#subtree#","#krbMaxRenewableAge"},
    {"2#subtree#","#krbPrincipalName"},
    {"6#subtree#","#krbPrincipalKey"},
    {"2#subtree#","#krbPrincipalExpiration"},
    {"2#subtree#","#krbPwdHistoryLength"},
    {"2#subtree#","#krbMinPwdLife"},
    {"2#subtree#","#krbMaxPwdLife"},
    {"2#subtree#","#krbPwdMinDiffChars"},
    {"2#subtree#","#krbPwdMinLength"},
    {"2#subtree#","#krbPwdPolicyReference"},
    {"2#subtree#","#krbLastPwdChange"},
    {"2#subtree#","#krbObjectReferences"},
    {"6#subtree#","#krbExtraData"},
    {"6#subtree#","#krbPasswordExpiration"},
    {"2#subtree#","#krbDefaultEncSaltTypes"},
    {"2#subtree#","#krbLastFailedAuth"},
    {"2#subtree#","#krbLoginFailedCount"},
    {"2#subtree#","#krbLastSuccessfulAuth"},
    { "", "" }
};

static char *security_container[][2] = {
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#krbContainerReference"},
    { "", "" }
};

static char *kerberos_container[][2] = {
    {"1#subtree#","#[Entry Rights]"},
    {"2#subtree#","#krbTicketPolicyReference"},
    { "", "" }
};


/*
 * This will set the rights for the Kerberos service objects.
 * The function will read the subtree attribute from the specified
 * realm name and will the appropriate rights on both the realm
 * container and the subtree. The kerberos context passed should
 * have a valid ldap handle, with appropriate rights to write acl
 * attributes.
 *
 * krb5_context - IN The Kerberos context with valid ldap handle
 *
 */

krb5_error_code
krb5_ldap_add_service_rights(context, servicetype, serviceobjdn, realmname, subtreeparam, contref, mask)
    krb5_context	context;
    int                 servicetype;
    char                *serviceobjdn;
    char                *realmname;
    char                **subtreeparam;                         
    char                *contref;                         
    int                 mask;
{

    int                    st=0,i=0,j=0;
    char                   *realmacls[2]={NULL}, *subtreeacls[2]={NULL}, *seccontacls[2]={NULL}, *krbcontacls[2]={NULL};
    LDAP                   *ld;
    LDAPMod                realmclass, subtreeclass, seccontclass, krbcontclass;
    LDAPMod                *realmarr[3]={NULL}, *subtreearr[3]={NULL}, *seccontarr[3]={NULL}, *krbcontarr[3]={NULL};
    char                   *realmdn=NULL, **subtree=NULL;
    kdb5_dal_handle        *dal_handle=NULL;
    krb5_ldap_context      *ldap_context=NULL;
    krb5_ldap_server_handle *ldap_server_handle=NULL;
    int                     subtreecount=0;

    SETUP_CONTEXT();
    GET_HANDLE();

    if ((serviceobjdn == NULL) || (realmname == NULL) || (servicetype < 0) || (servicetype > 4)
	|| (ldap_context->krbcontainer->DN == NULL)) {
	st=-1;
	goto cleanup;
    }

    if (subtreeparam != NULL) {
        while(subtreeparam[subtreecount])
            subtreecount++;
    }
    if (contref != NULL) {
        subtreecount++;
    }

    if (subtreecount) {
        subtree = (char **) malloc(sizeof(char *) * (subtreecount + 1));
        if(subtree == NULL) {
            st = ENOMEM;
            goto cleanup;
        }
        memset(subtree, 0, sizeof(char *) * (subtreecount + 1));
        if (subtreeparam != NULL) {
            for(i=0; subtreeparam[i]!=NULL; i++) {
            subtree[i] = strdup(subtreeparam[i]);
            if(subtree[i] == NULL) {
                st = ENOMEM;
                goto cleanup;
            }
        }
    }
        if (contref != NULL) {
            subtree[i] = strdup(contref);
        }
    }

    /* Set the rights for the realm */
    if (mask & LDAP_REALM_RIGHTS) {

    /* Set the rights for the service object on the security container */
    seccontclass.mod_op = LDAP_MOD_ADD;
    seccontclass.mod_type = "ACL";

    for (i=0; strcmp(security_container[i][0], "") != 0; i++) {

	asprintf(&seccontacls[0], "%s%s%s", security_container[i][0], serviceobjdn,
		 security_container[i][1]);
	seccontclass.mod_values = seccontacls;

	seccontarr[0] = &seccontclass;

	st = ldap_modify_ext_s(ld,
			       SECURITY_CONTAINER,
			       seccontarr,
			       NULL,
			       NULL);
	if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
	    free(seccontacls[0]);
	    st = set_ldap_error (context, st, OP_MOD);
	    goto cleanup;
	}
	free(seccontacls[0]);
    }


    /* Set the rights for the service object on the kerberos container */
    krbcontclass.mod_op = LDAP_MOD_ADD;
    krbcontclass.mod_type = "ACL";

    for (i=0; strcmp(kerberos_container[i][0], "") != 0; i++) {
	asprintf(&krbcontacls[0], "%s%s%s", kerberos_container[i][0], serviceobjdn,
		 kerberos_container[i][1]);
	krbcontclass.mod_values = krbcontacls;

	krbcontarr[0] = &krbcontclass;

	st = ldap_modify_ext_s(ld,
			       ldap_context->krbcontainer->DN,
			       krbcontarr,
			       NULL,
			       NULL);
	if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
	    free(krbcontacls[0]);
	    st = set_ldap_error (context, st, OP_MOD);
	    goto cleanup;
	}
	free(krbcontacls[0]);
    }

	/* Construct the realm dn from realm name */
	asprintf(&realmdn,"cn=%s,%s", realmname, ldap_context->krbcontainer->DN);

	realmclass.mod_op = LDAP_MOD_ADD;
	realmclass.mod_type = "ACL";

	if (servicetype == LDAP_KDC_SERVICE) {
	    for (i=0; strcmp(kdcrights_realmcontainer[i][0], "") != 0; i++) {
		asprintf(&realmacls[0], "%s%s%s", kdcrights_realmcontainer[i][0], serviceobjdn,
			 kdcrights_realmcontainer[i][1]);
		realmclass.mod_values = realmacls;

		realmarr[0] = &realmclass;

		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	} else if (servicetype == LDAP_ADMIN_SERVICE) {
	    for (i=0; strcmp(adminrights_realmcontainer[i][0], "") != 0; i++) {
		asprintf(&realmacls[0], "%s%s%s", adminrights_realmcontainer[i][0], serviceobjdn,
			 adminrights_realmcontainer[i][1]);
		realmclass.mod_values = realmacls;

		realmarr[0] = &realmclass;

		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	} else if (servicetype == LDAP_PASSWD_SERVICE) {
	    for (i=0; strcmp(pwdrights_realmcontainer[i][0], "")!=0; i++) {
		asprintf(&realmacls[0], "%s%s%s", pwdrights_realmcontainer[i][0], serviceobjdn,
			 pwdrights_realmcontainer[i][1]);
		realmclass.mod_values = realmacls;

		realmarr[0] = &realmclass;


		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	}
    } /* Realm rights settings ends here */


    /* Subtree rights to be set */
    if ((mask & LDAP_SUBTREE_RIGHTS) && (subtree != NULL)) {
	/* Populate the acl data to be added to the subtree */
	subtreeclass.mod_op = LDAP_MOD_ADD;
	subtreeclass.mod_type = "ACL";

	if (servicetype == LDAP_KDC_SERVICE) {
	    for (i=0; strcmp(kdcrights_subtree[i][0], "")!=0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", kdcrights_subtree[i][0], serviceobjdn,
			 kdcrights_subtree[i][1]);
		subtreeclass.mod_values = subtreeacls;

		subtreearr[0] = &subtreeclass;

                /* set rights to a list of subtrees */
                for(j=0; subtree[j]!=NULL && j<subtreecount;j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	} else if (servicetype == LDAP_ADMIN_SERVICE) {
	    for (i=0; strcmp(adminrights_subtree[i][0], "")!=0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", adminrights_subtree[i][0], serviceobjdn,
			 adminrights_subtree[i][1]);
		subtreeclass.mod_values = subtreeacls;

		subtreearr[0] = &subtreeclass;

                /* set rights to a list of subtrees */
                for(j=0; subtree[j]!=NULL && j<subtreecount;j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st !=LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	} else if (servicetype == LDAP_PASSWD_SERVICE) {
	    for (i=0; strcmp(pwdrights_subtree[i][0], "") != 0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", pwdrights_subtree[i][0], serviceobjdn,
			 pwdrights_subtree[i][1]);
		subtreeclass.mod_values = subtreeacls;

		subtreearr[0] = &subtreeclass;

                /* set rights to a list of subtrees */
                for(j=0; subtree[j]!=NULL && j<subtreecount;j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st != LDAP_TYPE_OR_VALUE_EXISTS && st != LDAP_OTHER) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	}
    } /* Subtree rights settings ends here */
    st = 0;

cleanup:

    if (realmdn)
	free(realmdn);

    if (subtree)
	free(subtree);

    krb5_ldap_put_handle_to_pool(ldap_context, ldap_server_handle);
    return st;
}


/*
  This will set the rights for the Kerberos service objects.
  The function will read the subtree attribute from the specified
  realm name and will the appropriate rights on both the realm
  container and the subtree. The kerberos context passed should
  have a valid ldap handle, with appropriate rights to write acl
  attributes.

  krb5_context - IN The Kerberos context with valid ldap handle

*/

krb5_error_code
krb5_ldap_delete_service_rights(context, servicetype, serviceobjdn, realmname, subtreeparam, contref, mask)
    krb5_context	context;
    int             servicetype;
    char            *serviceobjdn;
    char            *realmname;
    char            **subtreeparam; 
    char            *contref;
    int             mask;
{

    int                    st=0,i=0,j=0;
    char                   *realmacls[2] = { NULL }, *subtreeacls[2] = { NULL };
    LDAP                   *ld;
    LDAPMod                realmclass, subtreeclass;
    LDAPMod                *realmarr[3] = { NULL }, *subtreearr[3] = { NULL };
    char                   *realmdn=NULL;
    char                   **subtree=NULL;
    kdb5_dal_handle        *dal_handle=NULL;
    krb5_ldap_context      *ldap_context=NULL;
    krb5_ldap_server_handle *ldap_server_handle=NULL;
    int                     subtreecount = 0;  

    SETUP_CONTEXT();
    GET_HANDLE();

    if ((serviceobjdn == NULL) || (realmname == NULL) || (servicetype < 0) || (servicetype > 4)
	|| (ldap_context->krbcontainer->DN == NULL)) {
	st = -1;
	goto cleanup;
    }

    if (subtreeparam != NULL) {
    while(subtreeparam[subtreecount])
        subtreecount++;
    }
    if (contref != NULL) {
        subtreecount++;
    }

    if (subtreecount) {
        subtree = (char **) malloc(sizeof(char *) * (subtreecount + 1));
        if(subtree == NULL) {
            st = ENOMEM;
            goto cleanup;
        }
        memset(subtree, 0, sizeof(char *) * (subtreecount + 1));
        if (subtreeparam != NULL) {
            for(i=0; subtreeparam[i]!=NULL; i++) {
        subtree[i] = strdup(subtreeparam[i]);
        if(subtree[i] == NULL) {
            st = ENOMEM;
            goto cleanup;
        }
    }
        }
        if (contref != NULL) {
            subtree[i] = strdup(contref);
        }
    }


    /* Set the rights for the realm */
    if (mask & LDAP_REALM_RIGHTS) {

	asprintf(&realmdn,"cn=%s,%s", realmname, ldap_context->krbcontainer->DN);

	realmclass.mod_op=LDAP_MOD_DELETE;
	realmclass.mod_type="ACL";

	if (servicetype == LDAP_KDC_SERVICE) {
	    for (i=0; strcmp(kdcrights_realmcontainer[i][0], "") != 0; i++) {
		asprintf(&realmacls[0], "%s%s%s", kdcrights_realmcontainer[i][0], serviceobjdn,
			 kdcrights_realmcontainer[i][1]);
		realmclass.mod_values= realmacls;

		realmarr[0]=&realmclass;

		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	} else if (servicetype == LDAP_ADMIN_SERVICE) {
	    for (i=0; strcmp(adminrights_realmcontainer[i][0], "") != 0; i++) {
		asprintf(&realmacls[0], "%s%s%s", adminrights_realmcontainer[i][0], serviceobjdn,
			 adminrights_realmcontainer[i][1]);
		realmclass.mod_values= realmacls;

		realmarr[0]=&realmclass;

		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	} else if (servicetype == LDAP_PASSWD_SERVICE) {
	    for (i=0; strcmp(pwdrights_realmcontainer[i][0], "") != 0; i++) {
		asprintf(&realmacls[0], "%s%s%s", pwdrights_realmcontainer[i][0], serviceobjdn,
			 pwdrights_realmcontainer[i][1]);
		realmclass.mod_values= realmacls;

		realmarr[0]=&realmclass;

		st = ldap_modify_ext_s(ld,
				       realmdn,
				       realmarr,
				       NULL,
				       NULL);
		if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		    free(realmacls[0]);
		    st = set_ldap_error (context, st, OP_MOD);
		    goto cleanup;
		}
		free(realmacls[0]);
	    }
	}

    } /* Realm rights setting ends here */


    /* Set the rights for the subtree */
    if ((mask & LDAP_SUBTREE_RIGHTS) && (subtree != NULL)) {

	/* Populate the acl data to be added to the subtree */
	subtreeclass.mod_op=LDAP_MOD_DELETE;
	subtreeclass.mod_type="ACL";

	if (servicetype == LDAP_KDC_SERVICE) {
	    for (i=0; strcmp(kdcrights_subtree[i][0], "")!=0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", kdcrights_subtree[i][0], serviceobjdn,
			 kdcrights_subtree[i][1]);
		subtreeclass.mod_values= subtreeacls;

		subtreearr[0]=&subtreeclass;

                for(j=0; subtree[j]!=NULL && j<subtreecount; j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	} else if (servicetype == LDAP_ADMIN_SERVICE) {
	    for (i=0; strcmp(adminrights_subtree[i][0], "") != 0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", adminrights_subtree[i][0], serviceobjdn,
			 adminrights_subtree[i][1]);
		subtreeclass.mod_values= subtreeacls;

		subtreearr[0]=&subtreeclass;

                for(j=0; subtree[j]!=NULL && j<subtreecount; j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	} else if (servicetype == LDAP_PASSWD_SERVICE) {
	    for (i=0; strcmp(pwdrights_subtree[i][0], "") != 0; i++) {
		asprintf(&subtreeacls[0], "%s%s%s", pwdrights_subtree[i][0], serviceobjdn,
			 pwdrights_subtree[i][1]);
		subtreeclass.mod_values= subtreeacls;

		subtreearr[0]=&subtreeclass;

                for(j=0; subtree[j]!=NULL && j<subtreecount; j++) {
		    st = ldap_modify_ext_s(ld,
                                            subtree[j],
                                            subtreearr,
                                            NULL,
                                            NULL);
		    if (st != LDAP_SUCCESS && st != LDAP_NO_SUCH_ATTRIBUTE) {
		        free(subtreeacls[0]);
		        st = set_ldap_error (context, st, OP_MOD);
		        goto cleanup;
		    }
                }
		free(subtreeacls[0]);
	    }
	}
    } /* Subtree rights setting ends here */

    st = 0;

cleanup:

    if (realmdn)
	free(realmdn);

    if (subtree)
	free(subtree);

    krb5_ldap_put_handle_to_pool(ldap_context, ldap_server_handle);
    return st;
}

#endif
