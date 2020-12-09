#define _GNU_SOURCE /* for strndup */
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>

#include "fsetv.h"
#include <opengear/og_config.h>
#include <opengear/fset.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

char
*findmtddevice(const char *mtdname)
{
	FILE *f;
	char buf[80];
	int found;
	static char device[80];
	char *p, *q;

	f = fopen("/proc/mtd", "r");
	if (!f) {
		syslog(LOG_ERR, "setfset: open /proc/mtd failed");
		return NULL;
	}

	found = 0;
	while (!found && fgets(buf, sizeof(buf), f)) {
		p = strchr(buf, ':');
		if (!p)
			continue;
		*p++ = '\0';

		p = strchr(p, '"');
		if (!p)
			continue;
		p++;

		q = strchr(p, '"');
		if (!q)
			continue;
		*q = '\0';

		if (strcmp(p, mtdname) == 0) {
			found = 1;
			break;
		}
	}
	fclose(f);

	if (found) {
		sprintf(device, "/dev/%s", buf);
		return device;
	} else {
		fprintf(stderr, "setfset: mtd device '%s' not found\n", mtdname);
		return NULL;
	}
}

void
printraw(void)
{
	vset *next = firstValue();
	while (next != NULL) {
		printf("%s=%s\n", next->name, next->value);
		next = next->next;
	}
}

static const char *default_env[] = {
#if defined(PRODUCT_IM72xx)
		"bootpart=0",
		"bootdelay=0",
		"bootconsole=console=none",
		"baudrate=115200",
#else
		"bootargs=ttyAM0,115200",
		"bootcmd=gofsk 0x18260000 0x01000000",
		"bootdelay=2",
		"baudrate=115200",
#endif
		NULL};

void
resetDefaults(const char * flash, off_t offset)
{

	int i = 0;
	while (default_env[i] != NULL) {
		updateflash( flash, offset, default_env[i++] );
	}
}

