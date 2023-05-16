#define _DEFAULT_SOURCE		/* for struct ifreq */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#if defined(EMBED) && !defined(NGCS)
#include <config/autoconf.h>
#endif

#include <opengear/network.h>
#include <opengear/og_config.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define OPENGEAR_4002_HAS_EXTRA_INTERFACE 1

#define MAXLINELEN 128

static bool
is_dupe(char *ifname, opengear_net_iface_t *interfaces, size_t len)
{
	size_t i;
	opengear_net_iface_t *iface;

	for (i = 0; i < len; ++i) {
		iface = &interfaces[i];
		if (strcmp(iface->ni_name, ifname) == 0) {
			return true;
		}
	}
	return false;
}

static bool
is_alias(char *ifname)
{
	return (strchr(ifname, ':') != NULL);
}

static bool
is_loop(char *ifname)
{
	return (strcmp(ifname, "lo") == 0);
}

static bool
is_phys(char *ifname)
{
	return (strncmp(ifname, "eth", sizeof("eth") - 1) == 0
		|| strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0
		|| strncmp(ifname, "switch", sizeof("switch") - 1) == 0);
}

static bool
is_eth(char *ifname)
{
	return (strncmp(ifname, "eth", sizeof("eth") - 1) == 0
		&& (strchr(ifname, ':') == NULL)
		&& (strchr(ifname, '.') == NULL));
}

static bool
is_ppp(char *ifname)
{
	return (strncmp(ifname, "ppp", sizeof("ppp") - 1) == 0
		|| strncmp(ifname, "dialout", sizeof("dialout") - 1) == 0);
}

static bool
is_wwan(char *ifname)
{
	return (strncmp(ifname, "wwan", sizeof("wwan") - 1) == 0);
}

static bool
is_aggr(char *ifname)
{
	return (strncmp(ifname, "br", sizeof("br") - 1) == 0
		|| strncmp(ifname, "bond", sizeof("bond") - 1) == 0);
}

#ifdef HAVE_GIGABIT
static bool
is_gbit(char *ifname)
{
	/* Products which are 10/100/1000 capable. */
	if (strncmp(ifname, "eth", sizeof("eth") - 1) == 0) {
		return (true);
	}
	return (false);
}
#endif

static uint32_t
match_type(char *ifname, uint32_t flags)
{
	struct stat stat_buf;
	uint32_t rv = OG_NET_IFACE_NONE;

	if (stat("/var/run/.switch/ethernet_is_single", &stat_buf ) == 0) {
		if (strcmp(ifname, "eth1") == 0) {
			return rv;
		}
	}

	if (flags & OG_NET_IFACE_ETH) {
		if (is_eth(ifname)) {
			rv |= OG_NET_IFACE_ETH;
		}
	}
	if (flags & OG_NET_IFACE_ALIAS) {
		if (is_alias(ifname)) {
			rv |= OG_NET_IFACE_ALIAS;
		}
	}
	if (flags & OG_NET_IFACE_LOOP) {
		if (is_loop(ifname)) {
			rv |= OG_NET_IFACE_LOOP;
		}
	}
	if (flags & OG_NET_IFACE_PHYSICAL) {
		if (is_phys(ifname)) {
			rv |= OG_NET_IFACE_PHYSICAL;
		}
	}
	if (flags & OG_NET_IFACE_PPP) {
		if (is_ppp(ifname)) {
			rv |= OG_NET_IFACE_PPP;
		}
	}
	if (flags & OG_NET_IFACE_WWAN) {
		if (is_wwan(ifname)) {
			rv |= OG_NET_IFACE_WWAN;
		}
	}
	if (flags & OG_NET_IFACE_AGGR) {
		if (is_aggr(ifname)) {
			rv |= OG_NET_IFACE_AGGR;
		}
	}
#ifdef HAVE_GIGABIT
	if (flags & OG_NET_IFACE_GBIT) {
		if (is_gbit(ifname)) {
			rv |= OG_NET_IFACE_GBIT;
		}
	}
#endif

	return rv;
}

