#define _GNU_SOURCE

#include <sys/types.h>

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <grp.h>
#include <pwd.h>

#include <scew/scew.h>

#include <opengear/queue.h>
#include <opengear/users.h>
#include <opengear/serial.h>
#include <opengear/roles.h>
#include <opengear/pwgrp.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

struct host {
	char *address;
	TAILQ_ENTRY(host) list;
};

struct outlet {
	char *configpath;
	TAILQ_ENTRY(outlet) list;
};

struct user {
	char *username;
	uid_t uid;
	gid_t gid;
	int num_ports;
	bool *ports; /* Which ports are allowed for this user */
	TAILQ_HEAD(hosthead, host) hostlist;
	TAILQ_HEAD(outlethead, outlet) outletlist;
	SIMPLEQ_ENTRY(user) list;
};

struct usergroup {
	char *groupname;
	gid_t gid;
	int num_ports;
	bool *ports; /* Which ports are allowed for this group */
	struct hosthead hostlist;
	struct outlethead outletlist;
	SIMPLEQ_ENTRY(usergroup) list;
};

static SIMPLEQ_HEAD(, user) userlist = SIMPLEQ_HEAD_INITIALIZER(userlist);
static SIMPLEQ_HEAD(, usergroup) grouplist =
	SIMPLEQ_HEAD_INITIALIZER(grouplist);

#define USER_PASSWDFILE "/etc/passwd"
#define USER_GROUPFILE "/etc/group"
#define USER_MINUID 1000
#define MAXBUFLEN 4096

#define NO_UID ((uid_t) -1)
#define NO_GID ((gid_t) -1)

/** Return roles granted to the provided uid and gid. */
static roles_t
get_roles(xmldb_t *db, uid_t uid, gid_t gid)
{
	struct passwd *pw;
	struct group *gr;
	roles_t roles = NO_ROLES;

	if (gid != NO_GID) {
		gr = getgrgid(gid);
		if (gr) {
			roles_add_from_group(&roles, gr->gr_name, xmldb_root(db));
		}
	}

	if (uid != NO_UID) {
		pw = getpwuid(uid);
		if (pw) {
			roles_add_from_user(&roles, pw->pw_name, xmldb_root(db));
		}
	}

	return roles;
}

int
host_create(struct host **hostp, const char *address)
{
	struct host *host = calloc(1, sizeof(host));
	if (host == NULL) {
		perror("calloc");
		return (-1);
	}

	host->address = strdup(address);
	*hostp = host;
	return (0);
}

static void
host_destroy(struct host **hostp)
{
	struct host *host = *hostp;
	if (host != NULL) {
		if (host->address != NULL) {
			free(host->address);
		}
		free(host);
	}
	*hostp = NULL;
}

int
outlet_create(struct outlet **outletp, const char *configpath)
{
	struct outlet *outlet = calloc(1, sizeof(outlet));
	if (outlet == NULL) {
		perror("calloc");
		return (-1);
	}

	outlet->configpath = strdup(configpath);
	*outletp = outlet;
	return (0);
}

static void
outlet_destroy(struct outlet **outletp)
{
	struct outlet *outlet = *outletp;
	if (outlet != NULL) {
		if (outlet->configpath != NULL) {
			free(outlet->configpath);
		}
		free(outlet);
	}
	*outletp = NULL;
}

void
outlets_load(xmldb_t *db, const char *policy,
		const char *prefix, const char *name,
		struct outlethead *head)
{
	size_t outletidx, numoutlets, policyidx, numpolicies;
	const char *s;
	char id[sizeof("config.sdt.hosts.host9999"
			".power.outlet9999.group9999")];

	snprintf(id, sizeof(id), "%s.power", prefix);
	s = xmldb_getstring(db, id, "type");
	if (s == NULL || s[0] == '\0') {
		return;
	}

	numoutlets = xmldb_getsize(db, id, "outlets");
	for (outletidx = 1; outletidx <= numoutlets; ++outletidx) {
		snprintf(id, sizeof(id), "%s.power.outlet%zu.%ss",
			prefix, outletidx, policy);

		numpolicies = xmldb_getsize(db, id, "total");
		for (policyidx = 1; policyidx <= numpolicies; ++policyidx) {
			char policyid[sizeof("policy9999")];
			struct outlet *outlet = NULL;

			snprintf(policyid, sizeof(policyid), "%s%zu",
				policy, policyidx);
			s = xmldb_getstring(db, id, policyid);
			if (s == NULL || s[0] == '\0') {
				continue;
			}
			if (strcmp(s, name) != 0) {
				continue;
			}

			snprintf(id, sizeof(id), "%s.power.outlet%zu",
				prefix, outletidx);

			if (outlet_create(&outlet, id) == 0) {
				TAILQ_INSERT_TAIL(head, outlet, list);
				break;
			}
		}
	}
}

