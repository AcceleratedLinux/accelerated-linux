#ifndef _OPENGEAR_IFPROBE_H_
#define _OPENGEAR_IFPROBE_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/param.h>

#include <inttypes.h>

#define MAXVENDORLEN	64

#ifdef __cplusplus
extern "C" {
#endif

/** A probed network host. */
typedef struct opengear_ifprobe_host {

	/** The probed host's IP address. */
	struct in_addr ifp_address;

	/** The probed host's hardware address. */
	unsigned char ifp_mac[6];

	/** The probed host's name. */
	char *ifp_hostname;

	/** The probed host's vendor string. */
	char *ifp_vendor;

	/** The next probed host in the list. */
	struct opengear_ifprobe_host *next;

} opengear_ifprobe_host_t;

/**
 * Probe for network connected hosts on all network interfaces,
 * without parsing the results.
 * @return 0 on success or -1 on error.
 */
int opengear_ifprobe_probe(void);

/**
 * Retrieve a list of network connected hosts, probing for hosts
 * only if required.  The caller must free the list of hosts.
 * @param hosts Pointer to receive the head of the list.
 * @return The number of detected hosts or -1 on error.
 */
int opengear_ifprobe_gethosts(opengear_ifprobe_host_t **hosts);

/**
 * Free list of probed hosts.
 * @param hosts Head of the list.
 */
void opengear_ifprobe_freehosts(opengear_ifprobe_host_t *hosts);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_IFPROBE_H_ */