/* Converts a 32-digit hex string into a 16-octet IPv6 address */
static struct in6_addr
hex_to_ip6(const char src[static 32])
{
	struct in6_addr ret;
	unsigned int i;

	for (i = 0; i < 32; i += 2)
	    sscanf(&src[i], "%2hhx", &ret.s6_addr[i/2]);
	return ret;
}

typedef enum {
	NET_IPv6_NONE 	= 0x0,
	NET_IPv6_GLOBAL = 0x1,
	NET_IPv6_LOCAL 	= 0x2,
	NET_IPv6_LOOP 	= 0x4
} net_ipv6_t;

static int
opengear_network_getinterfaceipv6(
		const char *ifname, char *ipv6buf, size_t ipv6len,
		unsigned int flags)
{
	char line[MAXLINELEN];
	FILE *fp = NULL;
	int found = 0;
	unsigned int fl = 0;
	struct in6_addr addr;
	char *addrstr;

	if ((fp = fopen("/proc/net/if_inet6", "r")) == (FILE *) 0) {
		return (-1);
	}

	while (fgets(line, MAXLINELEN, fp) != NULL) {

		/* is it the given interface name */
		if (strstr(line, ifname) == (char *) 0) {
			continue;
		}

		/* get the interface type flag */
		if (sscanf(line, "%*32x %*02x %*02x %02x", &fl) != 1) {
			continue;
		}

		addr = hex_to_ip6(line);
		addrstr = inet_ntop(AF_INET6, &addr, ipv6buf, ipv6len);
		if (!addrstr) {
			continue;
		}

		/*
		 * if the first nibble of flag is 0x00; then it is global IPv6
		 * address
		 */
		if ((fl & 0xF0) == 0x00) {
			if ((flags & NET_IPv6_GLOBAL) == NET_IPv6_GLOBAL) {
				found = 1;
				break;
			}

		/*
		 * if the first nible of flag is 0x20; then it is link local
		 * IPv6 address
		 */
		} else if ((fl & 0xF0) == 0x20) {
			if ((flags & NET_IPv6_LOCAL) == NET_IPv6_LOCAL) {
				found = 1;
				break;
			}

		/*
		 * if the first nibble of flag is 0x10; then
		 * it is loopback IPv6 address
		 */
		} else if ((fl & 0xF0) == 0x10) {
			if ((flags & NET_IPv6_LOOP) == NET_IPv6_LOOP) {
				found = 1;
				break;
			}
		}
	}

	fclose(fp);

	if (!found) {
		return (-1);
	}

	return (0);
}

