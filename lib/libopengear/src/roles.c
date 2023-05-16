#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include <opengear/xml.h>
#include <opengear/roles.h>

/*
 * Well-known roles.
 * Please Keep these sorted by string value.
 */
const char ROLE_ADMIN[] = "admin";
const char ROLE_ALL_PORTS_USER[] = "all_ports_user";
const char ROLE_BASIC_WEBUI_USER[] = "basic_webui_user";
const char ROLE_PMSHELL_USER[] = "pmshell_user";
const char ROLE_SHELL_USER[] = "shell_user";
const char ROLE_PORT_ADMIN[] = "port_admin";

const struct role_desc ALL_ROLES[] = {
	{ .role = ROLE_ADMIN,
	  .text = "Full administration & access",
	},
	{ .role = ROLE_ALL_PORTS_USER,
	  .text = "Access to all serial ports and managed devices",
	},
	{ .role = ROLE_BASIC_WEBUI_USER,
	  .text = "Web UI access to the 'Manage' pages",
	},
	{ .role = ROLE_PMSHELL_USER,
	  .text = "CLI connections provide access to the Port Manager shell"
		  " (This takes precedence over the UNIX Shell Role)",
	},
	{ .role = ROLE_SHELL_USER,
	  .text = "CLI connections provide access to a UNIX shell",
	},
	{ .role = ROLE_PORT_ADMIN,
	  .text = "Management Access to specified serial ports and managed devices",
	},
	{ /* sentinel */ }
};


/**
 * The internal structure for a set of roles.
 *
 * Each element of a #roles structure is
 * <ul>
 *   <li>canonical; i.e. each element is pointer-equal to one of the roles
 *       in the #ALL_ROLES table, and non-NULL; and
 *   <li>unique within the roles structure (this is the set property); and
 *   <li>sorted within the member array by its value's pointer address
 * </ul>
 *
 * Currently, roles unknown to #ALL_ROLES are not permitted in a role set.
 *
 * We use a NULL-terminated array of role_t strings. The array is allocated
 * in chunks. The empty set should always be represented as a null pointer
 * #NO_ROLES = (struct roles*)0, and never as a zero-length array.
 *
 * The convention for dealing with this opaque structure is that callers
 * will allocate storage for a #roles_t variable, initialize it
 * to #NO_ROLES, and pass a pointer to that storage to these role functions.
 * We will malloc or realloc the stored pointer as required.
 * When the caller no longer needs the role set, they call #roles_free().
 * This translates into a single #free() that deallocates the storage.
 */
struct roles {
	role_t member[1]; /* NULL-terminated; the array is
			   * allocated in groups of NROLES_CHUNK */
};
#define NROLES_CHUNK 8 /* allocation chunk size for roles */
#define SIZEOF_ROLES(n) /* allocation required for n roles */ \
	((((n) + 1 + NROLES_CHUNK) & ~(NROLES_CHUNK - 1)) * sizeof (role_t))


/**
 * Look up a role_desc by its .role string.
 * @return the matching role descriptor, or @c NULL if not found.
 */
static const struct role_desc *
find_role_desc(const char *role) {
	const struct role_desc *desc;

	if (!role)
		return NULL;
	for (desc = ALL_ROLES; desc->role; desc++)
		if (strcmp(desc->role, role) == 0)
			return desc;
	return NULL;
}

/**
 * Grow a 'struct roles' in-place.
 * @param n	the current number of slots, including the NULL terminal
 * @param new_n the desired number of slots, including the NULL terminal
 * @return the resulting *roles_ptr on success, or
 *	   @c NULL on error without changing ptr */
static struct roles *
roles_grow(struct roles **roles_ptr, size_t n, size_t new_n)
{
	struct roles *new_roles;

	if (*roles_ptr && SIZEOF_ROLES(n) < SIZEOF_ROLES(new_n))
		return *roles_ptr; /* no need to grow */
	if (!*roles_ptr) {
		new_roles = malloc(SIZEOF_ROLES(new_n));
	} else {
		new_roles = realloc(*roles_ptr, SIZEOF_ROLES(new_n));
	}
	if (new_roles)
		*roles_ptr = new_roles;
	return new_roles;
}

