#ifndef _OPENGEAR_PWGRP_H_
#define _OPENGEAR_PWGRP_H_

/** @file
 * Passwd and Group editors.
 *
 * Functions to lock and load /etc/passwd, /etc/shadow and /etc/groups into
 * memory. The caller can make changes and then save them back again.
 *
 * All structures returned have managed string fields.
 * Examples of use:
 * <code>
 *     struct pwgrp *pg = pwgrp_open();
 *     struct passwd *pw = pwgrp_getpwbynam(pg, "root");
 *     pwgrp_setstr(&pw->pw_shell, "/bin/sh");
 *     pwgrp_close();
 * </code>
 */

#include <pwd.h>
#include <grp.h>
#ifdef HAVE_SHADOW
# include <shadow.h>
#endif
#include <stdarg.h>

#ifdef	__cplusplus
extern "C" {
#endif

/** Duplicates a passwd entry.
 *  @returns a passwd structure with managed strings. */
struct passwd *passwd_dup(const struct passwd *pw);

/** Frees a duplicated passwd entry, and assigns NULL to the pointer. */
void passwd_release(struct passwd **);

#ifdef HAVE_SHADOW
/** Duplicates a shadow entry. */
struct spwd *spwd_dup(const struct spwd *sp);

/** Frees a duplicated shadow entry, and assigns NULL to the pointer. */
void spwd_release(struct spwd **);
#endif

/** Duplicates a group entry. */
struct group *group_dup(const struct group *gr);

/** Frees a group entry, and assigns NULL to the pointer. */
void group_release(struct group **);

/** Adds a username to a duplicated group entry's membership.
 *  No effect if the name is already in the membership list.
 *  @return -1 on allocation error
 */
int group_add_mem(struct group *gr, const char *name);

/** Removes matching username from a duplicated group entry's membership.
 *  No effect if the name is not present. */
void group_del_mem(struct group *gr, const char *name);

/** Removes all usernames from a duplicated group entry's membership. */
void group_clear_mem(struct group *gr);

/** Tests if a group names a user as a member. */
int group_has_mem(const struct group *gr, const char *username);

/**
 * An in-memory representation of system databases.
 *
 * The passwd field points to an array of length npasswd pointers to
 * #passwd entries.
 * A NULL pointer indicates a deleted entry. Non-NULL entries always
 * point to "duplicated passwd" structures (see #passwd_dup()).
 * These structures have string fields that are always allocated
 * with #pwgrp_setstr().
 *
 * To append a new passwd entry to the end of the array, call
 * #pwgrp_new_passwd().
 * To delete a passwd entry in the array, call #passwd_release() to
 * release it and replace it with a NULL pointer. It is normal to have
 * NULL entries in the #pwgrp.passwd array.
 *
 * To modify a string field in an existing #passwd entry,
 * use #pwgrp_setstr().  This will first free the field, then replace
 * it with a copy of the new string.
 *
 * The other arrays (#pwgrp.group and #pwgrp.spwd) are used in the same way,
 * except that the gr_mem field of #group should be manipulated
 * with #group_add_mem(), #group_del_mem() or #group_clear_mem().
 */
struct pwgrp {
	int lockfd;		/* lock file descriptor */

	struct passwd **passwd;
	unsigned npasswd;

	struct group **group;
	unsigned ngroup;

#ifdef HAVE_SHADOW
	struct spwd **spwd;
	unsigned nspwd;
#endif
};

#ifndef _PATH_PASSWD_LOCK
# define _PATH_PASSWD_LOCK "/var/run/passwd.lck"
#endif


/** Allocate, lock and load content from system databases.
 *  @returns NULL on error and sets #errno. */
struct pwgrp *pwgrp_open();

/** Store content, unlock and release memory.
 *  @param pg (optional) editor handle; this is always invalidated
 *  @returns -1 on error and sets #errno. */
int pwgrp_close(struct pwgrp *pg);

/** Unlock and release memory. (Doesn't write)
 *  @param pg (optional) editor handle; this is always invalidated
 */
void pwgrp_abort(struct pwgrp *pg);

/** Appends a new entry to the passwd array.
 *  The entry returned will exist in the #pwgrp.passwd array.
 *  The entry's fields are all initialised to managed, empty strings
 *  and can be manipulated with #pwgrp_setstr().
 *
 *  The entire entry can be duplicated with #passwd_dup() or
 *  recursively released with #passwd_free() or #passwd_release().
 *
 *  @param pg editor handle
 *  @return NULL on error and sets #errno
 */
struct passwd *pwgrp_new_passwd(struct pwgrp *pg);
#ifdef HAVE_SHADOW
struct spwd *pwgrp_new_spwd(struct pwgrp *pg);
#endif
struct group *pwgrp_new_group(struct pwgrp *pg);

/** Search for an entry by name.
 *  Searches the #pwgrp.passwd array for the first matching #passwd.pw_name
 *  @param pg   editor handle
 *  @param name username to match
 *  @return first matching entry in @a pg->passwd, or
 *          NULL if not found.
 */
struct passwd *pwgrp_getpwnam(struct pwgrp *pg, const char *name);
struct passwd *pwgrp_getpwuid(struct pwgrp *pg, uid_t uid);
#ifdef HAVE_SHADOW
struct spwd *pwgrp_getspnam(struct pwgrp *pg, const char *name);
#endif
struct group *pwgrp_getgrnam(struct pwgrp *pg, const char *name);
struct group *pwgrp_getgrgid(struct pwgrp *pg, gid_t gid);
int pwgrp_getgrouplist(struct pwgrp *pg, const char *user, gid_t group,
                               gid_t *groups, int *ngroups);

/** Sets a string field. Replaces *destp with a copy of the src string,
 *  Releasing memory as needed.
 *  @param destp  reference to the pointer being replaced
 *  @param src    the string to copy (must not be NULL)
 *  @return the string allocated and stored at @a *destp, or
 *          NULL if allocation failed and *destp was left unchanged
 */
char *pwgrp_setstr(char **destp, const char *src);

/** Sets a string field using a format.
 *  @param destp  reference to the pointer being replaced
 *  @param fmt    a format argument; @see #vsprintf()
 *  @return the string allocated and stored at @a *destp, or
 *          NULL if allocation failed and *destp was left unchanged
 */
__attribute__((format(printf,2,3)))
char *pwgrp_setf(char **destp, const char *fmt, ...);

char *pwgrp_vsetf(char **destp, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_PWGRP_H_ */
