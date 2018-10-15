/* 
   Unix SMB/CIFS implementation.

   Winbind rpc backend functions

   Copyright (C) Tim Potter 2000-2001,2003
   Copyright (C) Simo Sorce 2003
   Copyright (C) Volker Lendecke 2004
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "includes.h"
#include "winbindd.h"

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_WINBIND

/* Query display info for a domain.  This returns enough information plus a
   bit extra to give an overview of domain users for the User Manager
   application. */
static NTSTATUS query_user_list(struct winbindd_domain *domain,
			       TALLOC_CTX *mem_ctx,
			       uint32 *num_entries, 
			       WINBIND_USERINFO **info)
{
	/* We don't have users */
	*num_entries = 0;
	*info = NULL;
	return NT_STATUS_OK;
}

/* list all domain groups */
static NTSTATUS enum_dom_groups(struct winbindd_domain *domain,
				TALLOC_CTX *mem_ctx,
				uint32 *num_entries, 
				struct acct_info **info)
{
	/* We don't have domain groups */
	*num_entries = 0;
	*info = NULL;
	return NT_STATUS_OK;
}

/* List all domain groups */

static NTSTATUS enum_local_groups(struct winbindd_domain *domain,
				TALLOC_CTX *mem_ctx,
				uint32 *num_entries, 
				struct acct_info **info)
{
	struct pdb_search *search;
	struct samr_displayentry *aliases;
	int i;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;

	search = pdb_search_aliases(&domain->sid);
	if (search == NULL) goto done;

	*num_entries = pdb_search_entries(search, 0, 0xffffffff, &aliases);
	if (*num_entries == 0) goto done;

	*info = TALLOC_ARRAY(mem_ctx, struct acct_info, *num_entries);
	if (*info == NULL) {
		result = NT_STATUS_NO_MEMORY;
		goto done;
	}

	for (i=0; i<*num_entries; i++) {
		fstrcpy((*info)[i].acct_name, aliases[i].account_name);
		fstrcpy((*info)[i].acct_desc, aliases[i].description);
		(*info)[i].rid = aliases[i].rid;
	}

	result = NT_STATUS_OK;
 done:
	pdb_search_destroy(search);
	return result;
}

/* convert a single name to a sid in a domain */
static NTSTATUS name_to_sid(struct winbindd_domain *domain,
			    TALLOC_CTX *mem_ctx,
			    const char *domain_name,
			    const char *name,
			    DOM_SID *sid,
			    enum lsa_SidType *type)
{
	DEBUG(10, ("Finding name %s\n", name));

	if ( !lookup_name( mem_ctx, name, LOOKUP_NAME_ALL, 
		NULL, NULL, sid, type ) )
	{
		return NT_STATUS_NONE_MAPPED;
	}

	return NT_STATUS_OK;
}

/*
  convert a domain SID to a user or group name
*/
static NTSTATUS sid_to_name(struct winbindd_domain *domain,
			    TALLOC_CTX *mem_ctx,
			    const DOM_SID *sid,
			    char **domain_name,
			    char **name,
			    enum lsa_SidType *type)
{
	const char *dom, *nam;

	DEBUG(10, ("Converting SID %s\n", sid_string_static(sid)));

	/* Paranoia check */
	if (!sid_check_is_in_builtin(sid) &&
	    !sid_check_is_in_our_domain(sid)) {
		DEBUG(0, ("Possible deadlock: Trying to lookup SID %s with "
			  "passdb backend\n", sid_string_static(sid)));
		return NT_STATUS_NONE_MAPPED;
	}

	if (!lookup_sid(mem_ctx, sid, &dom, &nam, type)) {
		return NT_STATUS_NONE_MAPPED;
	}

	*domain_name = talloc_strdup(mem_ctx, dom);
	*name = talloc_strdup(mem_ctx, nam);

	return NT_STATUS_OK;
}

static NTSTATUS rids_to_names(struct winbindd_domain *domain,
			      TALLOC_CTX *mem_ctx,
			      const DOM_SID *sid,
			      uint32 *rids,
			      size_t num_rids,
			      char **domain_name,
			      char ***names,
			      enum lsa_SidType **types)
{
	return NT_STATUS_UNSUCCESSFUL;
}

/* Lookup user information from a rid or username. */
static NTSTATUS query_user(struct winbindd_domain *domain, 
			   TALLOC_CTX *mem_ctx, 
			   const DOM_SID *user_sid,
			   WINBIND_USERINFO *user_info)
{
	return NT_STATUS_NO_SUCH_USER;
}

