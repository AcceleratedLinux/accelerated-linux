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

/*
 * Separated from setup.c.  --marekm
 */

#include <config.h>

#ident "$Id: setupenv.c,v 1.20 2005/09/30 14:29:11 kloczek Exp $"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include "prototypes.h"
#include "defines.h"
#include <pwd.h>
#include "getdef.h"
static void
addenv_path (const char *varname, const char *dirname, const char *filename)
{
	char *buf;

	buf = xmalloc (strlen (dirname) + strlen (filename) + 2);
	sprintf (buf, "%s/%s", dirname, filename);
	addenv (varname, buf);
	free (buf);
}


#ifndef USE_PAM
static void read_env_file (const char *filename)
{
	FILE *fp;
	char buf[1024];
	char *cp, *name, *val;

	fp = fopen (filename, "r");
	if (!fp)
		return;
	while (fgets (buf, sizeof buf, fp) == buf) {
		cp = strrchr (buf, '\n');
		if (!cp)
			break;
		*cp = '\0';

		cp = buf;
		/* ignore whitespace and comments */
		while (*cp && isspace (*cp))
			cp++;
		if (*cp == '\0' || *cp == '#')
			continue;
		/*
		 * ignore lines which don't follow the name=value format
		 * (for example, the "export NAME" shell commands)
		 */
		name = cp;
		while (*cp && !isspace (*cp) && *cp != '=')
			cp++;
		if (*cp != '=')
			continue;
		/* NUL-terminate the name */
		*cp++ = '\0';
		val = cp;
#if 0				/* XXX untested, and needs rewrite with fewer goto's :-) */
/*
 (state, char_type) -> (state, action)

 state: unquoted, single_quoted, double_quoted, escaped, double_quoted_escaped
 char_type: normal, white, backslash, single, double
 action: remove_curr, remove_curr_skip_next, remove_prev, finish XXX
*/
	      no_quote:
		if (*cp == '\\') {
			/* remove the backslash */
			remove_char (cp);
			/* skip over the next character */
			if (*cp)
				cp++;
			goto no_quote;
		} else if (*cp == '\'') {
			/* remove the quote */
			remove_char (cp);
			/* now within single quotes */
			goto s_quote;
		} else if (*cp == '"') {
			/* remove the quote */
			remove_char (cp);
			/* now within double quotes */
			goto d_quote;
		} else if (*cp == '\0') {
			/* end of string */
			goto finished;
		} else if (isspace (*cp)) {
			/* unescaped whitespace - end of string */
			*cp = '\0';
			goto finished;
		} else {
			cp++;
			goto no_quote;
		}
	      s_quote:
		if (*cp == '\'') {
			/* remove the quote */
			remove_char (cp);
			/* unquoted again */
			goto no_quote;
		} else if (*cp == '\0') {
			/* end of string */
			goto finished;
		} else {
			/* preserve everything within single quotes */
			cp++;
			goto s_quote;
		}
	      d_quote:
		if (*cp == '\"') {
			/* remove the quote */
			remove_char (cp);
			/* unquoted again */
			goto no_quote;
		} else if (*cp == '\\') {
			cp++;
			/* if backslash followed by double quote, remove backslash
			   else skip over the backslash and following char */
			if (*cp == '"')
				remove_char (cp - 1);
			else if (*cp)
				cp++;
			goto d_quote;
		}
		eise if (*cp == '\0') {
			/* end of string */
			goto finished;
		} else {
			/* preserve everything within double quotes */
			goto d_quote;
		}
	      finished:
#endif				/* 0 */
		/*
		 * XXX - should handle quotes, backslash escapes, etc.
		 * like the shell does.
		 */
		addenv (name, val);
	}
	fclose (fp);
}
#endif				/* USE_PAM */


/*
 *	change to the user's home directory
 *	set the HOME, SHELL, MAIL, PATH, and LOGNAME or USER environmental
 *	variables.
 */

void setup_env (struct passwd *info)
{
#ifndef USE_PAM
	char *envf;
#endif
	char *cp;

	/*
	 * Change the current working directory to be the home directory
	 * of the user.  It is a fatal error for this process to be unable
	 * to change to that directory.  There is no "default" home
	 * directory.
	 *
	 * We no longer do it as root - should work better on NFS-mounted
	 * home directories.  Some systems default to HOME=/, so we make
	 * this a configurable option.  --marekm
	 */

	if (chdir (info->pw_dir) == -1) {
		static char temp_pw_dir[] = "/";

		if (!getdef_bool ("DEFAULT_HOME") || chdir ("/") == -1) {
			fprintf (stderr, _("Unable to cd to \"%s\"\n"),
				 info->pw_dir);
			SYSLOG ((LOG_WARN,
				 "unable to cd to `%s' for user `%s'\n",
				 info->pw_dir, info->pw_name));
			closelog ();
			exit (1);
		}
		puts (_("No directory, logging in with HOME=/"));
		info->pw_dir = temp_pw_dir;
	}

	/*
	 * Create the HOME environmental variable and export it.
	 */

	addenv ("HOME", info->pw_dir);

	/*
	 * Create the SHELL environmental variable and export it.
	 */

	if (info->pw_shell == (char *) 0 || !*info->pw_shell) {
		static char temp_pw_shell[] = "/bin/sh";

		info->pw_shell = temp_pw_shell;
	}

	addenv ("SHELL", info->pw_shell);

	/*
	 * Export the user name.  For BSD derived systems, it's "USER", for
	 * all others it's "LOGNAME".  We set both of them.
	 */

	addenv ("USER", info->pw_name);
	addenv ("LOGNAME", info->pw_name);

#ifndef USE_PAM
	/*
	 * Create the PATH environmental variable and export it.
	 */

	cp = getdef_str ((info->pw_uid == 0) ? "ENV_SUPATH" : "ENV_PATH");

	if (!cp) {
		/* not specified, use a minimal default */
		addenv ("PATH=/bin:/usr/bin", NULL);
	} else if (strchr (cp, '=')) {
		/* specified as name=value (PATH=...) */
		addenv (cp, NULL);
	} else {
		/* only value specified without "PATH=" */
		addenv ("PATH", cp);
	}

	/*
	 * Create the MAIL environmental variable and export it.  login.defs
	 * knows the prefix.
	 */

	if ((cp = getdef_str ("MAIL_DIR")))
		addenv_path ("MAIL", cp, info->pw_name);
	else if ((cp = getdef_str ("MAIL_FILE")))
		addenv_path ("MAIL", info->pw_dir, cp);
	else {
#if defined(MAIL_SPOOL_FILE)
		addenv_path ("MAIL", info->pw_dir, MAIL_SPOOL_FILE);
#elif defined(MAIL_SPOOL_DIR)
		addenv_path ("MAIL", MAIL_SPOOL_DIR, info->pw_name);
#endif
	}

	/*
	 * Read environment from optional config file.  --marekm
	 */
	if ((envf = getdef_str ("ENVIRON_FILE")))
		read_env_file (envf);
#endif				/* !USE_PAM */
}