int
roles_add_role(struct roles **roles_ptr, role_t /*noncanon*/ role)
{
	const struct role_desc *desc;
	struct roles *roles;
	size_t nroles, i;

	if (!role)
		return 0;

	/* use an intern'd role_t */
	desc = find_role_desc(role);
	if (!desc)
		return 0; /* ignore unknown roles */
	roles = *roles_ptr;

	/* is the role already present? calculate nroles as side effect. */
	if (roles) {
		for (nroles = 0; roles->member[nroles]; nroles++)
			if (roles->member[nroles] == desc->role)
				return 0;
	} else
		nroles = 0;

	/* make space for nroles+1, plus the terminating NULL */
	roles = roles_grow(roles_ptr, roles ? nroles + 1 : 0, nroles + 2);
	if (!roles)
		return -1;

	/* insert, keeping the roles in address order,
	 * for roles_equal() and roles_intersect() */
	for (i = nroles; i > 0; i--) {
		if (roles->member[i - 1] > desc->role)
			roles->member[i] = roles->member[i - 1];
		else
			break;
	}
	roles->member[i] = desc->role;
	roles->member[nroles + 1] = NULL;
	return 0;
}

int
roles_add_all(struct roles **roles_ptr, const struct roles *roles)
{
	const role_t *r;

	if (roles) {
		for (r = roles->member; *r; r++) {
			if (roles_add_role(roles_ptr, *r) == -1) {
				return -1;
			}
		}
	}
	return 0;
}

unsigned int
roles_count(const struct roles *roles)
{
	unsigned count = 0;
	const role_t *r;

	if (roles) {
		for (r = roles->member; *r; r++) {
			count++;
		}
	}
	return count;
}

int
roles_equal(const struct roles *a, const struct roles *b)
{
	const role_t empty = NULL;
	const role_t *ra = a ? &a->member[0] : &empty;
	const role_t *rb = b ? &b->member[0] : &empty;

	while (*ra || *rb) {
		if (*ra++ != *rb++)
			return 0;
	}
	return 1;
}

int
roles_get(const struct roles *roles, role_t *buf, size_t bufsz)
{
	unsigned count = 0;
	const role_t *r;

	if (roles) {
		for (r = roles->member; *r; r++) {
			if ((count + 1) * sizeof (role_t) > bufsz) {
				errno = ENOMEM;
				return -1;
			}
			buf[count++] = *r;
		}
	}
	return count;
}

int
roles_has(const struct roles *roles, role_t /*noncanon*/ role)
{
	const role_t *r;

	if (!role || !roles)
		return 0;
	for (r = roles->member; *r; r++) {
		/* Use strcmp because the role argument may not be canonical */
		if (strcmp(role, *r) == 0)
			return 1;
	}
	return 0;
}

int
roles_intersect(struct roles **roles_ptr, const struct roles *a,
	const struct roles *b)
{
	const role_t *ra, *rb;

	if (!a || !b)
		return 0;

	/* Rely on member roles being canonical and sorted */
	ra = a->member;
	rb = b->member;
	while (*ra && *rb) {
		if (*ra == *rb) {
			if (roles_add_role(roles_ptr, *ra) == -1)
				return -1;
			ra++;
			rb++;
		} else if (*ra < *rb) {
			ra++;
		} else /* (*ra > *rb) */ {
			rb++;
		}
	}
	return 0;
}

static int
roles_add_from_list(struct roles **roles_ptr, const scew_element *tree, const char *noun)
{
	uint32_t total, i;

	if (!tree || !xml_get_uint(tree, "total", &total))
		return 0;
	for (i = 1; i <= total; i++) {
		char key[32];
		const char *value = NULL;

		snprintf(key, sizeof key, "%s%u", noun, i);
		if (xml_get_string(tree, key, &value)) {
			/* OK if value if NULL or empty or malformed, as
			 * because roles_add_role() will ignore it. */
			if (roles_add_role(roles_ptr, value) == -1)
				return -1;
		}
	}
	return 0;
}

/*
 * Special built-in groups have forced roles.
 * For example, group "admin" is always granted role ADMIN.
 * This addresses the special cases of reboot after upgrading, or
 * use after restoring a pre-role system.
 *
 * Note: this is not the gaining of group membership by dint of
 * having a role. Local unix group membership is determined by the
 * users configurator.
 */
static const struct forced_role {
	const char *group_name;
	role_t role;
} FORCED_ROLES[] = {
	{ "admin", ROLE_ADMIN },
	{ "users", ROLE_BASIC_WEBUI_USER },
	{ } /*sentinel*/
};

