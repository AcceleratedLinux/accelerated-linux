#ifndef _OPENGEAR_SERIAL_H_
#define _OPENGEAR_SERIAL_H_

#include <inttypes.h>
#include <stdbool.h>

#include <opengear/xmldb.h>
#include <opengear/og_config.h>

#define OG_DEV_SERCON	"/dev/sercon"
#define OG_DEV_NMEA	"/dev/cellnmea01"
#define OG_MAX_PORTS    10000
#define OG_MAX_PORTDEV	"port10000"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Counts the number of serial ports attached to the system.
 */
int opengear_serial_count(void);

int opengear_cascaded_node_count(const scew_element *root);

/**
 * Returns the portnr for OG_DEV_SERCON
 * @returns a port number for OG_DEV_SERCON,
 *          or 0 when there is no serial console.
 */
int opengear_serial_consoleport(void);

/**
 * Tests if the given port is OG_DEV_SERCON
 */
bool opengear_serial_isconsole(int portnr);

/**
 * Tests if the given port is OG_DEV_NMEA
 */
bool opengear_serial_isnmea(int portnr);

/**
 * Returns device name for serial console.
 * @return "sercon" if /dev/sercon is the system console,
 *         or NULL if there is no system console
 */
const char *opengear_serial_consoledevname();

/**
 * Indicates if the serial port appears disconnected.
 * This is possible for USB serial ports, when the USB
 * device is removed.
 */
bool opengear_serial_is_disconnected(int portnr);

/**
 * Returns the descriptive typestring for the port,
 * which describes the interface. The string format is
 * described in /etc/scripts/udev-serial, and is
 * comma-separated.
 * @returns pointer to a new string, or NULL.
 */
char *opengear_serial_get_typedesc(int portnr);

/**
 * Return the system's default label for a port.
 * @returns pointer to new string, or NULL.
 */
char *opengear_serial_get_deflabel(int portnr);

/**
 * Returns the display name of a serial port, embedding a
 * configured label, if any.
 * For example, returns either "Port <n>" or "Port <n> (<label>)".
 * @returns pointer to static storage
 */
const char *opengear_serial_displayname(xmldb_t *db, int portnr);

/**
 * Returns the user-defined label for the port, or NULL.
 */
const char *opengear_serial_label(xmldb_t *db, int portnr);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_SERIAL_H_ */
