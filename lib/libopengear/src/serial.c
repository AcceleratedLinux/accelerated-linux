#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <opengear/serial.h>
#include <opengear/xml.h>
#include <opengear/libinfod.h>


#ifdef EMBED

#define OG_SER_DEV_DIR "/dev/serial/by-opengear-id"
#define OG_SER_PATH	"/var/run/serial-ports"
#define OG_SER_LINELEN	256

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/**
 * Determine number of serial ports via udev/mdev
 * @return The serial port count.
 */
static int
opengear_serial_count_dev_dir(void)
{
	DIR *dir = opendir(OG_SER_DEV_DIR);
	struct dirent *entry = NULL;
	int count = 0;

	if (!dir) {
		return count;
	}

	for (entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
		if (strncmp(entry->d_name, "port", 4) == 0) {
			count++;
		}
	}
	closedir(dir);
	return (count);
}

int
opengear_serial_count(void)
{
	struct stat st;
	FILE *fp = NULL;
	static size_t nports = 0;
	char *line = NULL;

	if (stat(OG_SER_DEV_DIR, &st) == 0) {
		return (opengear_serial_count_dev_dir());
	}

	if (nports > 0) {
		return ((int) nports);
	}

	fp = fopen(OG_SER_PATH, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open " OG_SER_PATH ": %s\n",
				strerror(errno));
		return 0;
	}

	line = calloc(OG_SER_LINELEN, sizeof(*line));
	if (line == NULL) {
		fprintf(stderr, "Failed to allocate line buffer: %s\n",
			strerror(errno));
		fclose(fp);
		return 0;
	}

	while (fgets(line, OG_SER_LINELEN, fp) != NULL) {
		if (strncmp(line, "port", 4) == 0)
			nports++;
		memset(line, '\0', OG_SER_LINELEN);
	}

	fclose(fp);

	free(line);
	return ((int) nports);
}
#else
int
opengear_serial_count(void)
{
	return 8;
}
#endif

int
opengear_cascaded_node_count(const scew_element *root)
{
	const scew_element *tmp = root;
	scew_element *elem = NULL;
	int count = 0;
	int total;
	int i;

	if (tmp == NULL) {
		return 0;
	}

	tmp = scew_xml_subtree(tmp, "cascade.nodes");
	if (tmp == NULL) {
		return 0;
	}

	elem = scew_element_by_name(tmp, "total");

	total = scew_xml_int(elem);

	for (i = 1;i <= total; i++) {
		char slaveId[sizeof("node99999")];
		int ports = 0;
		scew_element *slave = NULL;

		snprintf(slaveId, sizeof(slaveId), "node%d", i);
		slave = scew_element_by_name(tmp, slaveId);
		if( slave == NULL ) {
			continue;
		}
		elem = scew_element_by_name(slave, "ports");
		ports = scew_xml_int(elem);

		count += ports;
	}
	return count;
}

/* Backward compatibility for sources using old name */
int opengear_slave_count(const scew_element *root)
	__attribute__((alias("opengear_cascaded_node_count")));

int
opengear_serial_consoleport(void)
{
	/*
	 * Only the builtin serial can be console, but it is sometimes also
	 * mapped as port 1
	 */
	return opengear_serial_isconsole(1) ? 1 : 0;
}

/* Check that /dev/portN matches the same character-special device */
static bool
port_is_same_cdev(int portnr, const char *devpath)
{
	struct stat portstat, devstat;
	char portpath[200];

	snprintf(portpath, sizeof portpath, "/dev/port%02d", portnr);
	return stat(portpath, &portstat) == 0 &&
	       S_ISCHR(portstat.st_mode) &&
	       stat(devpath, &devstat) == 0 &&
	       S_ISCHR(devstat.st_mode) &&
	       portstat.st_rdev == devstat.st_rdev;
}

bool
opengear_serial_isnmea(int portnr)
{
	return port_is_same_cdev(portnr, OG_DEV_NMEA);
}
bool
opengear_serial_isconsole(int portnr)
{
	return port_is_same_cdev(portnr, OG_DEV_SERCON);
}

bool
opengear_serial_is_disconnected(int portnr)
{
	char path[200];

	snprintf(path, sizeof(path), "/dev/port%02d", portnr);
	return access(path, F_OK) == -1 && errno == ENOENT;
}

const char *
opengear_serial_consoledevname()
{
	struct stat statbuf;
	if (stat(OG_DEV_SERCON, &statbuf) == 0 && S_ISCHR(statbuf.st_mode))
		return (OG_DEV_SERCON + strlen("/dev/"));
	return NULL;
}

const char *
opengear_serial_label(xmldb_t *db, int portnr)
{
	char portid[200];
	const char *label;

	snprintf(portid, sizeof portid, "config.ports.port%d", portnr);
	portid[sizeof portid - 1] = 0;
	label = xmldb_getstring(db, portid, "label");

	return label && *label ? label : NULL;
}

const char *
opengear_serial_displayname(xmldb_t *db, int portnr)
{
	static char display[1024];
	char *deflabel = NULL;
	const char *label = opengear_serial_label(db, portnr);

	if (!label) {
		deflabel = opengear_serial_get_deflabel(portnr);
		label = deflabel;
	}

	snprintf(display, sizeof display, "Port %d", portnr);
	if (label && *label && strcmp(label, display) != 0) {
		snprintf(display, sizeof display, "Port %d (%s)",
			portnr, label);
	}

	display[sizeof display - 1] = '\0';
	free(deflabel);

	return display;
}

/* CHelper function to fetch a port's property from infod */
static char *
get_infod_port_dev_property(int portnr, const char *prop)
{
	int ret;
	char prefix[sizeof "ports.port65535.MAXPROPNAME"];
	char *val = NULL;

	snprintf(prefix, sizeof prefix, "config.ports.port%d.dev.%s", portnr, prop);

	ret = infod_get(prefix, &val);
	return ret == -1 ? NULL : val;
}

char *
opengear_serial_get_deflabel(int portnr)
{
	return get_infod_port_dev_property(portnr, "deflabel");
}

char *
opengear_serial_get_typedesc(int portnr)
{
	return get_infod_port_dev_property(portnr, "typedesc");
}