int
role_group_is_forced(const char *group_name, role_t role)
{
	const struct forced_role *fr;

	for (fr = FORCED_ROLES; fr->group_name; fr++) {
		if (strcmp(fr->role, role) == 0 &&
		    strcmp(fr->group_name, group_name) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int
roles_add_from_group(struct roles **roles_ptr, const char *group_name,
	const scew_element *config)
{
	const scew_element *group_subtree;
	const scew_element *roles_subtree;
	const struct forced_role *fr;

	/* Add all the forced roles for this group */
	for (fr = FORCED_ROLES; fr->group_name; fr++) {
		if (strcmp(fr->group_name, group_name) == 0) {
			if (roles_add_role(roles_ptr, fr->role) == -1) {
				return -1;
			}
		}
	}

	/* Add all the roles as declared by configuration */
	group_subtree = scew_xml_search(config, "groups", "name", group_name);
	roles_subtree = scew_xml_subtree(group_subtree, "roles");
	if (roles_add_from_list(roles_ptr, roles_subtree, "role") == -1)
		return -1;

	return 0;
}

/**
 * A namelist is an internal structure used for collecting
 * a unique list of short strings (like group names).
 * Its maximum length and the size of the names is limited.
 */
#define NAMELIST_MAXLEN 256
#define NAME_MAXLEN 256
struct namelist {
	unsigned count;
	char name[NAMELIST_MAXLEN][NAME_MAXLEN];
};

/** Adds a name into a namelist, unless it is already there. */
static int
namelist_add_unique(struct namelist *nl, const char *name)
{
	unsigned i;
	int namelen = strlen(name);

	if (namelen >= NAME_MAXLEN) {
		errno = ENOMEM;
		return -1;
	}
	for (i = 0; i < nl->count; i++)
		if (strcmp(name, nl->name[i]) == 0)
			return 0;
	if (nl->count >= NAMELIST_MAXLEN) {
		errno = ENOMEM;
		return -1;
	}
	strcpy(nl->name[nl->count++], name);
	return 0;
}

int
roles_add_from_user(struct roles **roles_ptr, const char *user_name,
	const scew_element *config)
{
	struct passwd *pw;
	struct group *gr;
	struct namelist grouplist;
	unsigned i;

	/* Check that the user exists, and fetch their primary group */
	pw = getpwnam(user_name);
	if (!pw) {
		errno = ENOENT;
		return -1;
	}

	/* Get the group membership of the user (from /etc/groups) */
	grouplist.count = 0;
	setgrent();
	while ((gr = getgrent())) {
		char **memp;
		if (gr->gr_gid == pw->pw_gid) {
			/* this is the user's primary group */
			namelist_add_unique(&grouplist, gr->gr_name);
			continue;
		}
		for (memp = gr->gr_mem; *memp; memp++) {
			/* this group explicitly names the user as a member */
			if (strcmp(*memp, user_name) == 0) {
				namelist_add_unique(&grouplist, gr->gr_name);
				break;
			}
		}
	}
	endgrent();

	/* Union of roles over the user's groups */
	for (i = 0; i < grouplist.count; i++) {
		if (roles_add_from_group(roles_ptr, grouplist.name[i],
					 config) == -1)
			return -1;
	}

	/* Root always gets admin */
	if (strcmp(user_name, "root") == 0) {
		if (roles_add_role(roles_ptr, ROLE_ADMIN) == -1)
			return -1;
	}

	return 0;
}

int
roles_str(const struct roles *roles, char *buf, size_t bufsz)
{
	size_t n = 0;
	const role_t *r;
	const char *s;

	if (roles) {
		for (r = roles->member; n < bufsz && *r; r++) {
			if (n && n < bufsz) buf[n++] = ' ';
			for (s = *r; n < bufsz && *s; s++)
				buf[n++] = *s;
		}
	}
	if (n == bufsz) {
		/* Fill last two chars with "+\0" */
		if (n > 2) n -= 2;
		else	   n = 0;
		if (n + 1 < bufsz) buf[n++] = '+';
	}
	if (n < bufsz) buf[n] = '\0';
	return n;
}

void
roles_free(struct roles **roles_ptr)
{
	free(*roles_ptr);
	*roles_ptr = NO_ROLES;
}
