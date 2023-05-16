#ifndef _OPENGEAR_NETWORK_H_
#define _OPENGEAR_NETWORK_H_

#include <net/if.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OG_NET_SWITCH_EXEC		"/etc/scripts/switch-config"
#define OG_NET_MAX_INTERFACES 		64

/** The various types of interfaces. */
typedef enum {
	OG_NET_IFACE_NONE 	= (2 << 0),
	OG_NET_IFACE_PPP  	= (2 << 1),
	OG_NET_IFACE_PHYSICAL	= (2 << 2),
	OG_NET_IFACE_LOOP	= (2 << 3),
	OG_NET_IFACE_ALIAS	= (2 << 4),
	OG_NET_IFACE_ETH	= (2 << 5),
	OG_NET_IFACE_WWAN	= (2 << 6),
	OG_NET_IFACE_AGGR	= (2 << 7),
	OG_NET_IFACE_GBIT	= (2 << 8)
} opengear_net_iface_type_t;

typedef enum {
	OG_NET_NAMING_ASIX		= (1 << 0),
	OG_NET_NAMING_VLAN_SWITCH	= (1 << 1),
	OG_NET_NAMING_MVL88E6390	= (1 << 2)
} opengear_net_naming_type_t;

/** A network interface. */
typedef struct opengear_net_iface {

	/** The interfaces name. */
	char ni_name[IF_NAMESIZE];

	/** The interfaces type. */
	opengear_net_iface_type_t ni_type;

	/** The interfaces MAC address. */
	unsigned char ni_mac[6];

	/** The interfaces current IP address. */
	struct in_addr ni_address;

	/** The current subnet mask. */
	struct in_addr ni_netmask;

	/** The current network address. */
	struct in_addr ni_network;

	/** The current broadcast address. */
	struct in_addr ni_broadcast;

	/** The current IPv6 address. */
	char ni_ipv6global[sizeof("0000:0000:0000:0000:0000:0000:0000:0000")];

	/** The current IPv6 address. */
	char ni_ipv6local[sizeof("0000:0000:0000:0000:0000:0000:0000:0000")];

	/** The current IPv6 address. */
	char ni_ipv6loop[sizeof("0000:0000:0000:0000:0000:0000:0000:0000")];

} opengear_net_iface_t;

/**
 * Retrieve a list of interfaces.
 * @param interfacesp The preallocated buffer.
 * @param len The size of the preallocated buffer.
 * @param flags The type of interfaces to get (eg IFF_POINTTOPOINT)
 * @return The number of network interfaces or -1 on error.
 */
int opengear_network_getinterfaces(
		opengear_net_iface_t *interfaces, size_t len,
		uint32_t flags);

/**
 * Retrieve the number of network interfaces.
 * @param flags The types of network devices to count.
 * @return The number of interfaces found or -1 on error.
 */
int opengear_network_count(uint32_t flags);

/**
 * Retrieve an interface name by index.
 * @param index The number of this interface.
 * @param buf The client provided buffer to store the interface name in.
 * @param len The size of the buffer.
 * @param flags The flags which describe which interfaces to consider.
 * @return 0 on success otherwise -1.
 */
int opengear_network_ifname(
		size_t index, char *buf, size_t len, uint32_t flags);

/**
 * Retrieve the human readable name for an interface.
 * @param ifname The unique interface name.
 * @param buf Storage for readable name.
 * @param len The size of storage provided.
 * @flags flags The flags which can change how interfaces are named
 * @return A pointer to the human readable name or NULL on failure.
 */
const char * opengear_network_iftitle(
	const char *ifname, char *buf, size_t len, uint32_t flags);

/**
 * Retrieve the interface flags that are used by a few other
 * libopengear networking routines.
 * @return The flags to be passed around.
 */
uint32_t opengear_network_getflags(void);

/**
 * Retrieve the config alias for an interface.
 * @param ifname The unique interface name.
 * @param buf Storage for readable name.
 * @param len The size of storage provided.
 * @param flags to modify the behaviour
 * @return A pointer to the config alias or NULL on failure.
 */
const char * opengear_network_ifalias(
	const char *ifname, char *buf, size_t len, uint32_t flags);


/**
 * Given an interface name, retrieve the IP address
 * @param name The unique interface name.
 * @param buf Storage for ip address
 * @param len The size of storage provided.
 * @return 1 on failure, zero on success
 */
int opengear_network_getifaddr( const char * name, char * buf, size_t len );

/**
 * Given an interface name, retrieve the MAC address
 * @param name The unique interface name.
 * @param buf Storage for ip address
 * @param len The size of storage provided.
 * @return 1 on failure, zero on success
 */
int opengear_network_getifmac(const char *name, char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_NETWORK_H_ */
