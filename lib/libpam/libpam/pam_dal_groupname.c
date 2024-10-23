/*
 * Copyright (c) 2024 Digi International Inc.,
 *  9350 Excelsior Blvd, Suite 700, Hopkins, MN 55343
 *
 * pam_dal_groupname
 *      Extract group names from group attribute
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "pam_private.h"
#include <security/pam_dal.h>

static void free_groupname_data(pam_handle_t *pamh, void *groupname, int error_status)
{
	free(groupname);
}

int pam_dal_set_groupname_from_attr(pam_handle_t *pamh, const char *groupattr, size_t groupattr_len)
{
	const char *groupattr_end, *group_start, *group_end;
	int ret = PAM_SERVICE_ERR;
	size_t count = 1, grouptext_len = 0;
	bool escaped = false;

	if (groupattr_len == 0)
		return PAM_SUCCESS;
	groupattr_end = groupattr + groupattr_len - 1;
	group_start = group_end = groupattr;
	while (group_end <= groupattr_end && *group_end)
	{
		if ((*group_end == ',' && !escaped) || group_end == groupattr_end)
		{
			if (group_end != group_start)
			{
				if (group_end == groupattr_end)
					grouptext_len++;
				count++;
			}
			group_start = group_end = group_end + 1;
			continue;
		}
		if (escaped)
			escaped = false;
		else if (*group_end == '\\')
			escaped = true;
		if (!escaped)
			grouptext_len++;
		group_end++;
	}
	if (count > 1)
	{
		char *grouptext_start, *grouptext;
		char **groupname = malloc((sizeof *groupname * count) + grouptext_len + 1);
		if (!groupname)
			return PAM_BUF_ERR;
		grouptext_start = grouptext = (char *) &groupname[count];
		count = 0;
		group_start = group_end = groupattr;
		escaped = false;
		while (group_end <= groupattr_end && *group_end)
		{
			if ((*group_end == ',' && !escaped) || group_end == groupattr_end)
			{
				if (group_end != group_start)
				{
					if (group_end == groupattr_end)
						*grouptext++ = *group_end;
					*grouptext++ = '\0';
					groupname[count] = grouptext_start;
					count++;
				}
				grouptext_start = grouptext;
				group_start = group_end = group_end + 1;
				continue;
			}
			if (escaped)
				escaped = false;
			else if (*group_end == '\\')
				escaped = true;
			if (!escaped)
				*grouptext++ = *group_end;
			group_end++;
		}
		groupname[count] = NULL;

		ret = pam_set_data(pamh, "DAL_GROUPNAME", groupname, free_groupname_data);
		if (ret != PAM_SUCCESS)
			free_groupname_data(NULL, groupname, 0);
	}
	return ret;
}
