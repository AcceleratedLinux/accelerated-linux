#include <opengear/fset.h>

typedef struct F_SET {
#define MODEL_LEN 32
	char model[MODEL_LEN];
#define SERIAL_COUNT_LEN 4
	char serial_count[SERIAL_COUNT_LEN];
#define SERIAL_NO_LEN 24
	char serial_no[SERIAL_NO_LEN];
#define FACTORY_OPTS_LEN 48
	char factory_opts[FACTORY_OPTS_LEN];
#define SENSORS_LEN 8
	char sensors[SENSORS_LEN];
#define MAC_LEN 18
	char ethaddr[MAC_LEN];
	char eth1addr[MAC_LEN];
#define COUNTRY_LEN 2
	char country[COUNTRY_LEN + 1];

	char *power_supplies;
	char *power_input;
	char *pinout;
	char *console;
	char *ethernet_count;

} fset;

/*
 *	Search for a mtd partition in /proc/mtd.
 *	Assumes that each line starts with the device name followed
 *	by a ':', and the partition name is enclosed by quotes.
 *	Returns a pointer into a temporary buffer that will be
 *	overwritten on the next call.
 */
char *findmtddevice(const char *mtdname);

void printfset(const char *query);
void parseFactorySetup(fset *features);
void printraw(void);
void getfset(fset* features);
void resetDefaults(const char * flash, off_t offset);
