#ifndef _OPENGEAR_ROLES_H_
#define _OPENGEAR_ROLES_H_

/*
 * Application roles for users.
 *
 * Roles are identifiers used by applications to modify their behaviour.
 * Roles are assigned to groups through configuration.
 *
 * A users's roles are determined as follows:
 *   1. Find the set of groups to which the user belongs (via groups database);
 *   2. Look up each group for the roles it is granted (via configuration);
 *   3. Grant the user the union of those roles.
 *
 * This API uses role identifiers and role sets.
 *
 * Role identifiers (role_t) should point to valid, constant C strings. NULL
 * may be used where a role identifier is optional.
 *
 * Role sets (roles_t) are opaque pointers that begin life as NULL (NO_ROLES)
 * (meaning the empty set), and are auto-reallocated by API functions. They
 * should be released with roles_free() when no longer needed.
 */
#include <stdlib.h>

#include <scew/scew.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A role identifier.
 * A role identifier is encoded as a constant string pointer.
 * Roles are compared using string equality (strcmp).
 * Callers should use well-known role pointers (eg #ROLE_ADMIN).
 * Roles sort by their pointed-to string content.
 * The @c NULL role indicates no role, and is never a member of any set.
 */
typedef const char *role_t;

/**
 * A role descriptor.
 * Descriptors contain more information about a role, and are
 * intended for programs that manipulate roles.
 * @see #ALL_ROLES.
 */
struct role_desc {
        role_t role;
        char text[1024]; /* English description */
};

/**
 * An array of all known roles.
 * These roles are in sorted order, terminated by a sentinel entry with
 * a @c NULL role field.
 */
extern const struct role_desc ALL_ROLES[];

/* Well-known roles. Callers should use these constants. */
extern const char ROLE_ADMIN[];			/* "admin" */
extern const char ROLE_ALL_PORTS_USER[];	/* "all_ports_user" */
extern const char ROLE_BASIC_WEBUI_USER[];	/* "basic_webui_user" */
extern const char ROLE_PMSHELL_USER[];		/* "pmshell_user" */
extern const char ROLE_SHELL_USER[];		/* "shell_user" */
extern const char ROLE_PORT_ADMIN[];		/* "port_admin" */

/**
 * Container set of roles.
 * An opaque structure
 * A @c NULL set is always treated as an empty set.
 * The @c NULL role is never a member of any set.
 * @see #roles_has(), #roles_str(), #add_roles_for_user().
 */
struct roles;

typedef struct roles *roles_t;
#define NO_ROLES ((struct roles *)0)

/** Test membership of a role set.
 *  @param roles  (optional) role set
 *  @param role   (optional) role
 *  @return 0 if @a role is not a member of @a roles. */
int roles_has(const struct roles *roles, role_t role);

/** Find common roles.
 *  Computes the intersection of sets @a a and @a b then
 *  adds them to @c *roles_ptr.
 *  @param[out] roles_ptr   Result set into which to add the intersecting
 *                          elements.
 *  @param a                A set
 *  @param b                A set
 *  @return -1 on allocation error.
 */
int roles_intersect(struct roles **roles_ptr,
		    const struct roles *a, const struct roles *b);
/**
 * Adds the roles assigned to a group into a role set.
 *
 * Roles for a group are obtained from the configuration root.
 * @see #role_group_is_forced()
 *
 * @param roles_ptr  The role set to insert the roles into.
 *                   This can be initialized with NULL, and released
 *	             later with #free().
 * @param groupname  The name of the group
 * @param config     (optional) Root of the config db
 * @return -1 on error.
 */
int roles_add_from_group(struct roles **roles_ptr, const char *groupname,
		const scew_element *config);

/**
 * Collects the roles granted to a user then adds them to a role set.
 *
 * @param roles_ptr  The role-set into which to insert the user's roles.
 *                   This can be initialized with NULL, and released
 *	             later with #free().
 * @param username   The username to load the roles from.
 * @param config     (optional) Root of the config db
 * @return -1 on error, with #errno set to
 *                    #ENOENT - username not found,
 *                    #ENOMEM - out of memory.
 */
int roles_add_from_user(struct roles **roles_ptr, const char *username,
		const scew_element *config);

/**
 * Make a human-readable string form of a role set.
 * The string representation will be of the form "admin users".
 * The empty and NULL role sets will be represented as "".
 * If the buffer is too short, the last two chars will be "+\0".
 * @param roles  (optional) role set
 * @param buf    Storage for the string. This will be nul-terminated
 *               if @a bufz if non-zero, even if truncation is required.
 * @param bufsz  Total size of @a buf
 * @return the number of non-NUL characters that were stored in @a buf.
 */
int roles_str(const struct roles *roles, char *buf, size_t bufsz);

/** Inserts a role into a role set.
 *  Used for building up roles.
 *  This function has no effect if @a role is NULL or if
 *  @a role is already a member of * @a role_ptr.
 *  @param roles_ptr  Storage for roles pointer. Should contain
 *                    NULL or a role set allocated with #malloc().
 *                    This function may #realloc() the pointer.
 *  @param role       (optional) The new role to insert.
 *                    This must match one of the roles in #ALL_ROLES.
 *  @return -1 on error (ENOMEM)
 */
int roles_add_role(struct roles **roles_ptr, role_t role);

/**
 * Copies all the source roles into the destination roles.
 */
int roles_add_all(struct roles **roles_ptr, const struct roles *src_roles);

/** Copies roles out of a role set into an array.
 *  @param roles  the role set from which to copy role identifiers
 *  @param buf    destination buffer for receiving role identifiers
 *  @param bufsz  the size of @a buf in chars
 *  @returns -1 on error (#errno) or
 *           number of roles copied into @a buf
 */
int roles_get(const struct roles *roles, role_t *buf, size_t bufsz);

/** Counts the number of roles in a set */
unsigned int roles_count(const struct roles *roles);

/** Compares two role sets for equality */
int roles_equal(const struct roles *a, const struct roles *b);

/** Releases any memory asociated with a role set.
 *  @param roles_ptr  storage for a role set. This will be
 *                    set to #NO_ROLES.
 */
void roles_free(struct roles **roles_ptr);

/** Tests if a group/role combination is internally forced.
 *  Some groups (eg admin, users) have roles forced upon them
 *  independent of configuration.
 *  Forced roles are implemented by #roles_add_from_group() and
 *  (indirectly) #roles_add_from_user().
 */
int role_group_is_forced(const char *group_name, role_t role);

#ifdef __cplusplus
}
#endif
#endif /* _OPENGEAR_ROLES_H_ */