int
opengear_network_getinterfaces(
	opengear_net_iface_t *interfaces, size_t len, uint32_t flags)
{
	char ipv6[sizeof("0000:0000:0000:0000:0000:0000:0000:0000")];
	int s;
	struct ifreq *ifr;
	struct ifconf ifc;
	FILE *fp = NULL;
	char *line = NULL;
	size_t count = 0;
	struct ifreq* buf = calloc(OG_NET_MAX_INTERFACES, sizeof(*buf));
	if (buf == NULL) {
		return (-1);
	}

	/* Setup socket for ioctl() */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		free(buf);
		return (-1);
	}

	ifc.ifc_len = OG_NET_MAX_INTERFACES * sizeof(*buf);
	ifc.ifc_req = NULL;
	ifc.ifc_buf = (char*) buf;

	/* Request list of configured interfaces from kernel */
	if (ioctl(s, SIOCGIFCONF, (caddr_t) &ifc) != 0) {
		free(buf);
		close(s);
		return (-1);
	}

	/* Iterate through interface list */
  	for (ifr = ifc.ifc_req;
			count < len &&
			(char *) ifr < (char *) ifc.ifc_req + ifc.ifc_len;
			++ifr) {
		opengear_net_iface_t *iface = &interfaces[count];
		opengear_net_iface_type_t type;
		struct sockaddr_in *addr = NULL;

		type = match_type(ifr->ifr_name, flags);
		if (type == OG_NET_IFACE_NONE) {
			continue;
		}
		if (is_dupe(ifr->ifr_name, interfaces, count)) {
			continue;
		}

		iface->ni_type = type;
		strncpy(iface->ni_name, ifr->ifr_name,
				sizeof(iface->ni_name));

		/* Get IP address */
		if (ioctl(s, SIOCGIFADDR, ifr) == 0) {
			addr = (struct sockaddr_in *) &ifr->ifr_addr;
			memcpy(&iface->ni_address,
				&addr->sin_addr,
				sizeof(struct in_addr));
		}

		/* Get subnet mask */
		if (ioctl(s, SIOCGIFNETMASK, ifr) == 0) {
			addr = (struct sockaddr_in *) &ifr->ifr_netmask;
			memcpy(&iface->ni_netmask,
				&addr->sin_addr,
				sizeof(struct in_addr));
		}

		/* Get broadcast address */
		if (ioctl(s, SIOCGIFBRDADDR, ifr) == 0) {
			addr = (struct sockaddr_in *)
				&ifr->ifr_broadaddr;
			memcpy(&iface->ni_broadcast,
				&addr->sin_addr,
				sizeof(struct in_addr));
		}

		/* Get MAC address */
		if (ioctl(s, SIOCGIFHWADDR, ifr) == 0) {
			unsigned char *mac = (unsigned char *)
				ifr->ifr_hwaddr.sa_data;
			memcpy(&iface->ni_mac, mac, sizeof(iface->ni_mac));
		}

		/* Get the network address */
		iface->ni_network.s_addr =
			(iface->ni_netmask.s_addr &
			 iface->ni_address.s_addr);

		/* Get IPv6 global address. */
		memset(ipv6, '\0', sizeof(ipv6));
		if (opengear_network_getinterfaceipv6(
				iface->ni_name, ipv6, sizeof(ipv6),
				NET_IPv6_GLOBAL) == 0) {
			strncpy(iface->ni_ipv6global, ipv6,
				sizeof(iface->ni_ipv6global));
		}

		/* Get IPv6 link local address. */
		memset(ipv6, '\0', sizeof(ipv6));
		if (opengear_network_getinterfaceipv6(
				iface->ni_name, ipv6, sizeof(ipv6),
				NET_IPv6_LOCAL) == 0) {
			strncpy(iface->ni_ipv6local, ipv6,
				sizeof(iface->ni_ipv6local));
		}

		/* Get IPv6 loopback address. */
		memset(ipv6, '\0', sizeof(ipv6));
		if (opengear_network_getinterfaceipv6(
				iface->ni_name, ipv6, sizeof(ipv6),
				NET_IPv6_LOOP) == 0) {
			strncpy(iface->ni_ipv6loop, ipv6,
				sizeof(iface->ni_ipv6loop));
		}

		count++;
	}

	free(buf);
	close(s);

	/* Read unconfigured interfaces from procfs */
	line = calloc(MAXLINELEN, sizeof(*line));
	if (line == NULL) {
		return (-1);
	}
	fp = fopen("/proc/net/dev", "r");
	if (fp == NULL) {
		free(line);
		return (-1);
	}

	/* Eat the first 2 lines which is just column titles */
	(void) fgets(line, MAXLINELEN, fp);
	(void) fgets(line, MAXLINELEN, fp);

	/* Read the device file line by line */
	while (count < len && fgets(line, MAXLINELEN, fp) != NULL) {
		opengear_net_iface_t *iface = &interfaces[count];
		opengear_net_iface_type_t type;

		/*
		 * Eat all the space from the start of a line.
		 *FIXME: strtok() is bad.
		 */
		char *ifname = strtok(line, " :");

		type = match_type(ifname, flags);
		if (type == OG_NET_IFACE_NONE) {
			continue;
		}
		if (is_dupe(ifname, interfaces, count)) {
			continue;
		}

		iface->ni_type = type;
		strncpy(iface->ni_name, ifname,
				sizeof(iface->ni_name));

		count++;
	}

	free(line);
	fclose(fp);

	return (count);
}