/* Lookup groups a user is a member of.  I wish Unix had a call like this! */
static NTSTATUS lookup_usergroups(struct winbindd_domain *domain,
				  TALLOC_CTX *mem_ctx,
				  const DOM_SID *user_sid,
				  uint32 *num_groups, DOM_SID **user_gids)
{
	NTSTATUS result;
	DOM_SID *groups = NULL;
	gid_t *gids = NULL;
	size_t ngroups = 0;
	struct samu *user;

	if ( (user = samu_new(mem_ctx)) == NULL ) {
		return NT_STATUS_NO_MEMORY;
	}

	if ( !pdb_getsampwsid( user, user_sid ) ) {
		return NT_STATUS_NO_SUCH_USER;
	}

	result = pdb_enum_group_memberships( mem_ctx, user, &groups, &gids, &ngroups );

	TALLOC_FREE( user );

	*num_groups = (uint32)ngroups;
	*user_gids = groups;

	return result;
}

static NTSTATUS lookup_useraliases(struct winbindd_domain *domain,
				   TALLOC_CTX *mem_ctx,
				   uint32 num_sids, const DOM_SID *sids,
				   uint32 *p_num_aliases, uint32 **rids)
{
	NTSTATUS result;
	size_t num_aliases = 0;

	result = pdb_enum_alias_memberships(mem_ctx, &domain->sid,
					    sids, num_sids, rids, &num_aliases);

	*p_num_aliases = num_aliases;
	return result;
}

/* Lookup group membership given a rid.   */
static NTSTATUS lookup_groupmem(struct winbindd_domain *domain,
				TALLOC_CTX *mem_ctx,
				const DOM_SID *group_sid, uint32 *num_names, 
				DOM_SID **sid_mem, char ***names, 
				uint32 **name_types)
{
	size_t i, num_members, num_mapped;
	uint32 *rids;
	NTSTATUS result;
	const DOM_SID **sids;
	struct lsa_dom_info *lsa_domains;
	struct lsa_name_info *lsa_names;
	TALLOC_CTX *tmp_ctx;

	if (!sid_check_is_in_our_domain(group_sid)) {
		/* There's no groups, only aliases in BUILTIN */
		return NT_STATUS_NO_SUCH_GROUP;
	}

	if (!(tmp_ctx = talloc_init("lookup_groupmem"))) {
		return NT_STATUS_NO_MEMORY;
	}

	result = pdb_enum_group_members(tmp_ctx, group_sid, &rids,
					&num_members);
	if (!NT_STATUS_IS_OK(result)) {
		TALLOC_FREE(tmp_ctx);
		return result;
	}

	if (num_members == 0) {
		*num_names = 0;
		*sid_mem = NULL;
		*names = NULL;
		*name_types = NULL;
		TALLOC_FREE(tmp_ctx);
		return NT_STATUS_OK;
	}

	*sid_mem = TALLOC_ARRAY(mem_ctx, DOM_SID, num_members);
	*names = TALLOC_ARRAY(mem_ctx, char *, num_members);
	*name_types = TALLOC_ARRAY(mem_ctx, uint32, num_members);
	sids = TALLOC_ARRAY(tmp_ctx, const DOM_SID *, num_members);

	if (((*sid_mem) == NULL) || ((*names) == NULL) ||
	    ((*name_types) == NULL) || (sids == NULL)) {
		TALLOC_FREE(tmp_ctx);
		return NT_STATUS_NO_MEMORY;
	}

	/*
	 * Prepare an array of sid pointers for the lookup_sids calling
	 * convention.
	 */

	for (i=0; i<num_members; i++) {
		DOM_SID *sid = &((*sid_mem)[i]);
		if (!sid_compose(sid, &domain->sid, rids[i])) {
			TALLOC_FREE(tmp_ctx);
			return NT_STATUS_INTERNAL_ERROR;
		}
		sids[i] = sid;
	}

	result = lookup_sids(tmp_ctx, num_members, sids, 1,
			     &lsa_domains, &lsa_names);
	if (!NT_STATUS_IS_OK(result)) {
		TALLOC_FREE(tmp_ctx);
		return result;
	}

	num_mapped = 0;
	for (i=0; i<num_members; i++) {
		if (lsa_names[i].type != SID_NAME_USER) {
			DEBUG(2, ("Got %s as group member -- ignoring\n",
				  sid_type_lookup(lsa_names[i].type)));
			continue;
		}
		if (!((*names)[i] = talloc_strdup((*names),
						  lsa_names[i].name))) {
			TALLOC_FREE(tmp_ctx);
			return NT_STATUS_NO_MEMORY;
		}

		(*name_types)[i] = lsa_names[i].type;

		num_mapped += 1;
	}

	*num_names = num_mapped;

	TALLOC_FREE(tmp_ctx);
	return NT_STATUS_OK;
}