static int
user_create(struct user **userp, xmldb_t *db, struct passwd *pw)
{
	const scew_element *parent = NULL, *elem = NULL;
	size_t i, total = 0;
	struct user *u = NULL;
	/* Peruse config for the specified user configuration */
	total = xmldb_getsize(db, "config.users", "total");
	for (i = 1; i <= total; ++i) {
		char userid[sizeof("config.users.user9999")];
		const char *username = NULL;

		snprintf(userid, sizeof(userid), "config.users.user%zu", i);

		username = xmldb_getstring(db, userid, "username");
		if (username == NULL || username[0] == '\0') {
			continue;
		}
		if (strcmp(username, pw->pw_name) != 0) {
			continue;
		}

		u = calloc(1, sizeof(*u));
		if (u == NULL) {
			perror("calloc");
			return (-1);
		}
		u->username = strdup(username);
		if (u->username == NULL) {
			perror("calloc");
			free(u);
			return (-1);
		}
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
		u->ports = xmldb_portlist(db, userid, &u->num_ports);
		if (u->ports == NULL) {
			perror("xmldb_portlist");
			free(u->username);
			free(u);
			return -1;
		}
		TAILQ_INIT(&u->hostlist);
		TAILQ_INIT(&u->outletlist);
		break;
	}

	if (u == NULL) {
		return (-1);
	}
	/* Load accessible serial RPC outlets */
	parent = xmldb_subtree(db, "ports");
	scew_list *port_elem = NULL;
	scew_list *ports = parent ? scew_element_children(parent) : NULL;
	for (port_elem = ports; port_elem != NULL; port_elem = scew_list_next(port_elem)) {
    	elem = (scew_element *) scew_list_data(port_elem);
		const char *name = scew_element_name(elem);
		unsigned int portnum, len = sizeof("port") - 1;
		char id[sizeof("config.ports." OG_MAX_PORTDEV)];

		if (name == NULL) {
			continue;
		}
		if (strlen(name) < len + 1 ||
				strncasecmp(name, "port", len) != 0) {
			continue;
		}
		if (sscanf(name + len, "%u", &portnum) != 1) {
			continue;
		}

		snprintf(id, sizeof(id), "config.ports.port%u", portnum);
		outlets_load(db, "user", id, pw->pw_name,
				(struct outlethead *)&u->outletlist);
	}

	/* Load accessible hosts */
	total = xmldb_getsize(db, "config.sdt.hosts", "total");
	for (i = 1; i <= total; ++i) {
		char id[sizeof("config.sdt.hosts.host9999"
				".power.outlet9999.users.user9999")];
		struct host *host = NULL;
		const char *address = NULL;
		size_t j, numusers;

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu", i);
		address = xmldb_getstring(db, id, "address");
		if (address == NULL || address[0] == '\0') {
			continue;
		}

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu.users", i);
		numusers = xmldb_getsize(db, id, "total");
		for (j = 1; j <= numusers; ++j) {
			char userid[sizeof("user9999")];
			const char *user = NULL;

			snprintf(userid, sizeof(userid), "user%zu", j);
			user = xmldb_getstring(db, id, userid);
			if (user == NULL || user[0] == '\0') {
				continue;
			}
			if (strcmp(user, pw->pw_name) != 0) {
				continue;
			}

			if (host_create(&host, address) == 0) {
				TAILQ_INSERT_TAIL(&u->hostlist, host, list);
				break;
			}
		}

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu", i);
		outlets_load(db, "user", id, pw->pw_name,
				(struct outlethead *)&u->outletlist);
	}

	*userp = u;
	return (0);
}

static void
user_destroy(struct user **userp)
{
	struct user *u = *userp;

	if (u != NULL) {
		if (u->username != NULL) {
			free(u->username);
		}
		if (u->ports != NULL) {
			free(u->ports);
		}
		while (!TAILQ_EMPTY(&u->hostlist)) {
			struct host *host = TAILQ_FIRST(&u->hostlist);
			TAILQ_REMOVE(&u->hostlist, host, list);
			host_destroy(&host);
		}
		while (!TAILQ_EMPTY(&u->outletlist)) {
			struct outlet *outlet = TAILQ_FIRST(&u->outletlist);
			TAILQ_REMOVE(&u->outletlist, outlet, list);
			outlet_destroy(&outlet);
		}
		free(u);
	}
	*userp = NULL;
}

/**
 * Allocates and loads a #usergroup  structure from a config.groups subtree.
 *
 * Allocates a #usergroup and fills it in with data read from a matching
 * group subtree under config.groups as identified by gr->gr_name.
 *
 * @param[out] usergroupp  where to store the new #usergroup object pointer.
 *                         The caller should release with #usergroup_destroy().
 * @param[in]  db          config database
 * @param[in]  gr          which group to load; Only the gr_name field is used.
 * @return -1 on error/not found, 0 on success
 */
static int
usergroup_create(struct usergroup **usergroupp, xmldb_t *db, struct group *gr)
{
	const scew_element *parent = NULL, *elem = NULL;
	size_t i, total = 0;
	struct usergroup *g = NULL;
	/* Hunt config.groups for an existing group with name gr->gr_name */
	total = xmldb_getsize(db, "config.groups", "total");
	for (i = 1; i <= total; ++i) {
		char groupid[sizeof("config.groups.group9999")];
		const char *name = NULL;

		snprintf(
			groupid, sizeof(groupid), "config.groups.group%zu", i);
		name = xmldb_getstring(db, groupid, "name");
		if (name == NULL || name[0] == '\0') {
			perror(groupid);
			continue;
		}
		if (strcmp(name, gr->gr_name) != 0) {
			continue;
		}

		/* We found the group in config; allocate the struct usergroup
		 * and copy into it the portlist */
		g = calloc(1, sizeof(*g));
		if (g == NULL) {
			perror("calloc");
			return (-1);
		}
		g->groupname = strdup(name);
		if (g->groupname == NULL) {
			perror("calloc");
			free(g);
			return (-1);
		}
		g->ports = xmldb_portlist(db, groupid, &g->num_ports);
		if (g->ports == NULL) {
			perror("calloc");
			free(g->groupname);
			free(g);
			return (-1);
		}
		TAILQ_INIT(&g->hostlist);
		TAILQ_INIT(&g->outletlist);
		break;
	}

	if (g == NULL) {
		return (-1);
	}

	/* Load accessible serial RPC outlets */
	parent = xmldb_subtree(db, "ports");
	scew_list *outlet = NULL;
	scew_list *outlets = parent ? scew_element_children(parent) : NULL;
	for (outlet = outlets; outlet != NULL; outlet = scew_list_next(outlet)) {
    	elem = (scew_element *) scew_list_data(outlet);
		const char *name = scew_element_name(elem);
		unsigned int portnum, len = sizeof("port") - 1;
		char id[sizeof("config.ports." OG_MAX_PORTDEV)];

		if (name == NULL) {
			continue;
		}
		if (strlen(name) < len + 1 ||
				strncasecmp(name, "port", len) != 0) {
			continue;
		}
		if (sscanf(name + len, "%u", &portnum) != 1) {
			continue;
		}

		snprintf(id, sizeof(id), "config.ports.port%u", portnum);
		outlets_load(db, "group", id, gr->gr_name,
				(struct outlethead *)&g->outletlist);
	}

	/* Load accessible hosts */
	total = xmldb_getsize(db, "config.sdt.hosts", "total");
	for (i = 1; i <= total; ++i) {
		char id[sizeof("config.sdt.hosts.host9999.groups.group9999")];
		struct host *host = NULL;
		const char *address = NULL;
		size_t j, numusers;

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu", i);
		address = xmldb_getstring(db, id, "address");
		if (address == NULL || address[0] == '\0') {
			continue;
		}

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu.groups", i);
		numusers = xmldb_getsize(db, id, "total");
		for (j = 1; j <= numusers; ++j) {
			char userid[sizeof("group9999")];
			const char *user = NULL;

			snprintf(userid, sizeof(userid), "group%zu", j);
			user = xmldb_getstring(db, id, userid);
			if (user == NULL || user[0] == '\0') {
				continue;
			}
			if (strcmp(user, gr->gr_name) != 0) {
				continue;
			}

			if (host_create(&host, address) == 0) {
				TAILQ_INSERT_TAIL(&g->hostlist, host, list);
				break;
			}
		}

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu", i);
		outlets_load(db, "group", id, gr->gr_name,
				(struct outlethead *)&g->outletlist);
	}

	*usergroupp = g;
	return (0);
}