int
opengear_network_count(uint32_t flags)
{
	int count = 0;
	opengear_net_iface_t *tmp =
		calloc(OG_NET_MAX_INTERFACES, sizeof(*tmp));
	if (tmp == NULL) {
		return (0);
	}
	count = opengear_network_getinterfaces(tmp, OG_NET_MAX_INTERFACES, flags);
	free(tmp);
	return (count);
}

int
opengear_network_ifname(size_t index, char *buf, size_t len, uint32_t flags)
{
	size_t count = 0;
	int rc = 0;

	opengear_net_iface_t *tmp = calloc(OG_NET_MAX_INTERFACES, sizeof(*tmp));
	if (tmp == NULL) {
		rc = -1;
		goto onexit;
	}
	count = opengear_network_getinterfaces(tmp, OG_NET_MAX_INTERFACES, flags);
	if (index < 1 || index > count) {
		rc = -1;
		goto onexit;
	}

	strncpy(buf, tmp[index - 1].ni_name, len);

onexit:
	if (tmp != NULL) {
		free(tmp);
	}
	return (rc);
}

const char *
opengear_network_iftitle(const char *ifname, char *buf, size_t len, uint32_t flags)
{
	memset(buf, '\0', len);
#ifdef CONFIG_PRODUCT
	if (flags & OG_NET_NAMING_VLAN_SWITCH) {
		if (strcmp(ifname, "eth0") == 0) {
			strncpy(buf, "Network", len);
		} else if (strcmp(ifname, "eth1.2") == 0) {
			strncpy(buf, "Management LAN", len);
		} else if (strcmp(ifname, "eth1.3") == 0) {
			strncpy(buf, "Out-of-Band/Failover", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "Wireless Network", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "Switch Base", len);
		} else if (strcmp(ifname, "br0") == 0) {
			strncpy(buf, "Network (Bridged)", len);
		} else if (strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "Network (Bonded)", len);
		} else {
			return NULL;
		}
	} else if (flags & OG_NET_NAMING_ASIX) {
		if (strcmp(ifname, "eth0") == 0) {
			strncpy(buf, "Network", len);
		} else if (strcmp(ifname, "switch0") == 0) {
			strncpy(buf, "Management LAN", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "Out-of-Band/Failover", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "Wireless Network", len);
		} else if (strcmp(ifname, "br0") == 0) {
			strncpy(buf, "Network (Bridged)", len);
		} else if (strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "Network (Bonded)", len);
		} else {
			return NULL;
		}
	} else if (flags & OG_NET_NAMING_MVL88E6390) {
		if (strcmp(ifname, "eth0") == 0) {
			strncpy(buf, "Network", len);
		} else if (strcmp(ifname, "eth2") == 0) {
			strncpy(buf, "Management LAN", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "Out-of-Band/Failover", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "Wireless Network", len);
		} else if (strcmp(ifname, "br0") == 0) {
			strncpy(buf, "Network (Bridged)", len);
		} else if (strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "Network (Bonded)", len);
		} else {
			return NULL;
		}
	} else {
		if (strcmp(ifname, "eth0") == 0) {
			strncpy(buf, "Network", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "Management LAN", len);
		} else if (strcmp(ifname, "eth1:0") == 0) {
			strncpy(buf, "Out-of-Band/Failover", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "Wireless Network", len);
		} else if (strcmp(ifname, "br0") == 0) {
			strncpy(buf, "Network (Bridged)", len);
		} else if (strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "Network (Bonded)", len);
		} else {
			return NULL;
		}
	}
#endif
	return buf;
}

