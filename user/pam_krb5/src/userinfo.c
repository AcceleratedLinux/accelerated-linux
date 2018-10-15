/*
 * Copyright 2003,2004,2005,2006,2009 Red Hat, Inc.
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
 * ALTERNATIVELY, this product may be distributed under the terms of the
 * GNU Lesser General Public License, in which case the provisions of the
 * LGPL are required INSTEAD OF the above restrictions.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../config.h"

#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif

#ifdef HAVE_SECURITY_PAM_MODULES_H
#include <security/pam_modules.h>
#endif

#include KRB5_H
#ifdef USE_KRB4
#include KRB4_DES_H
#include KRB4_KRB_H
#ifdef KRB4_KRB_ERR_H
#include KRB4_KRB_ERR_H
#endif
#endif

#include "log.h"
#include "map.h"
#include "userinfo.h"
#include "v5.h"
#include "xstr.h"

#if defined(HAVE_GETPWNAM_R) || defined(HAVE___POSIX_GETPWNAM_R)
#define CHUNK_SIZE 128
/* Convert a name to a UID/GID pair. */
static int
_get_pw_nam(const char *name, uid_t *uid, gid_t *gid, char **homedir)
{
	struct passwd passwd, *pwd;
	char *buffer;
	int size, i;

	size = CHUNK_SIZE;
	do {
		/* Allocate a temporary buffer to hold the string data. */
		buffer = malloc(size);
		if (buffer == NULL) {
			return 1;
		}
		memset(buffer, '\0', size);

		/* Give it a shot. */
		pwd = NULL;
#if defined(HAVE_GETPWNAM_R) && !defined(sun)
		i = getpwnam_r(name, &passwd, buffer, size, &pwd);
#else
		i = __posix_getpwnam_r(name, &passwd, buffer, size, &pwd);
#endif

		/* If we got 0 back, AND pwd now points to the passwd
		 * structure, then we succeeded. */
		if ((i == 0) && (pwd == &passwd)) {
			break;
		}

		/* Free the buffer -- we'll reallocate it later. */
		xstrfree(buffer);
		buffer = NULL;

		/* We need to use more space if we got ERANGE back, and errno
		 * is ERANGE, so bail on any other condition. */
		if ((i != ERANGE) || (errno != ERANGE)) {
			return 1;
		}

		/* Increase the size of the buffer. */
		size += CHUNK_SIZE;
	} while (size > 0);

	/* If we exited successfully, then pull out the UID/GID. */
	if ((i == 0) && (pwd != NULL) && (buffer != NULL)) {
		*uid = pwd->pw_uid;
		*gid = pwd->pw_gid;
		*homedir = xstrdup(pwd->pw_dir);
		free(buffer);
		return 0;
	}

	/* Failed. */
	if (buffer != NULL) {
		free(buffer);
	}
	return 1;
}
#else
static int
_get_pw_nam(const char *name, uid_t *uid, gid_t *gid, char **homedir)
{
	struct passwd *pwd;
	pwd = getpwnam(name);
	if (pwd != NULL) {
		*uid = pwd->pw_uid;
		*gid = pwd->pw_gid;
		*homedir = xstrdup(pwd->pw_dir);
		return 0;
	}
	return 1;
}
#endif

struct _pam_krb5_user_info *
_pam_krb5_user_info_init(krb5_context ctx, const char *name,
			 struct _pam_krb5_options *options)
{
	struct _pam_krb5_user_info *ret = NULL;
	char local_name[LINE_MAX];
	char qualified_name[LINE_MAX];
	char mapped_name[LINE_MAX];

	ret = malloc(sizeof(struct _pam_krb5_user_info));
	if (ret == NULL) {
		return NULL;
	}
	memset(ret, 0, sizeof(struct _pam_krb5_user_info));

	/* See if we need to map this user name to a principal somehow. */
	if (map_lname_aname(options->mappings, options->n_mappings,
			    name, mapped_name, sizeof(mapped_name)) == 0) {
		if (strchr(mapped_name, '@') == NULL) {
			if (strlen(mapped_name) + 1 + strlen(options->realm) >=
			    sizeof(qualified_name)) {
				warn("principal name '%s' too long",
				     mapped_name);
				free(ret);
				return NULL;
			}
			snprintf(qualified_name, sizeof(qualified_name),
				 "%s@%s", mapped_name, options->realm);
		} else {
			if (strlen(mapped_name) >= sizeof(qualified_name)) {
				warn("principal name '%s' too long",
				     mapped_name);
				free(ret);
				return NULL;
			}
			snprintf(qualified_name, sizeof(qualified_name),
				 "%s", mapped_name);
		}
	} else {
		if (strchr(name, '@') == NULL) {
			if (strlen(name) + 1 + strlen(options->realm) >=
			    sizeof(qualified_name)) {
				warn("principal name '%s' too long", name);
				free(ret);
				return NULL;
			}
			snprintf(qualified_name, sizeof(qualified_name),
				 "%s@%s", name, options->realm);
		} else {
			if (strlen(name) >= sizeof(qualified_name)) {
				warn("principal name '%s' too long", name);
				free(ret);
				return NULL;
			}
			snprintf(qualified_name, sizeof(qualified_name),
				 "%s", name);
		}
	}

	/* Parse the user's determined principal name into a principal
	 * structure. */
	if (v5_parse_name(ctx, options, qualified_name,
			  &ret->principal_name) != 0) {
		warn("error parsing principal name '%s' derived from "
		     "user name '%s'", qualified_name, name);
		free(ret);
		return NULL;
	}

	/* Convert the principal back to a full principal name string. */
	if (krb5_unparse_name(ctx, ret->principal_name,
			      &ret->unparsed_name) != 0) {
		warn("error converting principal name to string");
		krb5_free_principal(ctx, ret->principal_name);
		free(ret);
		return NULL;
	}

	/* Use the local user name which the user gave us. */
	strncpy(local_name, name, sizeof(local_name) - 1);
	local_name[sizeof(local_name) - 1] = '\0';

	if (options->user_check) {
		/* Look up the user's UID/GID. */
		if (_get_pw_nam(local_name,
				&ret->uid, &ret->gid,
				&ret->homedir) != 0) {
			warn("error resolving user name '%s' to uid/gid pair",
			     local_name);
			v5_free_unparsed_name(ctx, ret->unparsed_name);
			krb5_free_principal(ctx, ret->principal_name);
			free(ret);
			return NULL;
		}
	} else {
		/* Set things to the current UID/GID. */
		ret->uid = -1;
		ret->gid = -1;
		ret->homedir = xstrdup("/");
	}

	return ret;
}

void
_pam_krb5_user_info_free(krb5_context ctx, struct _pam_krb5_user_info *info)
{
	krb5_free_principal(ctx, info->principal_name);
	v5_free_unparsed_name(ctx, info->unparsed_name);
	xstrfree(info->homedir);
	memset(info, 0, sizeof(struct _pam_krb5_user_info));
	free(info);
}
