#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <scew/scew.h>

#include <opengear/ifprobe.h>
#include <opengear/network.h>
#include <opengear/xml.h>

#define NMAP_EXEC	"/bin/nmap"
#define NETMASK_WIDTH	32

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static size_t
netmask_to_cidr(struct in_addr *netmask)
{
	size_t i;

	/* Network byte order */
	for (i = 0; i < NETMASK_WIDTH; i++) {
		if ((netmask->s_addr & (1L << i)) == 0) {
			break;
		}
	}
	return i;
}

static int
ping_sweep(opengear_net_iface_t *iface, char *filename)
{
	char target[sizeof("255.255.255.255/32")];
	int status = 0;
	pid_t pid = -1;

	if (iface->ni_address.s_addr == 0) {
		/* This interface has no IP, don't scan on him */
		return 1;
	}

	sprintf(target, "%s/%zu",
		inet_ntoa(iface->ni_network),
		netmask_to_cidr(&iface->ni_netmask));

	pid = fork();
	if (pid == 0) {
		/* Detach from the parent */
		int sid = setsid();
		if (sid < 0) {
			_exit(1);
		}
		/* Remap stdout/in/err to /dev/null */
		close(STDIN_FILENO); close(STDOUT_FILENO);
		close(STDERR_FILENO);
		if (dup(open("/dev/null", O_RDWR) == -1)) {
			perror("dup(/dev/null)");
		}
		if (dup(STDOUT_FILENO) == -1) {
			perror("dup(STDOUT_FILENO)");
		}

		execl(NMAP_EXEC, NMAP_EXEC, "-sP", "-oX",
			filename, target, NULL);
		_exit(1);
	} else if (pid == -1) {
		return 1;
	}
	waitpid(pid, &status, 0);
	if (WEXITSTATUS(status)) {
		return 1;
	}
	return 0;
}

