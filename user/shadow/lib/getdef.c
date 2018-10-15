/*
 * Copyright 1991 - 1994, Julianne Frances Haugh and Chip Rosenthal
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

#ident "$Id: getdef.c,v 1.37 2005/12/02 22:16:11 kloczek Exp $"

#include "prototypes.h"
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "getdef.h"
/*
 * A configuration item definition.
 */
struct itemdef {
	const char *name;	/* name of the item                     */
	char *value;		/* value given, or NULL if no value     */
};

#define NUMDEFS	(sizeof(def_table)/sizeof(def_table[0]))
static struct itemdef def_table[] = {
	{"CHFN_RESTRICT", NULL},
	{"CONSOLE_GROUPS", NULL},
	{"CONSOLE", NULL},
	{"CREATE_HOME", NULL},
	{"DEFAULT_HOME", NULL},
	{"ENV_PATH", NULL},
	{"ENV_SUPATH", NULL},
	{"ERASECHAR", NULL},
	{"FAIL_DELAY", NULL},
	{"FAKE_SHELL", NULL},
	{"GETPASS_ASTERISKS", NULL},
	{"GID_MAX", NULL},
	{"GID_MIN", NULL},
	{"HUSHLOGIN_FILE", NULL},
	{"KILLCHAR", NULL},
	{"LOGIN_RETRIES", NULL},
	{"LOGIN_TIMEOUT", NULL},
	{"LOG_OK_LOGINS", NULL},
	{"LOG_UNKFAIL_ENAB", NULL},
	{"MAIL_DIR", NULL},
	{"MAIL_FILE", NULL},
	{"MD5_CRYPT_ENAB", NULL},
	{"PASS_MAX_DAYS", NULL},
	{"PASS_MIN_DAYS", NULL},
	{"PASS_WARN_AGE", NULL},
	{"SULOG_FILE", NULL},
	{"SU_NAME", NULL},
	{"TTYGROUP", NULL},
	{"TTYPERM", NULL},
	{"TTYTYPE_FILE", NULL},
	{"UID_MAX", NULL},
	{"UID_MIN", NULL},
	{"UMASK", NULL},
	{"USERDEL_CMD", NULL},
	{"USERGROUPS_ENAB", NULL},
#ifndef USE_PAM
	{"CHFN_AUTH", NULL},
	{"CHSH_AUTH", NULL},
	{"CRACKLIB_DICTPATH", NULL},
	{"ENV_HZ", NULL},
	{"ENV_TZ", NULL},
	{"ENVIRON_FILE", NULL},
	{"FAILLOG_ENAB", NULL},
	{"FTMP_FILE", NULL},
	{"ISSUE_FILE", NULL},
	{"LASTLOG_ENAB", NULL},
	{"LOGIN_STRING", NULL},
	{"MAIL_CHECK_ENAB", NULL},
	{"MOTD_FILE", NULL},
	{"NOLOGINS_FILE", NULL},
	{"OBSCURE_CHECKS_ENAB", NULL},
	{"PASS_ALWAYS_WARN", NULL},
	{"PASS_CHANGE_TRIES", NULL},
	{"PASS_MAX_LEN", NULL},
	{"PASS_MIN_LEN", NULL},
	{"PORTTIME_CHECKS_ENAB", NULL},
	{"SU_WHEEL_ONLY", NULL},
	{"QUOTAS_ENAB", NULL},
	{"ULIMIT", NULL},
#endif
#ifdef USE_SYSLOG
	{"SYSLOG_SG_ENAB", NULL},
	{"SYSLOG_SU_ENAB", NULL},
#endif
#ifdef USE_PAM
	{"VIRTUAL_USER", NULL},
#endif
	{NULL, NULL}
};

#ifndef LOGINDEFS
#define LOGINDEFS "/etc/login.defs"
#endif

static char def_fname[] = LOGINDEFS;	/* login config defs file       */
static int def_loaded = 0;	/* are defs already loaded?     */

/* local function prototypes */
static struct itemdef *def_find (const char *);
static void def_load (void);


/*
 * getdef_str - get string value from table of definitions.
 *
 * Return point to static data for specified item, or NULL if item is not
 * defined.  First time invoked, will load definitions from the file.
 */

char *getdef_str (const char *item)
{
	struct itemdef *d;

	if (!def_loaded)
		def_load ();

	return ((d = def_find (item)) == NULL ? (char *) NULL : d->value);
}


/*
 * getdef_bool - get boolean value from table of definitions.
 *
 * Return TRUE if specified item is defined as "yes", else FALSE.
 */

int getdef_bool (const char *item)
{
	struct itemdef *d;

	if (!def_loaded)
		def_load ();

	if ((d = def_find (item)) == NULL || d->value == NULL)
		return 0;

	return (strcasecmp (d->value, "yes") == 0);
}


/*
 * getdef_num - get numerical value from table of definitions
 *
 * Returns numeric value of specified item, else the "dflt" value if
 * the item is not defined.  Octal (leading "0") and hex (leading "0x")
 * values are handled.
 */

int getdef_num (const char *item, int dflt)
{
	struct itemdef *d;

	if (!def_loaded)
		def_load ();

	if ((d = def_find (item)) == NULL || d->value == NULL)
		return dflt;

	return (int) strtol (d->value, (char **) NULL, 0);
}


