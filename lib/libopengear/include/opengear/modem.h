#ifndef _OPENGEAR_MODEM_H_
#define _OPENGEAR_MODEM_H_

#include <net/if.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <inttypes.h>

#define OG_MODEM_MAX		64
#define OG_MODEM_DEV_INTERNAL	"/dev/modem01"
#define OG_MODEM_DEV_PCMCIA	"/dev/pcmodem01"
#define OG_MODEM_DEV_USB	"/dev/usbmodem01"
#define OG_MODEM_DEV_CELL	"/dev/cellmodem01"
#define OG_COMMAND_DEV_CELL	"/dev/cellcommand01"

#define M_DEV_SIZE		256

#ifdef __cplusplus
extern "C" {
#endif

/** Modem type */
typedef enum {
	OG_MODEM_PORT_DB9	= 1 << 0,
	OG_MODEM_PORT_INTERNAL	= 1 << 1,
	OG_MODEM_PORT_PCMCIA 	= 1 << 2,
	OG_MODEM_PORT_USB	= 1 << 3,
	OG_MODEM_PORT_CELL	= 1 << 4,
	OG_MODEM_PORT_VIRTUAL	= 1 << 5
} opengear_modem_flags_t;

/** A modem */
typedef struct opengear_modem {

	/** The modem's primary device name */
	char m_data_dev[M_DEV_SIZE];

	/** The modem's secondary device name */
	char m_cmd_dev[M_DEV_SIZE];

	/** The modem's type */
	opengear_modem_flags_t m_flags;

} opengear_modem_t;

/**
 * Retrieve a list of modems
 * @param modems The preallocated buffer
 * @param len The size of the preallocated buffer
 * @return The number of modems or -1 on error
 */
int opengear_modem_getmodems(opengear_modem_t *modems, size_t len);

/**
 * Retrieve the number of modems
 * @return The number of modems found or -1 on error
 */
int opengear_modem_count(void);

/**
 * Retrieve a human readable title for the modem
 * @return Pointer to the human readable title
 */
const char *opengear_modem_title(const opengear_modem_t *modem);

/**
 * Retrieve the output of a given AT command to the modem command device.
 * @param command The AT command to send
 * @param expect The AT command response terminator ro expect.
 * @param buf The user provided buffer to install the reply in.
 * @param len The size of the given buffer.
 * @param timeout The timeout in seconds to await a reply.
 */
int opengear_modem_getatresponse(
	const char *command, const char *expect,
	char *buf, size_t len, time_t timeout);

/**
 * Retrieve the cellmodem bearer interface name
 * @return The bearer interface name.
 */
const char *opengear_modem_getcelliface(void);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_MODEM_H_ */