static void
usergroup_destroy(struct usergroup **usergroupp)
{
	struct usergroup *g = *usergroupp;

	if (g != NULL) {
		if (g->groupname != NULL) {
			free(g->groupname);
		}
		if (g->ports != NULL) {
			free(g->ports);
		}
		while (!TAILQ_EMPTY(&g->hostlist)) {
			struct host *host = TAILQ_FIRST(&g->hostlist);
			TAILQ_REMOVE(&g->hostlist, host, list);
			host_destroy(&host);
		}
		free(g);
	}
	*usergroupp = NULL;
}

bool
opengear_users_ispwingroup(struct passwd *pw, struct group *gr)
{
	return gr->gr_gid == pw->pw_gid || group_has_mem(gr, pw->pw_name);
}

static void update_ports(bool destports[], bool srcports[], int destports_size, int srcports_size) {
	int i;
	for (i = 0; i < srcports_size; i++) {
		if (i < destports_size) {
			if (srcports[i]) {
				destports[i] = true;
			}
		} else {
			break;
		}
	}
}


/**
 * opengear_users_permissions cache of user data loaded via opengear_users_load_permissions().
 * This contains all necessary data needed for check_access() which uses the data
 * to fullfil queries relating to user access to various resources.
 */

struct opengear_users_permissions {
	struct user *user;
	bool user_in_password_database;
	bool roles_allow_all;
	bool roles_port_admin;
	bool roles_admin;
	bool is_root;	/* uid == 0 */
	struct groups {
		struct usergroup *usergroup;
		bool roles_allow_all;
		bool roles_port_admin;
		bool roles_admin;
	} *groups; /* array of user groups */
	int groups_size; /* number of entries in groups */
	int groups_allocated_size; /* allocated number of entries in groups */
};

static void
initialise_opengear_users_permissions(struct opengear_users_permissions *perms)
{
	perms->user = NULL;
	perms->user_in_password_database = false;
	perms->roles_allow_all = false;
	perms->roles_port_admin = false;
	perms->roles_admin = false;
	perms->is_root = false;
	perms->groups = NULL;
	perms->groups_size = 0;
	perms->groups_allocated_size = 0;
}

/* add_group_permissions_item - Append groups item to the array in the opengear_users_permissions
 * struct. Takes ownership of struct usergroup *usergroup.
 */
static int
add_group_permissions_item(
	struct opengear_users_permissions *perms,
	struct usergroup *usergroup,
	bool roles_allow_all,
	bool roles_port_admin,
	bool roles_admin)
{
	/* First reallocate array if required */
	const int alloc_increment_size = 10;
	if (perms->groups_size + 1 > perms->groups_allocated_size) {
		/* allocate a bigger array */
		void *new_array = realloc(perms->groups,
			(perms->groups_allocated_size + alloc_increment_size)
			* sizeof(*perms->groups));
		if (!new_array) {
			return -1;
		}
		perms->groups = new_array;
		perms->groups_allocated_size += alloc_increment_size;
	}
	/* Append the entry and update size */
	perms->groups[perms->groups_size].usergroup = usergroup;
	perms->groups[perms->groups_size].roles_allow_all = roles_allow_all;
	perms->groups[perms->groups_size].roles_port_admin = roles_port_admin;
	perms->groups[perms->groups_size].roles_admin = roles_admin;
	perms->groups_size++;
	return 0;
}


