/* xml_config_setup.c - Common functions used by both blackbox and whitebox t-user unit tests
 * Include this file directly inline. */




/*
 * XML Config Setup
 */

/* xml_error - prints XML parsing errors to stderr */
static void xml_error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
}

static char empty_config[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
"<config>\n"
"</config>\n";

/* Return an empty xml database object */
static xmldb_t * setup_empty_config(void) {
	xmldb_t *db = xmldb_open_buffer(empty_config, sizeof(empty_config)-1, xml_error);
	assert(db != NULL);
	return db;
}

static scew_element *find_or_create_child(scew_element *parent, const char *name) {
	scew_element *res = scew_element_by_name(parent, name);
	if (!res) {
		res = scew_element_add(parent, name);
	}
	return res;
}

/* config_role adds config.group.group1.roles.ROLENAME to config */
static void config_role(xmldb_t *db, const char *username, const char *rolename) {
	scew_element *root = (scew_element *)xmldb_root(db);
	scew_element *groups = scew_element_add(root, "groups");
	scew_element *group1 = scew_element_add(groups, "group1");
	xml_element_add_pair(group1, "name", username);
	scew_element *roles = scew_element_add(group1, "roles");
	xml_element_add_pair(roles, "role1", rolename);
	xml_element_add_pair(roles, "total", "1");
	xml_element_add_pair(groups, "total", "1");
}

/* config_set_user1 - Sets config.users.user1 = user1 */
static scew_element *config_set_user1(xmldb_t *db) {
	scew_element *root = (scew_element *)xmldb_root(db);
	scew_element *users = scew_element_add(root, "users");
	scew_element *user1 = scew_element_add(users, "user1");
	xml_element_add_pair(user1, "username", "user1");
	xml_element_add_int(users, "total", 1);
	return user1;
}

/* config_set_user1_ports - Sets config.users.user1.ports list to match boolean
 * array in port_permissions */
static void config_set_user1_ports(xmldb_t *db, bool *port_permissions, int num_ports) {
	scew_element *user1 = config_set_user1(db);
	int i;
	for (i = 0; i < num_ports; i++) {
		char portname[256];
		sprintf(portname, "port%d", i+1);
		xml_element_add_pair(user1, portname, port_permissions[i] ? "true" : "false");
	}
	xml_element_add_int(user1, "total", num_ports);
}

/* config_increment_list_total - helper function to work with lists in config */
static int config_increment_list_total(scew_element *list) {
	scew_element *total = find_or_create_child(list, "total");
	char new_value[64];
	int res = scew_xml_int(total) + 1;
	snprintf(new_value, sizeof(new_value), "%d", res);
	scew_element_set_contents(total, new_value);
	return res;
}

/* config_add_list_item - helper function to add an element to a list in config.
 * The name of the item is determined by reading and incrementing list.total
 * and appending the resulting number to item_name. */
static scew_element *config_add_list_item(scew_element *list, const char *item_name) {
	int item_number = config_increment_list_total(list);
	char itemid[256];
	snprintf(itemid, sizeof(itemid), "%s%d", item_name, item_number);
	return scew_element_add(list, itemid);
}

/* config_add_group - Set config.groups.groupN.name = GROUP where N is
 * determined by the number of existing groups*/
static scew_element * config_add_group(xmldb_t *db, char *group) {
	scew_element *root = (scew_element *)xmldb_root(db);
	scew_element *xgroups = find_or_create_child(root, "groups");
	scew_element *xgroup = config_add_list_item(xgroups, "group");
	xml_element_add_pair(xgroup, "name", group);
	return xgroup;
}

/* config_set_group_ports - Given port ACL, apply it to the named group. Sets
 * config.groups.groupX.port[1..num_ports] where X is the number of the matching group (it
 * adds it if it doesn't exist). */
static void config_set_group_ports(xmldb_t *db, char *group,  bool *port_permissions, int num_ports) {
	scew_element *root = (scew_element *)xmldb_root(db);
	scew_element *xgroup = (scew_element *)scew_xml_search(root, "groups", "name", group);
	if (!xgroup) {
		xgroup = config_add_group(db, group);
	}
	int i;
	for (i = 0; i < num_ports; i++) {
		char portid[256];
		sprintf(portid, "port%d", i+1);
		xml_element_add_pair(xgroup, portid, port_permissions[i] ? "true" : "false");
		config_increment_list_total(xgroup);
	}
}
