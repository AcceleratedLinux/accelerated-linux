/*
 * Copyright 1989 - 1994, Julianne Frances Haugh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config.h>

/* Newer versions of Linux libc already have shadow support.  */
#ifndef HAVE_GETSPNAM

#ident "$Id: shadow.c,v 1.12 2005/08/31 17:24:56 kloczek Exp $"

#include <sys/types.h>
#include "prototypes.h"
#include "defines.h"
#include <stdio.h>
#ifdef	USE_NIS
static int nis_used;
static int nis_ignore;
static enum { native, start, middle, native2 } nis_state;
static int nis_bound;
static char *nis_domain;
static char *nis_key;
static int nis_keylen;
static char *nis_val;
static int nis_vallen;

#define	IS_NISCHAR(c) ((c)=='+')
#endif

static FILE *shadow;
static char spwbuf[BUFSIZ];
static struct spwd spwd;

#define	FIELDS	9
#define	OFIELDS	5

#ifdef	USE_NIS

/*
 * __setspNIS - turn on or off NIS searches
 */

void __setspNIS (int flag)
{
	nis_ignore = !flag;

	if (nis_ignore)
		nis_used = 0;
}

/*
 * bind_nis - bind to NIS server
 */

static int bind_nis (void)
{
	if (yp_get_default_domain (&nis_domain))
		return -1;

	nis_bound = 1;
	return 0;
}
#endif

/*
 * setspent - initialize access to shadow text and DBM files
 */

void setspent (void)
{
	if (shadow)
		rewind (shadow);
	else
		shadow = fopen (SHADOW_FILE, "r");

#ifdef	USE_NIS
	nis_state = native;
#endif
}

/*
 * endspent - terminate access to shadow text and DBM files
 */

void endspent (void)
{
	if (shadow)
		(void) fclose (shadow);

	shadow = (FILE *) 0;
}

/*
 * my_sgetspent - convert string in shadow file format to (struct spwd *)
 */

static struct spwd *my_sgetspent (const char *string)
{
	char *fields[FIELDS];
	char *cp;
	char *cpp;
	int i;

	/*
	 * Copy string to local buffer.  It has to be tokenized and we
	 * have to do that to our private copy.
	 */

	if (strlen (string) >= sizeof spwbuf)
		return 0;
	strcpy (spwbuf, string);

	if ((cp = strrchr (spwbuf, '\n')))
		*cp = '\0';

	/*
	 * Tokenize the string into colon separated fields.  Allow up to
	 * FIELDS different fields.
	 */

	for (cp = spwbuf, i = 0; *cp && i < FIELDS; i++) {
		fields[i] = cp;
		while (*cp && *cp != ':')
			cp++;

		if (*cp)
			*cp++ = '\0';
	}

	if (i == (FIELDS - 1))
		fields[i++] = cp;

	if ((cp && *cp) || (i != FIELDS && i != OFIELDS))
		return 0;

	/*
	 * Start populating the structure.  The fields are all in
	 * static storage, as is the structure we pass back.  If we
	 * ever see a name with '+' as the first character, we try
	 * to turn on NIS processing.
	 */

	spwd.sp_namp = fields[0];
#ifdef	USE_NIS
	if (IS_NISCHAR (fields[0][0]))
		nis_used = 1;
#endif
	spwd.sp_pwdp = fields[1];

	/*
	 * Get the last changed date.  For all of the integer fields,
	 * we check for proper format.  It is an error to have an
	 * incorrectly formatted number, unless we are using NIS.
	 */