opengear_users_permissions_t
opengear_users_load_permissions(xmldb_t *db, uid_t uid, gid_t gid)
{
	struct opengear_users_permissions *perms;
	bool failed = false;
	char *buf = NULL;
	roles_t roles = NULL;

	perms = malloc(sizeof(struct opengear_users_permissions));
	if (!perms) {
		perror("Failed to allocate opengear_users_permissions");
		failed = true;
		goto onexit;
	}
	initialise_opengear_users_permissions(perms);

	/* Root is always allowed */
	if (uid == 0) {
		perms->is_root = true;
		goto onexit;
	}

	roles = get_roles(db, uid, gid);
	if (roles_has(roles, ROLE_ALL_PORTS_USER) || roles_has(roles, ROLE_ADMIN)) {
		/* Note: roles_allow_all is sometimes combined with user info loaded below.
		 * Some functions only use the roles if the user is valid.
		 * Some functions use a raise_admin flag to decide whether to use the roles
		 * in the absence of a valid user. */
		perms->roles_allow_all = true;
	}
	if (roles_has(roles, ROLE_PORT_ADMIN)) {
		perms->roles_port_admin = true;
	}
	if (roles_has(roles, ROLE_ADMIN)) {
		perms->roles_admin = true;
	}

	/* Create a group entry. This record will be used if there is no user id match but also */
	/* used later as the first item when iterating over the getgrent list. */
	struct group gr, *grp = NULL;
	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (!buf) {
		perror("Failed to allocate getgrgid buffer");
		failed = true;
		goto onexit;
	}

	/* Check users credentials.*/
	struct passwd *pw = getpwuid(uid);
	if (pw == NULL) {
		/* No user on system with the given id. */

		if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0) {
			struct usergroup *usergroup = NULL;
			(void)usergroup_create(&usergroup, db, grp);
			/* Because there is no user for uid, the previous call to get_roles
			 * will be the same result as calling get_roles(db, NO_UID, gid).
			 * So we can just copy the flag into the group_permissions data. */
			if (add_group_permissions_item(perms, usergroup, perms->roles_allow_all, perms->roles_port_admin, perms->roles_admin) != 0) {
				perror("Failed to allocate storage for group_permissions item");
				failed = true;
				usergroup_destroy(&usergroup);
				goto onexit;
			}
		}
		goto onexit;
	}
	/* else pw is not null - User exists */
	perms->user_in_password_database = true;

	(void)user_create(&perms->user, db, pw);

	/* Check group credentials.*/
	setgrent();
	while (grp = getgrent()) {
		if (!opengear_users_ispwingroup(pw, grp)) {
			continue;
		}

		struct usergroup *usergroup = NULL;

		/* Set allow all flag based on group roles. Note some functions use
		 * the raise_admin flag to signal whether the group role should be used
		 * or not so we still need to load the rest of the group permissions
		 * even if the roles_allow_all flag is set. */
		roles_free(&roles);
		roles = get_roles(db, NO_UID, grp->gr_gid);
		bool roles_allow_all = (roles_has(roles, ROLE_ALL_PORTS_USER)
			|| roles_has(roles, ROLE_ADMIN));
		bool roles_port_admin = roles_has(roles, ROLE_PORT_ADMIN);
		bool roles_admin = roles_has(roles, ROLE_ADMIN);

		(void)usergroup_create(&usergroup, db, grp);

		if (add_group_permissions_item(perms, usergroup, roles_allow_all, roles_port_admin, roles_admin) != 0) {
			perror("Failed to allocate storage for usergroup item");
			failed = true;
			usergroup_destroy(&usergroup);
			goto onexit;
		}
	}
onexit:
	endgrent();
	free(buf);
	roles_free(&roles);
	if (failed) {
		opengear_users_destroy_permissions_data(perms);
		return NULL;
	} else {
		/* success */
		return perms;
	}
}

void
opengear_users_destroy_permissions_data(opengear_users_permissions_t perms)
{
	int i;

	if (!perms) {
		return;
	}

	user_destroy(&perms->user);
	for (i = 0; i < perms->groups_size; i++) {
		usergroup_destroy(&perms->groups[i].usergroup);
	}
	free(perms->groups);
	free(perms);
}

static bool
check_valid(opengear_users_permissions_t perms)
{
	if (!perms) {
		perror("Invalid opengear_users_permissions_t");
		return false;
	}
	return true;
}

void
opengear_users_getallowedports_with_perms(
	bool ports[], int numports,
	opengear_users_permissions_t perms)
{
	if (!check_valid(perms)) {
		return;
	}
	int i;
	if (perms->is_root || (perms->roles_allow_all && perms->user_in_password_database)) {
		for (i = 0; i < numports; i++) {
			ports[i] = true;
		}
		return;
	}
	if (perms->user) {
		struct user *u = perms->user;
		update_ports(ports, u->ports, numports, u->num_ports);
	}
	for (i = 0; i < perms->groups_size; i++) {
		struct usergroup *g = perms->groups[i].usergroup;
		if (g) {
			update_ports(ports, g->ports, numports, g->num_ports);
		}
	}
}

void
opengear_users_getallowedports(xmldb_t *db, bool ports[], int numports, uid_t uid, gid_t gid) {
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	opengear_users_getallowedports_with_perms(ports, numports, perms_h);
	opengear_users_destroy_permissions_data(perms_h);
}

static bool usergroup_has_port(struct usergroup *g, int port) {
	return (((port - 1) < g->num_ports) && (g->ports[(port - 1)]));
}

static bool user_has_port(struct user *u, int port) {
	return (((port - 1) < u->num_ports) && (u->ports[(port - 1)]));
}

static bool
grp_has_port(xmldb_t *db, struct group *grp, int port, bool raise_admin)
{
	struct usergroup *g = NULL;
	bool permit = false;

	if (raise_admin) {
		roles_t roles = get_roles(db, NO_UID, grp->gr_gid);
		permit = roles_has(roles, ROLE_ADMIN) || roles_has(roles, ROLE_ALL_PORTS_USER);
		roles_free(&roles);
		if (permit) {
			/* Group member explicitly admitted to port */
			return true;
		}
	}

	if (usergroup_create(&g, db, grp) != 0) {
		perror("usergroup_create");
		return false;
	}

	permit = usergroup_has_port(g, port);
	usergroup_destroy(&g);

	return (permit);
}

static bool
grp_is_portadmin(xmldb_t *db, struct group *grp)
{
	bool permit = false;

	roles_t roles = get_roles(db, NO_UID, grp->gr_gid);
	permit = roles_has(roles, ROLE_ADMIN) || roles_has(roles, ROLE_PORT_ADMIN);
	roles_free(&roles);
	if (permit) {
		/* Group member explicitly portadmin */
		return true;
	}
	return (permit);
}

