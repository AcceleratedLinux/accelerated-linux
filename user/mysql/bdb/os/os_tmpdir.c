/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char revid[] = "$Id: os_tmpdir.c,v 11.16 2001/01/08 20:42:06 bostic Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <stdlib.h>
#endif

#include "db_int.h"

#ifdef macintosh
#include <TFileSpec.h>
#endif

/*
 * __os_tmpdir --
 *	Set the temporary directory path.
 *
 * The order of items in the list structure and the order of checks in
 * the environment are documented.
 *
 * PUBLIC: int __os_tmpdir __P((DB_ENV *, u_int32_t));
 */
int
__os_tmpdir(dbenv, flags)
	DB_ENV *dbenv;
	u_int32_t flags;
{
	/*
	 * !!!
	 * Don't change this to:
	 *
	 *	static const char * const list[]
	 *
	 * because it creates a text relocation in position independent code.
	 */
	static const char * list[] = {
		"/var/tmp",
		"/usr/tmp",
		"/temp",		/* Windows. */
		"/tmp",
		"C:/temp",		/* Windows. */
		"C:/tmp",		/* Windows. */
		NULL
	};
	const char * const *lp, *p;

	/* Use the environment if it's permitted and initialized. */
	if (LF_ISSET(DB_USE_ENVIRON) ||
	    (LF_ISSET(DB_USE_ENVIRON_ROOT) && __os_isroot())) {
		if ((p = getenv("TMPDIR")) != NULL && p[0] == '\0') {
			__db_err(dbenv, "illegal TMPDIR environment variable");
			return (EINVAL);
		}
		/* Windows */
		if (p == NULL && (p = getenv("TEMP")) != NULL && p[0] == '\0') {
			__db_err(dbenv, "illegal TEMP environment variable");
			return (EINVAL);
		}
		/* Windows */
		if (p == NULL && (p = getenv("TMP")) != NULL && p[0] == '\0') {
			__db_err(dbenv, "illegal TMP environment variable");
			return (EINVAL);
		}
		/* Macintosh */
		if (p == NULL &&
		    (p = getenv("TempFolder")) != NULL && p[0] == '\0') {
			__db_err(dbenv,
			    "illegal TempFolder environment variable");
			return (EINVAL);
		}
		if (p != NULL)
			return (__os_strdup(dbenv, p, &dbenv->db_tmp_dir));
	}

#ifdef macintosh
	/* Get the path to the temporary folder. */
	{FSSpec spec;

		if (!Special2FSSpec(kTemporaryFolderType,
		    kOnSystemDisk, 0, &spec))
			return (__os_strdup(dbenv,
			    FSp2FullPath(&spec), &dbenv->db_tmp_dir));
	}
#endif
#ifdef DB_WIN32
	/* Get the path to the temporary directory. */
	{int isdir, len;
	char *eos, temp[MAXPATHLEN + 1];

		if ((len = GetTempPath(sizeof(temp) - 1, temp)) > 2) {
			eos = &temp[len];
			*eos-- = '\0';
			if (*eos == '\\' || *eos == '/')
				*eos = '\0';
			if (__os_exists(temp, &isdir) == 0 && isdir != 0)
				return (__os_strdup(dbenv,
				    temp, &dbenv->db_tmp_dir));
		}
	}
#endif

	/* Step through the static list looking for a possibility. */
	for (lp = list; *lp != NULL; ++lp)
		if (__os_exists(*lp, NULL) == 0)
			return (__os_strdup(dbenv, *lp, &dbenv->db_tmp_dir));
	return (0);
}