	if ((spwd.sp_lstchg = strtol (fields[2], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_lstchg = -1;
#else
		return 0;
#endif
	} else if (fields[2][0] == '\0')
		spwd.sp_lstchg = -1;

	/*
	 * Get the minimum period between password changes.
	 */

	if ((spwd.sp_min = strtol (fields[3], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_min = -1;
#else
		return 0;
#endif
	} else if (fields[3][0] == '\0')
		spwd.sp_min = -1;

	/*
	 * Get the maximum number of days a password is valid.
	 */

	if ((spwd.sp_max = strtol (fields[4], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_max = -1;
#else
		return 0;
#endif
	} else if (fields[4][0] == '\0')
		spwd.sp_max = -1;

	/*
	 * If there are only OFIELDS fields (this is a SVR3.2 /etc/shadow
	 * formatted file), initialize the other field members to -1.
	 */

	if (i == OFIELDS) {
		spwd.sp_warn = spwd.sp_inact = spwd.sp_expire =
		    spwd.sp_flag = -1;

		return &spwd;
	}

	/*
	 * Get the number of days of password expiry warning.
	 */

	if ((spwd.sp_warn = strtol (fields[5], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_warn = -1;
#else
		return 0;
#endif
	} else if (fields[5][0] == '\0')
		spwd.sp_warn = -1;

	/*
	 * Get the number of days of inactivity before an account is
	 * disabled.
	 */

	if ((spwd.sp_inact = strtol (fields[6], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_inact = -1;
#else
		return 0;
#endif
	} else if (fields[6][0] == '\0')
		spwd.sp_inact = -1;

	/*
	 * Get the number of days after the epoch before the account is
	 * set to expire.
	 */

	if ((spwd.sp_expire = strtol (fields[7], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_expire = -1;
#else
		return 0;
#endif
	} else if (fields[7][0] == '\0')
		spwd.sp_expire = -1;

	/*
	 * This field is reserved for future use.  But it isn't supposed
	 * to have anything other than a valid integer in it.
	 */

	if ((spwd.sp_flag = strtol (fields[8], &cpp, 10)) == 0 && *cpp) {
#ifdef	USE_NIS
		if (!nis_used)
			return 0;
		else
			spwd.sp_flag = -1;
#else
		return 0;
#endif
	} else if (fields[8][0] == '\0')
		spwd.sp_flag = -1;

	return (&spwd);
}

/*
 * fgetspent - get an entry from a /etc/shadow formatted stream
 */

struct spwd *fgetspent (FILE * fp)
{
	char buf[BUFSIZ];
	char *cp;

	if (!fp)
		return (0);

#ifdef	USE_NIS
	while (fgets (buf, sizeof buf, fp) != (char *) 0)
#else
	if (fgets (buf, sizeof buf, fp) != (char *) 0)
#endif
	{
		if ((cp = strchr (buf, '\n')))
			*cp = '\0';
#ifdef	USE_NIS
		if (nis_ignore && IS_NISCHAR (buf[0]))
			continue;
#endif
		return my_sgetspent (buf);
	}
	return 0;
}

/*
 * getspent - get a (struct spwd *) from the current shadow file
 */

struct spwd *getspent (void)
{
#ifdef	USE_NIS
	int nis_1_user = 0;
	struct spwd *val;
	char buf[BUFSIZ];
#endif
	if (!shadow)
		setspent ();

#ifdef	USE_NIS
      again:
	/*
	 * See if we are reading from the local file.
	 */

	if (nis_state == native || nis_state == native2) {

		/*
		 * Get the next entry from the shadow file.  Return NULL
		 * right away if there is none.
		 */

		if (!(val = fgetspent (shadow)))
			return 0;

		/*
		 * If this entry began with a NIS escape character, we have
		 * to see if this is just a single user, or if the entire
		 * map is being asked for.
		 */

		if (IS_NISCHAR (val->sp_namp[0])) {
			if (val->sp_namp[1])
				nis_1_user = 1;
			else
				nis_state = start;
		}

		/*
		 * If this isn't a NIS user and this isn't an escape to go
		 * use a NIS map, it must be a regular local user.
		 */

		if (nis_1_user == 0 && nis_state != start)
			return val;

		/*
		 * If this is an escape to use an NIS map, switch over to
		 * that bunch of code.
		 */

		if (nis_state == start)
			goto again;

		/*
		 * NEEDSWORK.  Here we substitute pieces-parts of this entry.
		 */

		return 0;
	} else {
		if (nis_bound == 0) {
			if (bind_nis ()) {
				nis_state = native2;
				goto again;
			}
		}
		if (nis_state == start) {
			if (yp_first (nis_domain, "shadow.bynam", &nis_key,
				      &nis_keylen, &nis_val, &nis_vallen)) {
				nis_state = native2;
				goto again;
			}
			nis_state = middle;
		} else if (nis_state == middle) {
			if (yp_next (nis_domain, "shadow.bynam", nis_key,
				     nis_keylen, &nis_key, &nis_keylen,
				     &nis_val, &nis_vallen)) {
				nis_state = native2;
				goto again;
			}
		}
		return my_sgetspent (nis_val);
	}
#else
	return (fgetspent (shadow));
#endif
}

/*
 * getspnam - get a shadow entry by name
 */

struct spwd *getspnam (const char *name)
{
	struct spwd *sp;

#ifdef	USE_NIS
	char buf[BUFSIZ];
	static char save_name[16];
	int nis_disabled = 0;
#endif

	setspent ();

#ifdef	USE_NIS
	/*
	 * Search the shadow.byname map for this user.
	 */

	if (!nis_ignore && !nis_bound)
		bind_nis ();

	if (!nis_ignore && nis_bound) {
		char *cp;

		if (yp_match (nis_domain, "shadow.byname", name,
			      strlen (name), &nis_val, &nis_vallen) == 0) {

			if (cp = strchr (nis_val, '\n'))
				*cp = '\0';

			nis_state = middle;
			if ((sp = my_sgetspent (nis_val))) {
				strcpy (save_name, sp->sp_namp);
				nis_key = save_name;
				nis_keylen = strlen (save_name);
			}
			endspent ();
			return sp;
		} else
			nis_state = native2;
	}
#endif
#ifdef	USE_NIS
	/*
	 * NEEDSWORK -- this is a mess, and it is the same mess in the
	 * other three files.  I can't just blindly turn off NIS because
	 * this might be the first pass through the local files.  In
	 * that case, I never discover that NIS is present.
	 */

	if (nis_used) {
		nis_ignore++;
		nis_disabled++;
	}
#endif
	while ((sp = getspent ()) != (struct spwd *) 0) {
		if (strcmp (name, sp->sp_namp) == 0)
			break;
	}
#ifdef	USE_NIS
	if (nis_disabled)
		nis_ignore--;
#endif
	endspent ();
	return (sp);
}
#else
extern int errno;		/* warning: ANSI C forbids an empty source file */
#endif