/* find the sequence number for a domain */
static NTSTATUS sequence_number(struct winbindd_domain *domain, uint32 *seq)
{
	BOOL result;
	time_t seq_num;

	result = pdb_get_seq_num(&seq_num);
	if (!result) {
		*seq = 1;
	}

	*seq = (int) seq_num;
	/* *seq = 1; */
	return NT_STATUS_OK;
}

static NTSTATUS lockout_policy(struct winbindd_domain *domain,
			       TALLOC_CTX *mem_ctx,
			       SAM_UNK_INFO_12 *policy)
{
	/* actually we have that */
	return NT_STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS password_policy(struct winbindd_domain *domain,
				TALLOC_CTX *mem_ctx,
				SAM_UNK_INFO_1 *policy)
{
	uint32 min_pass_len,pass_hist,password_properties;
	time_t u_expire, u_min_age;
	NTTIME nt_expire, nt_min_age;
	uint32 account_policy_temp;

	if ((policy = TALLOC_ZERO_P(mem_ctx, SAM_UNK_INFO_1)) == NULL) {
		return NT_STATUS_NO_MEMORY;
	}

	if (!pdb_get_account_policy(AP_MIN_PASSWORD_LEN, &account_policy_temp)) {
		return NT_STATUS_ACCESS_DENIED;
	}
	min_pass_len = account_policy_temp;

	if (!pdb_get_account_policy(AP_PASSWORD_HISTORY, &account_policy_temp)) {
		return NT_STATUS_ACCESS_DENIED;
	}
	pass_hist = account_policy_temp;

	if (!pdb_get_account_policy(AP_USER_MUST_LOGON_TO_CHG_PASS, &account_policy_temp)) {
		return NT_STATUS_ACCESS_DENIED;
	}
	password_properties = account_policy_temp;
	
	if (!pdb_get_account_policy(AP_MAX_PASSWORD_AGE, &account_policy_temp)) {
		return NT_STATUS_ACCESS_DENIED;
	}
	u_expire = account_policy_temp;

	if (!pdb_get_account_policy(AP_MIN_PASSWORD_AGE, &account_policy_temp)) {
		return NT_STATUS_ACCESS_DENIED;
	}
	u_min_age = account_policy_temp;

	unix_to_nt_time_abs(&nt_expire, u_expire);
	unix_to_nt_time_abs(&nt_min_age, u_min_age);

	init_unk_info1(policy, (uint16)min_pass_len, (uint16)pass_hist, 
	               password_properties, nt_expire, nt_min_age);

	return NT_STATUS_OK;
}

/* get a list of trusted domains */
static NTSTATUS trusted_domains(struct winbindd_domain *domain,
				TALLOC_CTX *mem_ctx,
				uint32 *num_domains,
				char ***names,
				char ***alt_names,
				DOM_SID **dom_sids)
{
	NTSTATUS nt_status;
	struct trustdom_info **domains;
	int i;
	TALLOC_CTX *tmp_ctx;

	*num_domains = 0;
	*names = NULL;
	*alt_names = NULL;
	*dom_sids = NULL;

	if (!(tmp_ctx = talloc_init("trusted_domains"))) {
		return NT_STATUS_NO_MEMORY;
	}

	nt_status = secrets_trusted_domains(tmp_ctx, num_domains,
					    &domains);
	if (!NT_STATUS_IS_OK(nt_status)) {
		TALLOC_FREE(tmp_ctx);
		return nt_status;
	}

	if (*num_domains) {
		*names = TALLOC_ARRAY(mem_ctx, char *, *num_domains);
		*alt_names = TALLOC_ARRAY(mem_ctx, char *, *num_domains);
		*dom_sids = TALLOC_ARRAY(mem_ctx, DOM_SID, *num_domains);

		if ((*alt_names == NULL) || (*names == NULL) || (*dom_sids == NULL)) {
			TALLOC_FREE(tmp_ctx);
			return NT_STATUS_NO_MEMORY;
		}
	} else {
		*names = NULL;
		*alt_names = NULL;
		*dom_sids = NULL;
	}

	for (i=0; i<*num_domains; i++) {
		(*alt_names)[i] = NULL;
		if (!((*names)[i] = talloc_strdup((*names),
						  domains[i]->name))) {
			TALLOC_FREE(tmp_ctx);
			return NT_STATUS_NO_MEMORY;
		}
		sid_copy(&(*dom_sids)[i], &domains[i]->sid);
	}

	TALLOC_FREE(tmp_ctx);
	return NT_STATUS_OK;
}

/* the rpc backend methods are exposed via this structure */
struct winbindd_methods passdb_methods = {
	False,
	query_user_list,
	enum_dom_groups,
	enum_local_groups,
	name_to_sid,
	sid_to_name,
	rids_to_names,
	query_user,
	lookup_usergroups,
	lookup_useraliases,
	lookup_groupmem,
	sequence_number,
	lockout_policy,
	password_policy,
	trusted_domains,
};