static bool
gid_isportallowed(xmldb_t *db, int port, gid_t gid, bool raise_admin)
{
	char *buf = NULL;
	struct group gr, *grp = NULL;
	bool permit = false;

	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (buf == NULL) {
		perror("calloc");
		goto onexit;
	}

	if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0
	    && grp_has_port(db, grp, port, raise_admin)) {
		/* Group admitted on port */
		permit = true;
	}

onexit:
	if (buf != NULL) {
		free(buf);
	}

#ifndef EMBED /* Always let them in when debugging */
	return (true);
#else
	return (permit);
#endif
}

bool
opengear_groups_isportallowed(xmldb_t *db, int port, gid_t gid)
{
	return (gid_isportallowed(db, port, gid, true));
}

bool
opengear_groups_isportvisible(xmldb_t *db, int port, gid_t gid)
{
	return (gid_isportallowed(db, port, gid, false));
}

static bool
gid_isportadmin(xmldb_t *db, gid_t gid)
{
	char *buf = NULL;
	struct group gr, *grp = NULL;
	bool permit = false;

	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (buf == NULL) {
		perror("calloc");
		goto onexit;
	}

	if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0
	    && grp_is_portadmin(db, grp)) {
		/* Group admitted on port */
		permit = true;
	}

onexit:
	if (buf != NULL) {
		free(buf);
	}

#ifndef EMBED /* Always let them in when debugging */
	return (true);
#else
	return (permit);
#endif
}

bool
opengear_groups_isportadmin(xmldb_t *db, gid_t gid)
{
	return (gid_isportadmin(db, gid));
}

static bool
uid_isportallowed_with_perms(int port, bool raise_admin,
	opengear_users_permissions_t perms)
{
	int i;

	if (!check_valid(perms)) {
		goto onfail;
	}
	if (perms->is_root) {
		return true;
	}
	if (perms->user) {
		struct user *u = perms->user;
		if (user_has_port(u, port)) {
			return true;
		}
	}
	for (i = 0; i < perms->groups_size; i++) {
		if (perms->groups[i].roles_allow_all && raise_admin) {
			return true;
		}

		if (perms->groups[i].usergroup && usergroup_has_port(perms->groups[i].usergroup, port)) {
			return true;
		}
	}
onfail:
#ifndef EMBED /* Always let them in when debugging */
	return true;
#else
	return false;
#endif
}

bool
opengear_users_isportallowed_with_perms(int port, opengear_users_permissions_t perms_h)
{
	return uid_isportallowed_with_perms(port, true, perms_h);
}

bool
opengear_users_isportallowed(xmldb_t *db, int port, uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return uid_isportallowed_with_perms(port, true, perms_h);
}

bool
opengear_users_isportvisible(xmldb_t *db, int port, uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return uid_isportallowed_with_perms(port, false, perms_h);
}

static bool
uid_isportadmin_with_perms(
	opengear_users_permissions_t perms)
{
	int i;

	if (!check_valid(perms)) {
		goto onfail;
	}
	if (perms->is_root) {
		return true;
	}
	if (perms->roles_port_admin || perms->roles_admin) {
		return true;
	}

	for (i = 0; i < perms->groups_size; i++) {
		if (perms->groups[i].roles_port_admin || perms->groups[i].roles_admin) {
			return true;
		}
	}
onfail:
#ifndef EMBED /* Always let them in when debugging */
	return true;
#else
	return false;
#endif
}

bool
opengear_users_isportadmin(xmldb_t *db, uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return uid_isportadmin_with_perms(perms_h);
}

bool
opengear_users_isportadmin_with_perms(opengear_users_permissions_t perms_h)
{
	return uid_isportadmin_with_perms(perms_h);
}