void
parseFactorySetup(fset *features)
{
	int corrupt = 0;

	/*
	 * model
	 * ports N
	 * serial_no N
	 * ethaddr XX:XX:XX:XX:XX:XX
	 * eth1addr XX:XX:XX:XX:XX:XX
	 * factory_opts [modem,flash,3g,wireless]
	 * power [single|dual|external]/[ac|dc]
	 * pinout [cyclades|cisco|opengear]
	 * console [dedicated|shared]
	 * ethernet [single|dual]
	 * sensors [number of temp sensors/number of contact sensors]
	 * country
	 */

	/* model */
	vset *var = findValue("model") ;
	if (var == NULL) {
		if (!corrupt) {
			syslog( LOG_DEBUG,
				"failed to retrieve model setting");
			corrupt = 1;
		}
	} else {
		strncpy(features->model, var->value, MODEL_LEN - 1);
		features->model[MODEL_LEN - 1] = '\0';
	}

	/* ports N. */
	var = findValue("ports");
	if (var == NULL) {
		if (!corrupt) {
			syslog(LOG_DEBUG, "failed to retrieve port setting");
			corrupt = 1;
		}
	} else {
		strncpy(features->serial_count,
				var->value, SERIAL_COUNT_LEN - 1);
		features->serial_count[SERIAL_COUNT_LEN - 1] = '\0';
	}

	/* serial_no N */
	var = findValue("serial_no") ;
	if (var == NULL) {
		if (!corrupt) {
			syslog(LOG_DEBUG, "failed to retrieve serial setting");
			corrupt = 1;
		}
	} else {
		strncpy(features->serial_no, var->value, SERIAL_NO_LEN - 1);
		features->serial_no[SERIAL_NO_LEN - 1] = '\0';
	}

	/* ethernet [single|dual] */
	var = findValue("ethernet") ;
	if (var == NULL) {
		features->ethernet_count = "unknown";
		if (!corrupt) {
			syslog(LOG_DEBUG,
				"failed to retrieve ethernet setting");
			corrupt = 1;
		}
	} else {
		if (!strcmp(var->value, "single")) {
			features->ethernet_count = "single";
		} else if( !strcmp( var->value, "dual" ) ) {
			features->ethernet_count = "dual";
		} else {
			features->ethernet_count = "unknown";
			if (!corrupt) {
				syslog(LOG_DEBUG,
					"unexpected ethernet setting");
				corrupt = 1;
			}
		}
	}

	/* ethaddr XX:XX:XX:XX:XX:XX */
	var = findValue("ethaddr") ;
	if (var == NULL) {
		if (!corrupt) {
			syslog(LOG_DEBUG,
				"failed to retrieve mac address for eth0");
			corrupt = 1;
		}
	} else {
		strncpy(features->ethaddr, var->value, MAC_LEN - 1);
		features->ethaddr[MAC_LEN - 1] = '\0';
	}

	/* eth1addr XX:XX:XX:XX:XX:XX */
	var = findValue("eth1addr") ;
	if (var == NULL) {
		if (strcmp(features->ethernet_count, "dual") == 0) {
			if (!corrupt) {
				syslog(LOG_DEBUG, "failed to retrieve mac "
					"address for eth1");
				corrupt = 1;
			}
		}
	} else {
		strncpy(features->eth1addr, var->value, MAC_LEN - 1);
		features->eth1addr[MAC_LEN - 1] = '\0';
	}

	/* factory_opts [modem,flash,3g,wireless] */
	var = findValue("factory_opts") ;
	if (var == NULL) {
		if (!corrupt) {
			syslog(LOG_DEBUG, "failed to retrieve factory options");
			corrupt = 1;
		}
	} else {
		strncpy( features->factory_opts, var->value, FACTORY_OPTS_LEN - 1 );
		features->factory_opts[FACTORY_OPTS_LEN - 1] = '\0';
	}

	/* power [single|dual|external]/[ac|dc] */
	var = findValue( "power" ) ;
	if( var == NULL ) {
		features->power_supplies = "unknown";
		features->power_input = "unknown";
		if( !corrupt ) {
			syslog( LOG_DEBUG,
				"failed to retrieve power setting" );
			corrupt = 1;
		}
	} else {
		char * a = strstr( var->value, "/" );
		if( a == NULL ) {
			features->power_supplies = "unknown";
			features->power_input = "unknown";
		} else {
			if( !strncmp( var->value, "single/", 7 ) ) {
				features->power_supplies = "single";
			} else if(!strncmp( var->value, "dual/", 5 ) ) {
				features->power_supplies = "dual";
			} else if(!strncmp( var->value, "external/", 9 ) ) {
				features->power_supplies = "external supply";
			} else {
				features->power_supplies = "unknown";
				if( !corrupt ) {
					syslog( LOG_DEBUG,
						"unexpected power setting" );
					corrupt = 1;
				}
			}

			if( !strcmp( a, "/ac" ) ) {
				features->power_input = "ac";
			} else if( !strcmp( a, "/dc" ) ) {
				features->power_input = "dc";
			} else {
				features->power_input = "unknown";
				if( !corrupt ) {
					syslog( LOG_DEBUG,
						"unexpected input setting" );
					corrupt = 1;
				}
			}
		}
	}

	/* pinout [cyclades|cisco|opengear] */
	var = findValue( "pinout" ) ;
	if( var == NULL ) {
		features->pinout = "unknown";
		if( !corrupt ) {
			syslog( LOG_DEBUG,
				"failed to retrieve pinout setting" );
			corrupt = 1;
		}
	} else {
		if( !strcmp( var->value, "cyclades" ) ) {
			features->pinout = "cyclades";
		} else if( !strcmp( var->value, "cisco" ) ) {
			features->pinout = "cisco";
		} else if( !strcmp( var->value, "opengear" ) ) {
			features->pinout = "opengear";
		} else {
			features->pinout = "unknown";
			if( !corrupt ) {
				syslog( LOG_DEBUG,
					"unexpected pinout setting" );
				corrupt = 1;
			}
		}
	}


	/* console [dedicated|shared] */
	var = findValue( "console" ) ;
	if( var == NULL ) {
		features->console = "unknown";
		if( !corrupt ) {
			syslog( LOG_DEBUG,
				"failed to retrieve console setting" );
			corrupt = 1;
		}
	} else {
		if( !strcmp( var->value, "dedicated" ) ) {
			features->console = "dedicated";
		} else if( !strcmp( var->value, "shared" ) ) {
			features->console = "shared";
		} else {
			features->console = "unknown";
			if( !corrupt ) {
				syslog( LOG_DEBUG,
					"unexpected console setting" );
				corrupt = 1;
			}
		}
	}



	/* sensors [number of temp sensors/number of contact sensors] */
	var = findValue( "sensors" ) ;
	if( var == NULL ) {
		if( !corrupt ) {
			syslog( LOG_DEBUG,
				"failed to retrieve sensor options" );
			corrupt = 1;
		}
	} else {
		strncpy( features->sensors, var->value, SENSORS_LEN - 1 );
		features->sensors[SENSORS_LEN - 1] = '\0';
	}

	var = findValue("country") ;
	if(var) {
		strncpy(features->country, var->value, COUNTRY_LEN);
		features->country[COUNTRY_LEN] = '\0';
	}
}

void
printfset(const char * query)
{
	fset features;
	memset( &features, 0, sizeof(features) );

	parseFactorySetup( &features );

#define PRINTF(x) \
	if( query == NULL || strstr( query, x ) ) printf

	PRINTF("m")("Model: %.31s\n", features.model);
	PRINTF("n")("Serial number: %s\n", features.serial_no);
	PRINTF("s")("Serial port count: %s\n", features.serial_count);
	PRINTF("p")("Pinout: %s\n", features.pinout);
	PRINTF("c")("Console: %s\n", features.console);
	PRINTF("o")("Power: %s\n", features.power_supplies);
	PRINTF("i")("Power Input: %s\n", features.power_input);
	PRINTF("e")("Ethernet: %s\n", features.ethernet_count);
	PRINTF("f")("Factory options: %s\n", features.factory_opts);
	PRINTF("t")("Internal sensors: %s\n", features.sensors);
	PRINTF("0")("MAC address for eth0: %s\n", features.ethaddr);
	PRINTF("1")("MAC address for eth1: %s\n", features.eth1addr);
	PRINTF("d")("Country: %s\n", features.country);

#undef PRINTF
}

void
getfset(fset* features)
{
	memset(features, 0, sizeof(*features));
	parseFactorySetup(features);
}