static int
parse_nmap_xml(char *filename, opengear_ifprobe_host_t **head, opengear_ifprobe_host_t **tail)
{
	unsigned int mac[6];
	opengear_ifprobe_host_t *host = NULL;
	scew_element *root, *hostelem;
	size_t count = -1;
	size_t i = 0;
	scew_parser *parser = NULL;
	scew_tree *tree = NULL;
	scew_reader *reader = NULL;
	XML_Char const *ipv4addr = NULL, *macaddr = NULL,
		*vendor = NULL, *hostname = NULL;

	parser = scew_parser_create();
	if (parser == NULL) {
		return -1;
	}

	scew_parser_ignore_whitespaces(parser, 1);
	reader = scew_reader_file_create(filename);
	if (reader == NULL) {
		goto parse_error;
	}

	tree = scew_parser_load(parser, reader);
	if (tree == NULL) {
		goto parse_error;
	}
	root = scew_tree_root(tree);
	if (root == NULL) {
		goto parse_error;
	}

	count = 0;
	hostelem = NULL;
	scew_list *host_entry = NULL;
	scew_list *host_list = root ? scew_element_children(root) : NULL;
	for (host_entry = host_list; host_entry != NULL; host_entry = scew_list_next(host_entry)) {
		hostelem = (scew_element *) scew_list_data(host_entry);
		scew_attribute *attr;
		scew_element *elem = NULL;
		ipv4addr = macaddr = vendor = hostname = NULL;

		if (strcmp("host", scew_element_name(hostelem)) != 0) {
			continue;
		}

		scew_list *elem_entry = NULL;
		scew_list *elem_list = hostelem ? scew_element_children(hostelem) : NULL;
		for (elem_entry = elem_list; elem_entry != NULL; elem_entry = scew_list_next(elem_entry)) {
			elem = (scew_element *) scew_list_data(elem_entry);
			if (!strcmp("status", scew_element_name(elem))) {
				attr = scew_element_attribute_by_name(elem, "state");
				if (attr == NULL) {
					break;
				}
				/* Skip hosts that are not up */
				if (strcmp("up", scew_attribute_value(attr))) {
					break;
				}
			} else if (!strcmp("address", scew_element_name(elem))) {
				attr = scew_element_attribute_by_name(elem, "addrtype");
				if (attr == NULL) {
					break;
				}
				if (!strcmp("ipv4", scew_attribute_value(attr))) {
					attr = scew_element_attribute_by_name(elem, "addr");
					if (attr == NULL) {
						break;
					}
					ipv4addr = scew_attribute_value(attr);
				} else if (!strcmp("mac", scew_attribute_value(attr))) {
					attr = scew_element_attribute_by_name(elem, "addr");
					if (attr == NULL) {
						break;
					}
					macaddr = scew_attribute_value(attr);

					/* Vendor is optional */
					attr = scew_element_attribute_by_name(elem, "vendor");
					if (attr != NULL) {
						vendor = scew_attribute_value(attr);
					}
				}
			} else if (!strcmp("hostnames", scew_element_name(elem))) {
				/* Only consider a single hostname */
				scew_element *hn_elem = scew_element_by_name(elem, "hostname");

				/* Hostname is optional */
				if (hn_elem != NULL) {
					attr = scew_element_attribute_by_name(hn_elem, "name");
					if (attr != NULL) {
						hostname = scew_attribute_value(attr);
					}
				}
			}
		}
		if (ipv4addr == NULL || macaddr == NULL) {
			continue;
		}

		host = calloc(1, sizeof(opengear_ifprobe_host_t));
		if (host == NULL) {
			return -1;
		}

		inet_aton(ipv4addr, &(host->ifp_address));
		sscanf(macaddr, "%x:%x:%x:%x:%x:%x",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		for (i = 0; i < sizeof(mac) / sizeof mac[0]; i++) {
			host->ifp_mac[i] = (uint8_t) mac[i];
		}
		if (hostname != NULL) {
			host->ifp_hostname = strdup(hostname);
		} else {
			host->ifp_hostname = NULL;
		}
		if (vendor != NULL) {
			host->ifp_vendor = strdup(vendor);
		} else {
			host->ifp_vendor = NULL;
		}
		host->next = NULL;

		if (*head == NULL) {
			*head = host;
		}
		if (*tail != NULL) {
			(*tail)->next = host;
		}
		*tail = host;

		count++;
	}

parse_error:
	if (parser != NULL) scew_parser_free(parser);
	if (tree != NULL) scew_tree_free(tree);
	if (reader != NULL) scew_reader_free(reader);
	return count;
}

int
opengear_ifprobe_probe()
{
	opengear_net_iface_t* ifaces;
	char filename[sizeof("/var/run/ifprobe-.xml") + IF_NAMESIZE];
	int ifcount, i;
	int retval = 0;

	ifaces = (opengear_net_iface_t *) calloc(OG_NET_MAX_INTERFACES,
		sizeof(*ifaces));
	if (ifaces == NULL) {
		return (-1);
	}

	ifcount = opengear_network_getinterfaces(
		ifaces, OG_NET_MAX_INTERFACES,
		OG_NET_IFACE_PHYSICAL | OG_NET_IFACE_ALIAS);
	if (ifcount < 0) {
		free(ifaces);
		return (-1);
	}

	for (i = 0; i < ifcount; ++i) {
		sprintf(filename, "/var/run/ifprobe-%s.xml", ifaces[i].ni_name);
		if (ping_sweep(&ifaces[i], filename) != 0) {
			retval = -1;
		}
	}
	free(ifaces);
	return (retval);
}

void
opengear_ifprobe_freehosts(opengear_ifprobe_host_t *hosts)
{
	opengear_ifprobe_host_t *host = hosts;
	opengear_ifprobe_host_t *h = NULL;
	while (host != NULL) {
		if (host->ifp_hostname) {
			free(host->ifp_hostname);
		}
		if (host->ifp_vendor) {
			free(host->ifp_vendor);
		}
		h = host;
		host = h->next;
		if (h != NULL) {
			free(h);
		}
	}
}

int
opengear_ifprobe_gethosts(opengear_ifprobe_host_t **hosts)
{
	opengear_net_iface_t* ifaces;
	opengear_ifprobe_host_t *head = NULL;
	opengear_ifprobe_host_t *tail = NULL;
	char filename[sizeof("/var/run/ifprobe-.xml") + IF_NAMESIZE];
	int count = 0;
	int ifcount;
	int hostcount = 0;
	int i;

	ifaces = (opengear_net_iface_t *) calloc(OG_NET_MAX_INTERFACES,
		sizeof(*ifaces));
	if (ifaces == NULL) {
		return (-1);
	}

	ifcount = opengear_network_getinterfaces(
		ifaces, OG_NET_MAX_INTERFACES,
		OG_NET_IFACE_PHYSICAL | OG_NET_IFACE_ALIAS);
	if (ifcount < 0) {
		free(ifaces);
		return (-1);
	}

	hostcount = 0;
	for (i = 0; i < ifcount; ++i) {
		struct stat statbuf;

		sprintf(filename, "/var/run/ifprobe-%s.xml", ifaces[i].ni_name);
		if (stat(filename, &statbuf) != 0) {
			if (ping_sweep(&ifaces[i], filename) != 0) {
				/* Try the next interface */
				continue;
			}
		}
		count = parse_nmap_xml(filename, &head, &tail);
		if (count < 0) {
			opengear_ifprobe_freehosts(head);
			free(ifaces);
			return (-1);
		}
		hostcount += count;
	}
	*hosts = head;

	free(ifaces);
	return hostcount;
}
