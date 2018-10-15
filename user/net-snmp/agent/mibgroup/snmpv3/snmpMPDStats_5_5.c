/*
 * snmpMPDStats.c: tallies errors for SNMPv3 message processing.
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/sysORTable.h>

#include "snmpMPDStats_5_5.h"

#include <utilities/snmp_get_statistic.h>

#define snmpMPDMIBObjects 1, 3, 6, 1, 6, 3, 11, 2
#define snmpMPDMIBCompliances snmpMPDMIBObjects, 3, 1

static oid snmpMPDStats[] = { snmpMPDMIBObjects, 1 };

static netsnmp_handler_registration* snmpMPDStats_reg = NULL;
static oid snmpMPDCompliance[] = { snmpMPDMIBCompliances, 1 };

void
init_snmpMPDStats_5_5(void)
{
    netsnmp_handler_registration* s =
        netsnmp_create_handler_registration(
            "snmpMPDStats", NULL, snmpMPDStats, OID_LENGTH(snmpMPDStats),
            HANDLER_CAN_RONLY);
    if (s &&
	NETSNMP_REGISTER_STATISTIC_HANDLER(s, 1, MPD) == MIB_REGISTERED_OK) {
        REGISTER_SYSOR_ENTRY(snmpMPDCompliance,
                             "The MIB for Message Processing and Dispatching.");
        snmpMPDStats_reg = s;
    }
}

void
shutdown_snmpMPDStats_5_5(void)
{
    UNREGISTER_SYSOR_ENTRY(snmpMPDCompliance);
    if (snmpMPDStats_reg) {
        netsnmp_unregister_handler(snmpMPDStats_reg);
        snmpMPDStats_reg = NULL;
    }
}