static bool
user_hasaddress(struct user *user, const char *address)
{
	struct host *host = NULL;
	TAILQ_FOREACH(host, &user->hostlist, list) {
		if (strcmp(host->address, address) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
usergroup_hasaddress(struct usergroup *group, const char *address)
{
	struct host *host = NULL;
	TAILQ_FOREACH(host, &group->hostlist, list) {
		if (strcmp(host->address, address) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
grp_hasaddress(xmldb_t *db, struct group *grp, const char *address,
	bool raise_admin)
{
	struct usergroup *g = NULL;
	bool permit = false;

	if (raise_admin) {
		roles_t roles = get_roles(db, NO_UID, grp->gr_gid);
		permit = roles_has(roles, ROLE_ADMIN) || roles_has(roles, ROLE_ALL_PORTS_USER);
		roles_free(&roles);
		if (permit) {
			/* Group member explicitly admitted to host */
			return true;
		}
	}

	if (usergroup_create(&g, db, grp) != 0) {
		perror("usergroup_create");
		return false;
	}

	permit = usergroup_hasaddress(g, address);
	usergroup_destroy(&g);

	return (permit);
}

static bool
gid_ishostallowed(xmldb_t *db, const char *address, gid_t gid,
	bool raise_admin)
{
	char *buf = NULL;
	struct group gr, *grp = NULL;
	bool permit = false;

	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (buf == NULL) {
		perror("calloc");
		goto onexit;
	}

	if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0
	    && grp_hasaddress(db, grp, address, raise_admin)) {
		/* Group admitted to host */
		permit = true;
	}

onexit:
	if (buf != NULL) {
		free(buf);
	}

#ifndef EMBED /* Always let them in when debugging */
	return (true);
#else
	return (permit);
#endif
}

bool
opengear_groups_ishostallowed(xmldb_t *db, const char *address, gid_t gid)
{
	return (gid_ishostallowed(db, address, gid, true));
}

bool
opengear_groups_ishostvisible(xmldb_t *db, const char *address, gid_t gid)
{
	return (gid_ishostallowed(db, address, gid, false));
}

static bool
uid_ishostallowed_with_perms(const char *address, bool raise_admin,
	opengear_users_permissions_t perms)
{
	int i;

	if (!check_valid(perms)) {
		goto onfail;
	}
	if (perms->is_root) {
		return true;
	}
	if (perms->user) {
		struct user *u = perms->user;
		if (user_hasaddress(u, address)) {
			return true;
		}
	}
	for (i = 0; i < perms->groups_size; i++) {
		if (perms->groups[i].roles_allow_all && raise_admin) {
			return true;
		}

		if (perms->groups[i].usergroup && usergroup_hasaddress(perms->groups[i].usergroup, address)) {
			return true;
		}
	}
onfail:
#ifndef EMBED /* Always let them in when debugging */
	return true;
#else
	return false;
#endif
}

bool
opengear_users_ishostallowed_with_perms(
	const char *address, opengear_users_permissions_t perms_h)
{
	return uid_ishostallowed_with_perms(address, true, perms_h);
}

bool
opengear_users_ishostallowed(xmldb_t *db, const char *address,
	uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return uid_ishostallowed_with_perms(address, true, perms_h);
}

bool
opengear_users_ishostvisible(xmldb_t *db, const char *address,
	uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return uid_ishostallowed_with_perms(address, false, perms_h);
}

static bool
user_hasrpc(struct user *user, const char *configpath)
{
	struct outlet *outlet = NULL;
	TAILQ_FOREACH(outlet, &user->outletlist, list) {
		if (strncmp(outlet->configpath,
				configpath,
				strlen(configpath)) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
usergroup_hasrpc(struct usergroup *group, const char *configpath)
{
	struct outlet *outlet = NULL;
	TAILQ_FOREACH(outlet, &group->outletlist, list) {
		if (strncmp(outlet->configpath,
				configpath,
				strlen(configpath)) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
grp_hasrpc(xmldb_t *db, struct group *grp, const char *configpath,
	bool raise_admin)
{
	struct usergroup *g = NULL;
	bool permit = false;

	if (raise_admin) {
		roles_t roles = get_roles(db, NO_UID, grp->gr_gid);
		permit = roles_has(roles, ROLE_ADMIN) || roles_has(roles, ROLE_ALL_PORTS_USER);
		roles_free(&roles);
		if (permit) {
			/* Group member explicitly admitted to outlet */
			return true;
		}
	}

	if (usergroup_create(&g, db, grp) != 0) {
		perror("usergroup_create");
		return false;
	}

	permit = usergroup_hasrpc(g, configpath);
	usergroup_destroy(&g);

	return (permit);
}

static bool
user_hasoutlet(struct user *user, const char *configpath)
{
	struct outlet *outlet = NULL;
	TAILQ_FOREACH(outlet, &user->outletlist, list) {
		if (strcmp(outlet->configpath, configpath) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
usergroup_hasoutlet(struct usergroup *group, const char *configpath)
{
	struct outlet *outlet = NULL;
	TAILQ_FOREACH(outlet, &group->outletlist, list) {
		if (strcmp(outlet->configpath, configpath) == 0) {
			return (true);
		}
	}
	return (false);
}

static bool
grp_hasoutlet(xmldb_t *db, struct group *grp, const char *configpath,
	bool raise_admin)
{
	struct usergroup *g = NULL;
	bool permit = false;

	if (raise_admin) {
		roles_t roles = get_roles(db, NO_UID, grp->gr_gid);
		permit = roles_has(roles, ROLE_ADMIN) || roles_has(roles, ROLE_ALL_PORTS_USER);
		roles_free(&roles);
		if (permit) {
			/* Group member explicitly admitted to outlet */
			return true;
		}
	}

	if (usergroup_create(&g, db, grp) != 0) {
		perror("usergroup_create");
		return false;
	}

	permit = usergroup_hasoutlet(g, configpath);
	usergroup_destroy(&g);

	return (permit);
}

static bool
gid_isoutletallowed(xmldb_t *db, const char *configpath, gid_t gid,
	bool raise_admin)
{
	char *buf = NULL;
	struct group gr, *grp = NULL;
	bool permit = false;

	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (buf == NULL) {
		perror("calloc");
		goto onexit;
	}

	if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0
	    && grp_hasoutlet(db, grp, configpath, raise_admin)) {
		/* Group admitted to outlet */
		permit = true;
	}

onexit:
	if (buf != NULL) {
		free(buf);
	}

#ifndef EMBED /* Always let them in when debugging */
	return (true);
#else
	return (permit);
#endif
}

bool
opengear_groups_isoutletallowed(xmldb_t *db, const char *configpath, gid_t gid)
{
	return (gid_isoutletallowed(db, configpath, gid, true));
}

bool
opengear_groups_isoutletvisible(xmldb_t *db, const char *configpath, gid_t gid)
{
	return (gid_isoutletallowed(db, configpath, gid, false));
}

static bool
uid_isoutletallowed_with_perms(const char *configpath, bool raise_admin,
	opengear_users_permissions_t perms)
{
	int i;

	if (!check_valid(perms)) {
		goto onfail;
	}
	if (perms->is_root) {
		return true;
	}
	if (perms->user) {
		struct user *u = perms->user;
		if (user_hasoutlet(u, configpath)) {
			return true;
		}
	}
	for (i = 0; i < perms->groups_size; i++) {
		if (perms->groups[i].roles_allow_all && raise_admin) {
			return true;
		}

		if (perms->groups[i].usergroup && usergroup_hasoutlet(perms->groups[i].usergroup, configpath)) {
			return true;
		}
	}
onfail:
#ifndef EMBED /* Always let them in when debugging */
	return true;
#else
	return false;
#endif
}

static bool
uid_isoutletallowed(xmldb_t *db, const char *configpath,
	uid_t uid, gid_t gid, bool raise_admin)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	bool permit = uid_isoutletallowed_with_perms(configpath, raise_admin, perms_h);
	return (permit);
}

bool
opengear_users_isoutletallowed_with_perms(
	const char *configpath, opengear_users_permissions_t perms_h)
{
	return uid_isoutletallowed_with_perms(configpath, true, perms_h);
}


bool
opengear_users_isoutletallowed(
		xmldb_t *db, const char *configpath, uid_t uid, gid_t gid)
{
	return (uid_isoutletallowed(db, configpath, uid, gid, true));
}

bool
opengear_users_isoutletvisible(
		xmldb_t *db, const char *configpath, uid_t uid, gid_t gid)
{
	return (uid_isoutletallowed(db, configpath, uid, gid, false));
}

static bool
uid_isoutletallowedbyaddress(
		xmldb_t *db, const char *address, unsigned int outletnum,
		uid_t uid, gid_t gid, bool raise_admin)
{
	char id[sizeof("config.sdt.hosts.host9999")];
	char configpath[sizeof("config.sdt.hosts.host9999"
		".power.outlet9999")] = { '\0' };
	size_t i, total;

	total = xmldb_getsize(db, "config.sdt.hosts", "total");
	for (i = 1; i <= total; ++i) {
		const char *haddress = NULL;

		snprintf(id, sizeof(id), "config.sdt.hosts.host%zu", i);
		haddress = xmldb_getstring(db, id, "address");
		if (haddress == NULL || haddress[0] == '\0') {
			continue;
		}
		if (strcmp(address, haddress) == 0) {
			snprintf(configpath, sizeof(configpath),
				"config.sdt.hosts.host%zu.power.outlet%u",
				i, outletnum);
			break;
		}
	}
	if (configpath[0] == '\0') {
		/* Host not found. */
		return false;
	}

	return uid_isoutletallowed(db, configpath, uid, gid, raise_admin);
}

bool
opengear_users_isoutletallowedbyaddress(
		xmldb_t *db, const char *address, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbyaddress(db, address, outletnum,
		uid, gid, true));
}

bool
opengear_users_isoutletvisiblebyaddress(
		xmldb_t *db, const char *address, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbyaddress(db, address, outletnum,
		uid, gid, false));
}

static bool
uid_isoutletallowedbydevice(
		xmldb_t *db, const char *device, unsigned int outletnum,
		uid_t uid, gid_t gid, bool raise_admin)
{
	char id[sizeof("config.ports." OG_MAX_PORTDEV)];
	char configpath[sizeof("config.ports." OG_MAX_PORTDEV
		".power.outlet9999")] = { '\0' };
	size_t i, total;

	total = opengear_serial_count();
	for (i = 1; i <= total; ++i) {
		const char *pdevice = NULL;

		snprintf(id, sizeof(id), "config.ports.port%zu", i);
		pdevice = xmldb_getstring(db, id, "device");
		if (pdevice == NULL || pdevice[0] == '\0') {
			continue;
		}
		if (strcmp(device, pdevice) == 0) {
			snprintf(configpath, sizeof(configpath),
				"config.ports.port%zu.power.outlet%u",
				i, outletnum);
			break;
		}
	}
	if (configpath[0] == '\0') {
		unsigned long int portnum;

		portnum = strtoul(device + strlen("/dev/port"), NULL, 10);
		if (portnum == ULONG_MAX) {
			/* Invalid device. */
			return false;
		}
		snprintf(configpath, sizeof(configpath),
			"config.ports.port%lu.power.outlet%u",
			portnum, outletnum);
	}

	return uid_isoutletallowed(db, configpath, uid, gid, raise_admin);
}

bool
opengear_users_isoutletallowedbydevice(
		xmldb_t *db, const char *device, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbydevice(db, device, outletnum,
		uid, gid, true));
}

bool
opengear_users_isoutletvisiblebydevice(
		xmldb_t *db, const char *device, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbydevice(db, device, outletnum,
		uid, gid, false));
}

static bool
uid_isoutletallowedbyportnum(
		xmldb_t *db, unsigned int portnum, unsigned int outletnum,
		uid_t uid, gid_t gid, bool raise_admin)
{
	char configpath[
		sizeof("config.ports." OG_MAX_PORTDEV ".power.outlet9999")];

	snprintf(configpath, sizeof(configpath),
		"config.ports.port%u.power.outlet%u",
		portnum, outletnum);
	return uid_isoutletallowed(db, configpath, uid, gid, raise_admin);
}

bool
opengear_users_isoutletallowedbyportnum(
		xmldb_t *db, unsigned int portnum, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbyportnum(db, portnum, outletnum,
		uid, gid, true));
}

bool
opengear_users_isoutletvisiblebyportnum(
		xmldb_t *db, unsigned int portnum, unsigned int outletnum,
		uid_t uid, gid_t gid)
{
	return (uid_isoutletallowedbyportnum(db, portnum, outletnum,
		uid, gid, false));
}

static bool
gid_isrpcallowed(xmldb_t *db, const char *configpath, gid_t gid,
	bool raise_admin)
{
	char *buf = NULL;
	struct group gr, *grp = NULL;
	bool permit = false;

	buf = calloc(MAXBUFLEN, sizeof(*buf));
	if (buf == NULL) {
		perror("calloc");
		goto onexit;
	}

	if (getgrgid_r(gid, &gr, buf, MAXBUFLEN, &grp) == 0
	    && grp_hasrpc(db, grp, configpath, raise_admin)) {
		/* Group admitted to RPC */
		permit = true;
	}

onexit:
	if (buf != NULL) {
		free(buf);
	}

#ifndef EMBED /* Always let them in when debugging */
	return (true);
#else
	return (permit);
#endif
}

bool
opengear_groups_isrpcallowed(xmldb_t *db, const char *configpath, gid_t gid)
{
	return (gid_isrpcallowed(db, configpath, gid, true));
}

bool
opengear_groups_isrpcvisible(xmldb_t *db, const char *configpath, gid_t gid)
{
	return (gid_isrpcallowed(db, configpath, gid, false));
}

static bool
isrpc_with_perms(const char *configpath, bool raise_admin,
	opengear_users_permissions_t perms)
{
	int i;

	if (!check_valid(perms)) {
		goto onfail;
	}
	if (perms->is_root) {
		return true;
	}
	if (perms->user) {
		struct user *u = perms->user;
		if (user_hasrpc(u, configpath)) {
			return true;
		}
	}
	for (i = 0; i < perms->groups_size; i++) {
		if (perms->groups[i].roles_allow_all && raise_admin) {
			return true;
		}

		if (perms->groups[i].usergroup && usergroup_hasrpc(perms->groups[i].usergroup, configpath)) {
			return true;
		}
	}
onfail:
#ifndef EMBED /* Always let them in when debugging */
	return true;
#else
	return false;
#endif
}

bool opengear_users_isrpcallowed_with_perms(
	const char *configpath, opengear_users_permissions_t perms_h)
{
	return isrpc_with_perms(configpath, true, perms_h);
}

bool
opengear_users_isrpcallowed(xmldb_t *db, const char *configpath,
	uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return isrpc_with_perms(configpath, true, perms_h);
}

bool
opengear_users_isrpcvisible(xmldb_t *db, const char *configpath,
	uid_t uid, gid_t gid)
{
	opengear_users_permissions_t perms_h = opengear_users_load_permissions(db, uid, gid);
	return isrpc_with_perms(configpath, false, perms_h);
}

int
getNextUid()
{
	FILE *stream = NULL;
	struct passwd* pwp = NULL;
	int *uids = NULL;
	int count = 0;
	int i = 0;
	int j = 0;

	stream = fopen(USER_PASSWDFILE, "r");
	if (stream == NULL) {
		return (USER_MINUID);
	}

	/* Load up the used uids */
	for (pwp = fgetpwent(stream); pwp != NULL; pwp = fgetpwent(stream)) {
		if (pwp->pw_uid >= USER_MINUID) {
			count++;
		}
	}

	if (count == 0) {
		fclose(stream);
		return USER_MINUID;
	}

	rewind(stream);
	uids = calloc(count, sizeof(int));
	if (uids == NULL) {
		fclose(stream);
		return -1;
	}

	for (pwp = fgetpwent(stream); pwp != NULL; pwp = fgetpwent(stream)) {
		if (pwp->pw_uid >= USER_MINUID) {
			uids[i] = pwp->pw_uid;
			i++;
		}
	}

	/* Sort the thing */
	for (i = 0; i < count - 1; i++) {
		for (j = 0; j < count - i - 1; j++) {
			if (uids[j] > uids[j+1]) {
				int k = uids[j+1];
				uids[j+1] = uids[j];
				uids[j] = k;
			}
		}
	}

	if (uids[0] > USER_MINUID) {
		free(uids);
		fclose(stream);
		return USER_MINUID;
	}

	/* Find a gap, or run off of the end */
	for (i = 0; i < count - 1; i++) {
		if (uids[i+1] - uids[i] > 1) {
			j = uids[i] + 1;
			free(uids);
			fclose(stream);
			return j;
		}
	}

	j = uids[count - 1] + 1;
	free(uids);
	fclose(stream);
	return j;
}

/**
 * Get a list of local groups that the user is a member of.
 * This is primarily used by pam_adduser
 */
char ** opengear_users_getlocalgroups(xmldb_t *db, const char *username, size_t *num_groups) {
	const char *tmp = NULL;
	size_t user_idx = 0;
	size_t group_idx = 0;
	size_t array_idx = 0;
	size_t total = 0;
	bool user_found = false;
	char user_ref[sizeof("config.users.user9999")];
	char group_ref[sizeof("groups.group9999")];
	char **groups = NULL;

	*num_groups = 0;

	/* Peruse config for the specified user configuration */
	total = xmldb_getsize(db, "config.users", "total");
	for (user_idx = 1; user_idx <= total; ++user_idx) {
		const char *_user = NULL;
		snprintf(user_ref, sizeof(user_ref) - 1, "config.users.user%zu", user_idx);
		_user = xmldb_getstring(db, user_ref, "username");
		if (_user == NULL || _user[0] == '\0') {
			continue;
		}
		if (strcmp(username, _user) != 0) {
			continue;
		} else {
			user_found = true;
			break;
		}
	}
	if (!user_found) {
		return NULL;
	}

	/* Get the group list for the user */
	tmp = xmldb_getstring(db, user_ref, "groups.total");
	if (tmp == NULL || tmp[0] == '\0') {
		/* We don't have a groups.total. Oh joy. */
		group_idx = 1;
		while (1) {
			const char *_group = NULL;
			snprintf(group_ref, sizeof(group_ref) - 1, "groups.group%zu", group_idx);
			_group = xmldb_getstring(db, user_ref, group_ref);
			if (_group == NULL || _group[0] == '\0') {
				break;
			}
			group_idx++;
		}
		total = group_idx;
	} else {
		total = xmldb_getsize(db, user_ref, "groups.total") + 1;
	}
	/* Allocate the return array - with a null terminator. Our "total" count includes that */
	groups = calloc(total, sizeof(char *));
	if (groups == NULL) {
		return NULL;
	}

	for (group_idx = 1; group_idx <= total; ++group_idx) {
		const char *_group = NULL;
		snprintf(group_ref, sizeof(group_ref) - 1, "groups.group%zu", group_idx);
		_group = xmldb_getstring(db, user_ref, group_ref);
		if (_group == NULL || _group[0] == '\0') {
			break;
		}
		groups[array_idx] = strdup(_group);
		if (groups[array_idx] == NULL) {
			goto free_groups;
		}
		array_idx++;
	}
	*num_groups = array_idx;
	return groups;

free_groups:
	array_idx = 0;
	while (1) {
		if (groups[array_idx] != NULL) {
			free(groups[array_idx]);
			array_idx++;
		} else {
			break;
		}
	}
	free(groups);
	return NULL;
}

/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */
