/* 
   Unix SMB/CIFS implementation.
   filename handling routines
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Jeremy Allison 1999-2004
   Copyright (C) Ying Chen 2000
   
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

/*
 * New hash table stat cache code added by Ying Chen.
 */

#include "includes.h"

static BOOL scan_directory(connection_struct *conn, const char *path, char *name,size_t maxlength);

/****************************************************************************
 Check if two filenames are equal.
 This needs to be careful about whether we are case sensitive.
****************************************************************************/

static BOOL fname_equal(const char *name1, const char *name2, BOOL case_sensitive)
{
	/* Normal filename handling */
	if (case_sensitive)
		return(strcmp(name1,name2) == 0);

	return(strequal(name1,name2));
}

/****************************************************************************
 Mangle the 2nd name and check if it is then equal to the first name.
****************************************************************************/

static BOOL mangled_equal(const char *name1, const char *name2,
			  const struct share_params *p)
{
	pstring tmpname;
	
	pstrcpy(tmpname, name2);
	mangle_map(tmpname, True, False, p);
	return strequal(name1, tmpname);
}

/****************************************************************************
 Cope with the differing wildcard and non-wildcard error cases.
****************************************************************************/

static NTSTATUS determine_path_error(const char *name, BOOL allow_wcard_last_component)
{
	const char *p;

	if (!allow_wcard_last_component) {
		/* Error code within a pathname. */
		return NT_STATUS_OBJECT_PATH_NOT_FOUND;
	}

	/* We're terminating here so we
	 * can be a little slower and get
	 * the error code right. Windows
	 * treats the last part of the pathname
	 * separately I think, so if the last
	 * component is a wildcard then we treat
	 * this ./ as "end of component" */

	p = strchr(name, '/');

	if (!p && (ms_has_wild(name) || ISDOT(name))) {
		/* Error code at the end of a pathname. */
		return NT_STATUS_OBJECT_NAME_INVALID;
	} else {
		/* Error code within a pathname. */
		return NT_STATUS_OBJECT_PATH_NOT_FOUND;
	}
}
	
/****************************************************************************
This routine is called to convert names from the dos namespace to unix
namespace. It needs to handle any case conversions, mangling, format
changes etc.

We assume that we have already done a chdir() to the right "root" directory
for this service.

The function will return an NTSTATUS error if some part of the name except for the last
part cannot be resolved, else NT_STATUS_OK.

Note NT_STATUS_OK doesn't mean the name exists or is valid, just that we didn't
get any fatal errors that should immediately terminate the calling
SMB processing whilst resolving.

If the saved_last_component != 0, then the unmodified last component
of the pathname is returned there. This is used in an exceptional
case in reply_mv (so far). If saved_last_component == 0 then nothing
is returned there.

If last_component_wcard is true then a MS wildcard was detected and
should be allowed in the last component of the path only.

On exit from unix_convert, if *pst was not null, then the file stat
struct will be returned if the file exists and was found, if not this
stat struct will be filled with zeros (and this can be detected by checking
for nlinks = 0, which can never be true for any file).
****************************************************************************/

NTSTATUS unix_convert(connection_struct *conn,
			pstring name,
			BOOL allow_wcard_last_component,
			char *saved_last_component, 
			SMB_STRUCT_STAT *pst)
{
	SMB_STRUCT_STAT st;
	char *start, *end;
	pstring dirpath;
	pstring orig_path;
	BOOL component_was_mangled = False;
	BOOL name_has_wildcard = False;

	SET_STAT_INVALID(*pst);

	*dirpath = 0;

	if(saved_last_component) {
		*saved_last_component = 0;
	}

	if (conn->printer) {
		/* we don't ever use the filenames on a printer share as a
			filename - so don't convert them */
		return NT_STATUS_OK;
	}

	DEBUG(5, ("unix_convert called on file \"%s\"\n", name));

	/* 
	 * Conversion to basic unix format is already done in check_path_syntax().
	 */

	/* 
	 * Names must be relative to the root of the service - any leading /.
	 * and trailing /'s should have been trimmed by check_path_syntax().
	 */

#ifdef DEVELOPER
	SMB_ASSERT(*name != '/');
#endif

	/*
	 * If we trimmed down to a single '\0' character
	 * then we should use the "." directory to avoid
	 * searching the cache, but not if we are in a
	 * printing share.
	 * As we know this is valid we can return true here.
	 */

	if (!*name) {
		name[0] = '.';
		name[1] = '\0';
		if (SMB_VFS_STAT(conn,name,&st) == 0) {
			*pst = st;
		}
		DEBUG(5,("conversion finished \"\" -> %s\n",name));
		return NT_STATUS_OK;
	}

	if (name[0] == '.' && (name[1] == '/' || name[1] == '\0')) {
		/* Start of pathname can't be "." only. */
		if (name[1] == '\0' || name[2] == '\0') {
			return NT_STATUS_OBJECT_NAME_INVALID;
		} else {
			return determine_path_error(&name[2], allow_wcard_last_component);
		}
	}

	/*
	 * Ensure saved_last_component is valid even if file exists.
	 */

	if(saved_last_component) {
		end = strrchr_m(name, '/');
		if (end) {
			pstrcpy(saved_last_component, end + 1);
		} else {
			pstrcpy(saved_last_component, name);
		}
	}

	/*
	 * Large directory fix normalization. If we're case sensitive, and
	 * the case preserving parameters are set to "no", normalize the case of
	 * the incoming filename from the client WHETHER IT EXISTS OR NOT !
	 * This is in conflict with the current (3.0.20) man page, but is
	 * what people expect from the "large directory howto". I'll update
	 * the man page. Thanks to jht@samba.org for finding this. JRA.
	 */

	if (conn->case_sensitive && !conn->case_preserve && !conn->short_case_preserve) {
		strnorm(name, lp_defaultcase(SNUM(conn)));
	}
	
	start = name;
	pstrcpy(orig_path, name);

	if(!conn->case_sensitive && stat_cache_lookup(conn, name, dirpath, &start, &st)) {
		*pst = st;
		return NT_STATUS_OK;
	}

	/* 
	 * stat the name - if it exists then we are all done!
	 */

	if (SMB_VFS_STAT(conn,name,&st) == 0) {
		/* Ensure we catch all names with in "/."
		   this is disallowed under Windows. */
		const char *p = strstr(name, "/."); /* mb safe. */
		if (p) {
			if (p[2] == '/') {
				/* Error code within a pathname. */
				return NT_STATUS_OBJECT_PATH_NOT_FOUND;
			} else if (p[2] == '\0') {
				/* Error code at the end of a pathname. */
				return NT_STATUS_OBJECT_NAME_INVALID;
			}
		}
		stat_cache_add(orig_path, name, conn->case_sensitive);
		DEBUG(5,("conversion finished %s -> %s\n",orig_path, name));
		*pst = st;
		return NT_STATUS_OK;
	}

	DEBUG(5,("unix_convert begin: name = %s, dirpath = %s, start = %s\n", name, dirpath, start));

	/* 
	 * A special case - if we don't have any mangling chars and are case
	 * sensitive then searching won't help.
	 */

	if (conn->case_sensitive && 
			!mangle_is_mangled(name, conn->params) &&
			!*lp_mangled_map(conn->params)) {
		return NT_STATUS_OK;
	}

	/* 
	 * is_mangled() was changed to look at an entire pathname, not 
	 * just a component. JRA.
	 */

	if (mangle_is_mangled(start, conn->params)) {
		component_was_mangled = True;
	}

	/* 
	 * Now we need to recursively match the name against the real 
	 * directory structure.
	 */

	/* 
	 * Match each part of the path name separately, trying the names
	 * as is first, then trying to scan the directory for matching names.
	 */

	for (; start ; start = (end?end+1:(char *)NULL)) {
		/* 
		 * Pinpoint the end of this section of the filename.
		 */
		end = strchr(start, '/'); /* mb safe. '/' can't be in any encoded char. */

		/* 
		 * Chop the name at this point.
		 */
		if (end) {
			*end = 0;
		}

		if (saved_last_component != 0) {
			pstrcpy(saved_last_component, end ? end + 1 : start);
		}

		/* The name cannot have a component of "." */

		if (ISDOT(start)) {
			if (!end)  {
				/* Error code at the end of a pathname. */
				return NT_STATUS_OBJECT_NAME_INVALID;
			}
			return determine_path_error(end+1, allow_wcard_last_component);
		}

		/* The name cannot have a wildcard if it's not
		   the last component. */

		name_has_wildcard = ms_has_wild(start);

		/* Wildcard not valid anywhere. */
		if (name_has_wildcard && !allow_wcard_last_component) {
			return NT_STATUS_OBJECT_NAME_INVALID;
		}

		/* Wildcards never valid within a pathname. */
		if (name_has_wildcard && end) {
			return NT_STATUS_OBJECT_NAME_INVALID;
		}

		/* 
		 * Check if the name exists up to this point.
		 */

		if (SMB_VFS_STAT(conn,name, &st) == 0) {
			/*
			 * It exists. it must either be a directory or this must be
			 * the last part of the path for it to be OK.
			 */
			if (end && !(st.st_mode & S_IFDIR)) {
				/*
				 * An intermediate part of the name isn't a directory.
				 */
				DEBUG(5,("Not a dir %s\n",start));
				*end = '/';
				/* 
				 * We need to return the fact that the intermediate
				 * name resolution failed. This is used to return an
				 * error of ERRbadpath rather than ERRbadfile. Some
				 * Windows applications depend on the difference between
				 * these two errors.
				 */
				return NT_STATUS_OBJECT_PATH_NOT_FOUND;
			}

			if (!end) {
				/*
				 * We just scanned for, and found the end of the path.
				 * We must return the valid stat struct.
				 * JRA.
				 */

				*pst = st;
			}

		} else {
			pstring rest;

			/* Stat failed - ensure we don't use it. */
			SET_STAT_INVALID(st);
			*rest = 0;

			/*
			 * Remember the rest of the pathname so it can be restored
			 * later.
			 */

			if (end) {
				pstrcpy(rest,end+1);
			}

			/* Reset errno so we can detect directory open errors. */
			errno = 0;

			/*
			 * Try to find this part of the path in the directory.
			 */

			if (name_has_wildcard || 
			    !scan_directory(conn, dirpath, start, sizeof(pstring) - 1 - (start - name))) {
				if (end) {
					/*
					 * An intermediate part of the name can't be found.
					 */
					DEBUG(5,("Intermediate not found %s\n",start));
					*end = '/';

					/* 
					 * We need to return the fact that the intermediate
					 * name resolution failed. This is used to return an
					 * error of ERRbadpath rather than ERRbadfile. Some
					 * Windows applications depend on the difference between
					 * these two errors.
					 */

					/* ENOENT and ENOTDIR both map to NT_STATUS_OBJECT_PATH_NOT_FOUND
					   in the filename walk. */

					if (errno == ENOENT || errno == ENOTDIR) {
						return NT_STATUS_OBJECT_PATH_NOT_FOUND;
					}
					return map_nt_error_from_unix(errno);
				}
	      
				/* ENOENT is the only valid error here. */
				if (errno != ENOENT) {
					/* ENOENT and ENOTDIR both map to NT_STATUS_OBJECT_PATH_NOT_FOUND
					   in the filename walk. */
					if (errno == ENOTDIR) {
						return NT_STATUS_OBJECT_PATH_NOT_FOUND;
					}
					return map_nt_error_from_unix(errno);
				}

				/*
				 * Just the last part of the name doesn't exist.
				 * We need to strupper() or strlower() it as
				 * this conversion may be used for file creation 
				 * purposes. Fix inspired by Thomas Neumann <t.neumann@iku-ag.de>.
				 */
				if (!conn->case_preserve ||
				    (mangle_is_8_3(start, False, conn->params) &&
						 !conn->short_case_preserve)) {
					strnorm(start, lp_defaultcase(SNUM(conn)));
				}

				/*
				 * check on the mangled stack to see if we can recover the 
				 * base of the filename.
				 */

				if (mangle_is_mangled(start, conn->params)) {
					mangle_check_cache( start, sizeof(pstring) - 1 - (start - name), conn->params);
				}

				DEBUG(5,("New file %s\n",start));
				return NT_STATUS_OK;
			}

			/* 
			 * Restore the rest of the string. If the string was mangled the size
			 * may have changed.
			 */
			if (end) {
				end = start + strlen(start);
				if (!safe_strcat(start, "/", sizeof(pstring) - 1 - (start - name)) ||
				    !safe_strcat(start, rest, sizeof(pstring) - 1 - (start - name))) {
					return map_nt_error_from_unix(ENAMETOOLONG);
				}
				*end = '\0';
			} else {
				/*
				 * We just scanned for, and found the end of the path.
				 * We must return a valid stat struct if it exists.
				 * JRA.
				 */

				if (SMB_VFS_STAT(conn,name, &st) == 0) {
					*pst = st;
				} else {
					SET_STAT_INVALID(st);
				}
			}
		} /* end else */

#ifdef DEVELOPER
		if (VALID_STAT(st) && get_delete_on_close_flag(st.st_dev, st.st_ino)) {
			return NT_STATUS_DELETE_PENDING;
		}
#endif

		/* 
		 * Add to the dirpath that we have resolved so far.
		 */
		if (*dirpath) {
			pstrcat(dirpath,"/");
		}

		pstrcat(dirpath,start);

		/*
		 * Don't cache a name with mangled or wildcard components
		 * as this can change the size.
		 */
		
		if(!component_was_mangled && !name_has_wildcard) {
			stat_cache_add(orig_path, dirpath, conn->case_sensitive);
		}
	
		/* 
		 * Restore the / that we wiped out earlier.
		 */
		if (end) {
			*end = '/';
		}
	}
  
	/*
	 * Don't cache a name with mangled or wildcard components
	 * as this can change the size.
	 */

	if(!component_was_mangled && !name_has_wildcard) {
		stat_cache_add(orig_path, name, conn->case_sensitive);
	}

	/* 
	 * The name has been resolved.
	 */

	DEBUG(5,("conversion finished %s -> %s\n",orig_path, name));
	return NT_STATUS_OK;
}