uint32_t
opengear_network_getflags(void)
{
	uint32_t interface_flags = 0;
	struct stat stat_buf;

	if (stat("/var/run/.switch/switch_is_asix", &stat_buf ) == 0) {
		interface_flags |= OG_NET_NAMING_ASIX;
	}

	if (stat("/var/run/.switch/switch_is_ksz8895", &stat_buf ) == 0) {
		interface_flags |= OG_NET_NAMING_VLAN_SWITCH;
	}

	if (stat("/var/run/.switch/switch_is_mvl88e6390", &stat_buf ) == 0) {
		interface_flags |= OG_NET_NAMING_MVL88E6390;
	}

	return interface_flags;
}

const char *
opengear_network_ifalias(const char *ifname, char *buf, size_t len, uint32_t flags)
{
	memset(buf, '\0', len);
#ifdef CONFIG_PRODUCT
	if (flags & OG_NET_NAMING_VLAN_SWITCH) {
		if (strcmp(ifname, "eth0") == 0 || strcmp(ifname, "br0") == 0 || strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "wan", len);
		} else if (strcmp(ifname, "eth1.2") == 0) {
			strncpy(buf, "lan", len);
		} else if (strcmp(ifname, "eth1.3") == 0) {
			strncpy(buf, "oobfo", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "wlan", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "swbase", len);
		} else {
			return NULL;
		}
	} else if (flags & OG_NET_NAMING_ASIX) {
		if (strcmp(ifname, "eth0") == 0 || strcmp(ifname, "br0") == 0 || strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "wan", len);
		} else if (strcmp(ifname, "switch0") == 0) {
			strncpy(buf, "lan", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "oobfo", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "wlan", len);
		} else {
			return NULL;
		}
	} else if (flags & OG_NET_NAMING_MVL88E6390) {
		if (strcmp(ifname, "eth0") == 0 || strcmp(ifname, "br0") == 0 || strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "wan", len);
		} else if (strcmp(ifname, "eth2") == 0) {
			strncpy(buf, "lan", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "oobfo", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "wlan", len);
		} else {
			return NULL;
		}
	} else {
		if (strcmp(ifname, "eth0") == 0 || strcmp(ifname, "br0") == 0 || strcmp(ifname, "bond0") == 0) {
			strncpy(buf, "wan", len);
		} else if (strcmp(ifname, "eth1") == 0) {
			strncpy(buf, "lan", len);
		} else if (strcmp(ifname, "eth1:0") == 0) {
			strncpy(buf, "oobfo", len);
		} else if (strncmp(ifname, "wlan", sizeof("wlan") - 1) == 0) {
			strncpy(buf, "wlan", len);
		} else {
			return NULL;
		}
	}
#endif
	return buf;
}

int
opengear_network_getifaddr(const char *name, char *buf, size_t len)
{
	char *dev = NULL;
	char *cp = NULL;
	int s = -1;
	int retval = 1;
	int i = 1;

	dev = strdup(name);
	if (!dev) {
		return 1;
	}

	cp = strstr(dev, ":");
	if (cp) {
		*cp = '\0';
	}

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (s == -1) {
		free(dev);
		return 1;
	}

	for ( i = 1; ; i++) {
		struct ifreq ifr;
		struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

		ifr.ifr_ifindex = i;
		if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
			break;

		if( strcmp( dev, ifr.ifr_name ) )
			continue;

		strcpy( ifr.ifr_name, name );
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
			continue;

		strncpy( buf, inet_ntoa( sin->sin_addr), len );
		retval = 0;
		break;
	}

	free(dev);
	close(s);
	return retval;
}

int
opengear_network_getifmac(const char *name, char *buf, size_t len)
{
	struct ifreq ifr;
	char *p;
	int s = -1, retval = 1;

	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), name);
	if ((p = strstr(ifr.ifr_name, ":")) != NULL) {
		*p = '\0';
	}

  	if ((s = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
		if (ioctl(s, SIOCGIFHWADDR, (char *)&ifr) != -1) {
			unsigned char *mac = (unsigned char *)
				ifr.ifr_hwaddr.sa_data;
			snprintf(buf, len, "%02X:%02X:%02X:%02X:%02X:%02X",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			retval = 0;
		}
		close(s);
	}
	return retval;
}
