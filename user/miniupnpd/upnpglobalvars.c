/* $Id: upnpglobalvars.c,v 1.1 2008-09-15 12:28:52 winfred Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard 
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <netinet/in.h>

#include "config.h"
#include "upnpglobalvars.h"

/* network interface for internet */
const char * ext_if_name = 0;

/* file to store leases */
#ifdef ENABLE_LEASEFILE
const char* lease_file = 0;
#endif

/* forced ip address to use for this interface
 * when NULL, getifaddr() is used */
const char * use_ext_ip_addr = 0;

/* LAN address */
/*const char * listen_addr = 0;*/

unsigned long downstream_bitrate = 0;
unsigned long upstream_bitrate = 0;

/* startup time */
time_t startup_time = 0;

#if 0
/* use system uptime */
int sysuptime = 0;

/* log packets flag */
int logpackets = 0;

#ifdef ENABLE_NATPMP
int enablenatpmp = 0;
#endif
#endif

int runtime_flags = 0;

const char * pidfilename = "/var/run/miniupnpd.pid";

char uuidvalue[] = "uuid:00000000-0000-0000-0000-000000000000";
char serialnumber[SERIALNUMBER_MAX_LEN] = "00000000";

char modelnumber[MODELNUMBER_MAX_LEN] = "1";

/* presentation url :
 * http://nnn.nnn.nnn.nnn:ppppp/  => max 30 bytes including terminating 0 */
char presentationurl[PRESENTATIONURL_MAX_LEN];

/* UPnP permission rules : */
struct upnpperm * upnppermlist = 0;
unsigned int num_upnpperm = 0;

#ifdef ENABLE_NATPMP
/* NAT-PMP */
unsigned int nextnatpmptoclean_timestamp = 0;
unsigned short nextnatpmptoclean_eport = 0;
unsigned short nextnatpmptoclean_proto = 0;
#endif

#ifdef USE_PF
const char * queue = 0;
const char * tag = 0;
#endif

int n_lan_addr = 0;
struct lan_addr_s lan_addr[MAX_LAN_ADDR];


