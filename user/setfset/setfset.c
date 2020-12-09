/****************************************************************************/

/*
 *	setfset.c --  Set feature sets for Opengear devices
 *
 */

/****************************************************************************/

/*#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>*/
#include <opengear/fset.h>
#include "fsetv.h"

/****************************************************************************/

char * powerNames[] = {
	"external supply",
	"single internal supply",
	"dual internal supply",
	"invalid recoded supply"
};


	char * pinoutNames[] = {
		"standard",
		"cyclades",
		"cisco",
		"invalid"
	};


typedef enum {
	none,
	update,
	delete,
	raw
} update_action;

/****************************************************************************/

void usage(int rc)
{
	printf("usage: setfset [-h?] [OPTION]...\n"
		"\t-f <flash-device>\n"
		"\t-o <offset>\n"
		"\t-d <name>=<value>     delete values\n"
		"\t-u <name>=<value>     update values\n"
		"\t-r                    query raw values\n"
		"\t-q <values>           query one or more specific factory settings\n"
		"\t-z 			 Clear all variables. \n"
		"\t			 Use to recover from corrupted flash\n"
		"\t\tm - model\n"
		"\t\tn - serial number\n"
		"\t\ts - serial ports\n"
		"\t\tp - pinout\n"
		"\t\tc - console\n"
		"\t\to - power supplies\n"
		"\t\ti - power input\n"
		"\t\te - ethernet ports\n"
		"\t\tf - factory options\n"
		"\t\tt - Internal sensors\n"
		"\t\t0 - MAC address of eth0\n"
		"\t\t1 - MAC address of eth1\n"
		"\t\td - Country\n"
	);
	exit(rc);
}

/****************************************************************************/

int main(int argc, char *argv[])
{
	int c;
	char *query = NULL;
	char *flash = DEFAULTFLASH;
	char *mtdname = NULL;
	off_t offset = DEFAULTOFFS;
	char * updvar = NULL;
	size_t zeroflag = 0;
	update_action updact = none;

	while ((c = getopt(argc, argv, "h?ro:f:q:u:d:z")) > 0) {
		switch (c) {
		case '?':
		case 'h':
			usage(0);
		case 'f':
			flash = optarg;
			break;
		case 'o':
			offset = strtoul(optarg, NULL, 0);
			break;
		case 'q':
			query = optarg;
			break;
		case 'r':
			updact = raw;
			break;
		case 'u':
			if( updvar != NULL ) {
				usage(1);
			}
			updvar = optarg;
			updact = update;
			
			break;
		case 'd':
			if( updvar != NULL ) {
				usage(1);
			}
			updvar = optarg;
			updact = delete;
			break;
		case 'z':
			zeroflag = 1;
			break;
		default:
			usage(1);
		}
	}

	if (mtdname)
		flash = findmtddevice(mtdname);

	if(zeroflag && flash) {
		//clear the block of flash
		saveFlash(flash, offset);
		//set defaults to recover boot procedure
		resetDefaults(flash, offset);
	}
		

	if (flash) {
		size_t errors = readfsetflash(flash, offset);
		switch(errors) {
		case (1): 
			exit(1);
			break;
		case (2):
			usage(1);
			break;
		default:
			break;
		}
	}

	switch( updact ) {
		case none:
			printfset( query );
			break;
		case update:
			if (updateflash( flash, offset, updvar ) == 2) {
				usage(1);
			}
			break;
		case delete:
			deleteflash( flash, offset, updvar );
			break;
		case raw:
			printraw();
			break;
	}
			
	freeVars();

	return 0;
}

/****************************************************************************/