/****************************************************************************
 Check a filename - possibly caling reducename.
 This is called by every routine before it allows an operation on a filename.
 It does any final confirmation necessary to ensure that the filename is
 a valid one for the user to access.
****************************************************************************/

NTSTATUS check_name(connection_struct *conn, const pstring name)
{
	if (IS_VETO_PATH(conn, name))  {
		/* Is it not dot or dot dot. */
		if (!((name[0] == '.') && (!name[1] || (name[1] == '.' && !name[2])))) {
			DEBUG(5,("check_name: file path name %s vetoed\n",name));
			return map_nt_error_from_unix(ENOENT);
		}
	}

	if (!lp_widelinks(SNUM(conn)) || !lp_symlinks(SNUM(conn))) {
		NTSTATUS status = reduce_name(conn,name);
		if (!NT_STATUS_IS_OK(status)) {
			DEBUG(5,("check_name: name %s failed with %s\n",name, nt_errstr(status)));
			return status;
		}
	}

	return NT_STATUS_OK;
}

/****************************************************************************
 Scan a directory to find a filename, matching without case sensitivity.
 If the name looks like a mangled name then try via the mangling functions
****************************************************************************/

static BOOL scan_directory(connection_struct *conn, const char *path, char *name, size_t maxlength)
{
	struct smb_Dir *cur_dir;
	const char *dname;
	BOOL mangled;
	long curpos;

	mangled = mangle_is_mangled(name, conn->params);

	/* handle null paths */
	if (*path == 0)
		path = ".";

	/*
	 * The incoming name can be mangled, and if we de-mangle it
	 * here it will not compare correctly against the filename (name2)
	 * read from the directory and then mangled by the mangle_map()
	 * call. We need to mangle both names or neither.
	 * (JRA).
	 *
	 * Fix for bug found by Dina Fine. If in case sensitive mode then
	 * the mangle cache is no good (3 letter extension could be wrong
	 * case - so don't demangle in this case - leave as mangled and
	 * allow the mangling of the directory entry read (which is done
	 * case insensitively) to match instead. This will lead to more
	 * false positive matches but we fail completely without it. JRA.
	 */

	if (mangled && !conn->case_sensitive) {
		mangled = !mangle_check_cache( name, maxlength, conn->params);
	}

	/* open the directory */
	if (!(cur_dir = OpenDir(conn, path, NULL, 0))) {
		DEBUG(3,("scan dir didn't open dir [%s]\n",path));
		return(False);
	}

	/* now scan for matching names */
	curpos = 0;
	while ((dname = ReadDirName(cur_dir, &curpos))) {

		/* Is it dot or dot dot. */
		if ((dname[0] == '.') && (!dname[1] || (dname[1] == '.' && !dname[2]))) {
			continue;
		}

		/*
		 * At this point dname is the unmangled name.
		 * name is either mangled or not, depending on the state of the "mangled"
		 * variable. JRA.
		 */

		/*
		 * Check mangled name against mangled name, or unmangled name
		 * against unmangled name.
		 */

		if ((mangled && mangled_equal(name,dname,conn->params)) || fname_equal(name, dname, conn->case_sensitive)) {
			/* we've found the file, change it's name and return */
			safe_strcpy(name, dname, maxlength);
			CloseDir(cur_dir);
			return(True);
		}
	}

	CloseDir(cur_dir);
	errno = ENOENT;
	return(False);
}