/*
 * getdef_unum - get unsigned numerical value from table of definitions
 *
 * Returns numeric value of specified item, else the "dflt" value if
 * the item is not defined.  Octal (leading "0") and hex (leading "0x")
 * values are handled.
 */

unsigned int getdef_unum (const char *item, unsigned int dflt)
{
	struct itemdef *d;

	if (!def_loaded)
		def_load ();

	if ((d = def_find (item)) == NULL || d->value == NULL)
		return dflt;

	return (unsigned int) strtoul (d->value, (char **) NULL, 0);
}


/*
 * getdef_long - get long integer value from table of definitions
 *
 * Returns numeric value of specified item, else the "dflt" value if
 * the item is not defined.  Octal (leading "0") and hex (leading "0x")
 * values are handled.
 */

long getdef_long (const char *item, long dflt)
{
	struct itemdef *d;

	if (!def_loaded)
		def_load ();

	if ((d = def_find (item)) == NULL || d->value == NULL)
		return dflt;

	return strtol (d->value, (char **) NULL, 0);
}


/*
 * putdef_str - override the value read from /etc/login.defs
 * (also used when loading the initial defaults)
 */

int putdef_str (const char *name, const char *value)
{
	struct itemdef *d;
	char *cp;

	if (!def_loaded)
		def_load ();

	/*
	 * Locate the slot to save the value.  If this parameter
	 * is unknown then "def_find" will print an err message.
	 */
	if ((d = def_find (name)) == NULL)
		return -1;

	/*
	 * Save off the value.
	 */
	if ((cp = strdup (value)) == NULL) {
		fprintf (stderr,
			 _("Could not allocate space for config info.\n"));
		SYSLOG ((LOG_ERR, "could not allocate space for config info"));
		return -1;
	}

	if (d->value)
		free (d->value);

	d->value = cp;
	return 0;
}


/*
 * def_find - locate named item in table
 *
 * Search through a table of configurable items to locate the
 * specified configuration option.
 */

static struct itemdef *def_find (const char *name)
{
	int n;
	struct itemdef *ptr;


	/*
	 * Search into the table.
	 */

	for (ptr = def_table; ptr->name; ptr++) {
		if (!(n = strcmp (ptr->name, name)))
			return ptr;
	}

	/*
	 * Item was never found.
	 */

	fprintf (stderr,
		 _
		 ("configuration error - unknown item '%s' (notify administrator)\n"),
		 name);
	SYSLOG ((LOG_CRIT, "unknown configuration item `%s'", name));
	return (struct itemdef *) NULL;
}

/*
 * def_load - load configuration table
 *
 * Loads the user-configured options from the default configuration file
 */

static void def_load (void)
{
	int i;
	FILE *fp;
	char buf[1024], *name, *value, *s;

	/*
	 * Open the configuration definitions file.
	 */
	if ((fp = fopen (def_fname, "r")) == NULL) {
#if 0
		SYSLOG ((LOG_CRIT, "cannot open login definitions %s [%m]",
			 def_fname));
		exit (1);
#else
		SYSLOG ((LOG_NOTICE, "cannot open login definitions %s [%m]",
			 def_fname));
		++def_loaded;
		return;
#endif
	}

	/*
	 * Set the initialized flag.
	 * (do it early to prevent recursion in putdef_str())
	 */
	++def_loaded;

	/*
	 * Go through all of the lines in the file.
	 */
	while (fgets (buf, sizeof (buf), fp) != NULL) {

		/*
		 * Trim trailing whitespace.
		 */
		for (i = strlen (buf) - 1; i >= 0; --i) {
			if (!isspace (buf[i]))
				break;
		}
		buf[++i] = '\0';

		/*
		 * Break the line into two fields.
		 */
		name = buf + strspn (buf, " \t");	/* first nonwhite */
		if (*name == '\0' || *name == '#')
			continue;	/* comment or empty */

		s = name + strcspn (name, " \t");	/* end of field */
		if (*s == '\0')
			continue;	/* only 1 field?? */

		*s++ = '\0';
		value = s + strspn (s, " \"\t");	/* next nonwhite */
		*(value + strcspn (value, "\"")) = '\0';

		/*
		 * Store the value in def_table.
		 */
		putdef_str (name, value);
	}

	if (ferror (fp)) {
		SYSLOG ((LOG_CRIT, "cannot read login definitions %s [%m]",
			 def_fname));
		exit (1);
	}

	(void) fclose (fp);
}


#ifdef CKDEFS
int main (int argc, char **argv)
{
	int i;
	char *cp;
	struct itemdef *d;

	def_load ();

	for (i = 0; i < NUMDEFS; ++i) {
		if ((d = def_find (def_table[i].name)) == NULL)
			printf ("error - lookup '%s' failed\n",
				def_table[i].name);
		else
			printf ("%4d %-24s %s\n", i + 1, d->name, d->value);
	}
	for (i = 1; i < argc; i++) {
		if ((cp = getdef_str (argv[1])) != NULL)
			printf ("%s `%s'\n", argv[1], cp);
		else
			printf ("%s not found\n", argv[1]);
	}
	exit (0);
}
#endif
