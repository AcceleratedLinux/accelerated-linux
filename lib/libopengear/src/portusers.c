#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/syslog.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <scew/scew.h>
#include <opengear/serial.h>
#include <opengear/xml.h>
#include <opengear/pmctl.h>
#include <opengear/queue.h>
#include <opengear/portusers.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int get_port_users_cb(void *ptr, const scew_tree *tree)
{
	struct portusers *portusers = (struct portusers *)ptr;
	scew_element *root = scew_tree_root(tree);
	scew_element *port = NULL;
	scew_list *port_entry = NULL;
	scew_list *port_list = root ? scew_element_children(root) : NULL;
	for (port_entry = port_list; port_entry != NULL; port_entry = scew_list_next(port_entry)) {
		port = (scew_element *) scew_list_data(port_entry);
		scew_element *users = scew_element_by_name(port, "users");
		scew_element *number = scew_element_by_name(port, "port");
		scew_element *label = scew_element_by_name(port, "label");
		scew_element *user = NULL;
		const char *labelvalue = NULL;
		int portnum = 0;

		if (number) {
			portnum = scew_xml_int(number);
		}

		if (label) {
			labelvalue = scew_xml_string(label);
		}
		scew_list *user_entry = NULL;
		scew_list *user_list = users ? scew_element_children(users) : NULL;
		for (user_entry = user_list; user_entry != NULL; user_entry = scew_list_next(user_entry)) {
			user = (scew_element *) scew_list_data(user_entry);
			scew_element *name = NULL;
			const char *namevalue = NULL;

			portuser_t *p = calloc(1, sizeof(*p));
			if (!p) {
				break;
			}

			name = scew_element_by_name(user, "name");
			if (name) {
				namevalue = scew_xml_string(name);
				if (namevalue) {
					strncpy(p->name, namevalue, sizeof(p->name) - 1);
				}
			}

			p->port = portnum;
			if (labelvalue) {
				strncpy(p->label, labelvalue, sizeof(p->label) - 1);
			} else {
				snprintf(p->label, sizeof(p->label) - 1, "Port %d", portnum);
			}

			TAILQ_INSERT_TAIL(portusers, p, list);
		}
	}

	return 0;
}

bool opengear_get_port_users(struct portusers *users)
{
	scew_tree *request = scew_tree_create();
	scew_element *root, *elem;
	pmctl_t *pmctl = NULL;
	int ret = -1;

	if (!request) {
		return false;
	}

	root = scew_tree_set_root(request, "command");
	if (!root) {
		goto out;
	}

	elem = scew_element_add(root, "users");
	if (!elem) {
		goto out;
	}

	pmctl = pmctl_open();
	if (!pmctl) {
		goto out;
	}

	ret = pmctl_xml_request(pmctl, request, get_port_users_cb, users);

out:
	if (request) {
		scew_tree_free(request);
	}

	if (pmctl) {
		pmctl_close(pmctl);
	}

	return (ret == 0);
}

/*
 * Return a space separated list of user names that are currently
 * logged in the given serial port
 *
 * NOTE: callers must free it once done with it
 */
char *opengear_get_port_users_for_port(int port)
{
	portusers_t users;
	portuser_t *p;
	char *names = NULL;

	TAILQ_INIT(&users);

	if (!opengear_get_port_users(&users)) {
		return NULL;
	}

	TAILQ_FOREACH(p, &users, list) {
		if (p->port != port || !p->name) {
			continue;
		}

		if (!names) {
			names = strdup(p->name);
			if (!names) {
				break;
			}
		} else {
			/*
			 * Two more bytes for the whitespace and NULL
			 * terminator respectively
			 */
			names = realloc(names, strlen(names) +
					strlen(p->name) + 2);
			if (!names) {
				break;
			}

			names = strcat(names, " ");
			names = strcat(names, p->name);
		}
	}

	while ((p = TAILQ_FIRST(&users)) != NULL) {
		TAILQ_REMOVE(&users, p, list);
		free(p);
	}

	return names;
}

static int disconnect_port_users_cb(void *ptr, const scew_tree *tree)
{
	scew_element *root, *elem;
	const char *value;
	long count = 0;

	if (!ptr) {
		return 0;
	}

	root = scew_tree_root(tree);
	if (root) {
		elem = scew_element_by_name(root, "count");
		if (elem) {
			value = scew_element_contents(elem);
			if (value) {
				count = strtol(value, NULL, 10);
				if (errno != ERANGE && count > 0) {
					*((int *)ptr) += count;
				}
			}
		}
	}

	return 0;
}

int opengear_disconnect_port_users(const char *name, const int port, int *count)
{
	scew_tree *request = scew_tree_create();
	scew_element *root, *elem, *portelem, *userelem;
	pmctl_t *pmctl = NULL;
	char buf[32];
	int ret = -1;

	if (!request) {
		return -1;
	}

	root = scew_tree_set_root(request, "command");
	if (!root) {
		goto out;
	}

	elem = scew_element_add(root, "disconnect");
	if (!elem) {
		goto out;
	}

	portelem = scew_element_add(elem, "port");
	if (!portelem) {
		goto out;
	}

	sprintf(buf, "port%02d", port);
	scew_element_set_contents(portelem, buf);

	if (name) {
		userelem = scew_element_add(elem, "user");
		if (!userelem) {
			goto out;
		}
		scew_element_set_contents(userelem, name);
	}

	pmctl = pmctl_open();
	if (!pmctl) {
		goto out;
	}

	ret = pmctl_xml_request(pmctl, request, disconnect_port_users_cb, count);

out:
	if (request) {
		scew_tree_free(request);
	}

	if (pmctl) {
		pmctl_close(pmctl);
	}

	return ret;
}
