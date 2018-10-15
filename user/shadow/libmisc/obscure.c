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

#ifndef USE_PAM

#ident "$Id: obscure.c,v 1.14 2005/08/31 17:24:57 kloczek Exp $"


/*
 * This version of obscure.c contains modifications to support "cracklib"
 * by Alec Muffet (alec.muffett@uk.sun.com).  You must obtain the Cracklib
 * library source code for this function to operate.
 */
#include <ctype.h>
#include <stdio.h>
#include "prototypes.h"
#include "defines.h"
#include "getdef.h"
/*
 * can't be a palindrome - like `R A D A R' or `M A D A M'
 */
 /*ARGSUSED*/ static int palindrome (const char *old, const char *new)
{
	int i, j;

	i = strlen (new);

	for (j = 0; j < i; j++)
		if (new[i - j - 1] != new[j])
			return 0;

	return 1;
}

/*
 * more than half of the characters are different ones.
 */

 /*ARGSUSED*/ static int similar (const char *old, const char *new)
{
	int i, j;

	/*
	 * XXX - sometimes this fails when changing from a simple password
	 * to a really long one (MD5).  For now, I just return success if
	 * the new password is long enough.  Please feel free to suggest
	 * something better...  --marekm
	 */
	if (strlen (new) >= 8)
		return 0;

	for (i = j = 0; new[i] && old[i]; i++)
		if (strchr (new, old[i]))
			j++;

	if (i >= j * 2)
		return 0;

	return 1;
}

/*
 * a nice mix of characters.
 */

 /*ARGSUSED*/ static int simple (const char *old, const char *new)
{
	int digits = 0;
	int uppers = 0;
	int lowers = 0;
	int others = 0;
	int size;
	int i;

	for (i = 0; new[i]; i++) {
		if (isdigit (new[i]))
			digits++;
		else if (isupper (new[i]))
			uppers++;
		else if (islower (new[i]))
			lowers++;
		else
			others++;
	}

	/*
	 * The scam is this - a password of only one character type
	 * must be 8 letters long.  Two types, 7, and so on.
	 */

	size = 9;
	if (digits)
		size--;
	if (uppers)
		size--;
	if (lowers)
		size--;
	if (others)
		size--;

	if (size <= i)
		return 0;

	return 1;
}

static char *str_lower (char *string)
{
	char *cp;

	for (cp = string; *cp; cp++)
		*cp = tolower (*cp);
	return string;
}

static const char *password_check (const char *old, const char *new,
				   const struct passwd *pwdp)
{
	const char *msg = NULL;
	char *oldmono, *newmono, *wrapped;

#ifdef HAVE_LIBCRACK
	char *dictpath;

#ifdef HAVE_LIBCRACK_PW
	char *FascistCheckPw ();
#else
	char *FascistCheck ();
#endif
#endif

	if (strcmp (new, old) == 0)
		return _("no change");

	newmono = str_lower (xstrdup (new));
	oldmono = str_lower (xstrdup (old));
	wrapped = xmalloc (strlen (oldmono) * 2 + 1);
	strcpy (wrapped, oldmono);
	strcat (wrapped, oldmono);

	if (palindrome (oldmono, newmono))
		msg = _("a palindrome");

	if (!msg && strcmp (oldmono, newmono) == 0)
		msg = _("case changes only");

	if (!msg && similar (oldmono, newmono))
		msg = _("too similar");

	if (!msg && simple (old, new))
		msg = _("too simple");

	if (!msg && strstr (wrapped, newmono))
		msg = _("rotated");

#ifdef HAVE_LIBCRACK
	/*
	 * Invoke Alec Muffett's cracklib routines.
	 */

	if (!msg && (dictpath = getdef_str ("CRACKLIB_DICTPATH")))
#ifdef HAVE_LIBCRACK_PW
		msg = FascistCheckPw (new, dictpath, pwdp);
#else
		msg = FascistCheck (new, dictpath);
#endif
#endif
	strzero (newmono);
	strzero (oldmono);
	strzero (wrapped);
	free (newmono);
	free (oldmono);
	free (wrapped);

	return msg;
}

 /*ARGSUSED*/
    static const char *obscure_msg (const char *old, const char *new,
				    const struct passwd *pwdp)
{
	int maxlen, oldlen, newlen;
	char *new1, *old1;
	const char *msg;

	oldlen = strlen (old);
	newlen = strlen (new);

	if (newlen < getdef_num ("PASS_MIN_LEN", 0))
		return _("too short");

	/*
	 * Remaining checks are optional.
	 */
	if (!getdef_bool ("OBSCURE_CHECKS_ENAB"))
		return NULL;

	msg = password_check (old, new, pwdp);
	if (msg)
		return msg;

	/* The traditional crypt() truncates passwords to 8 chars.  It is
	   possible to circumvent the above checks by choosing an easy
	   8-char password and adding some random characters to it...
	   Example: "password$%^&*123".  So check it again, this time
	   truncated to the maximum length.  Idea from npasswd.  --marekm */

	if (getdef_bool ("MD5_CRYPT_ENAB"))
		return NULL;	/* unlimited password length */

	maxlen = getdef_num ("PASS_MAX_LEN", 8);
	if (oldlen <= maxlen && newlen <= maxlen)
		return NULL;

	new1 = xstrdup (new);
	old1 = xstrdup (old);
	if (newlen > maxlen)
		new1[maxlen] = '\0';
	if (oldlen > maxlen)
		old1[maxlen] = '\0';

	msg = password_check (old1, new1, pwdp);

	memzero (new1, newlen);
	memzero (old1, oldlen);
	free (new1);
	free (old1);

	return msg;
}

/*
 * Obscure - see if password is obscure enough.
 *
 *	The programmer is encouraged to add as much complexity to this
 *	routine as desired.  Included are some of my favorite ways to
 *	check passwords.
 */

int obscure (const char *old, const char *new, const struct passwd *pwdp)
{
	const char *msg = obscure_msg (old, new, pwdp);

	if (msg) {
		printf (_("Bad password: %s.  "), msg);
		return 0;
	}
	return 1;
}

#endif				/* !USE_PAM */
