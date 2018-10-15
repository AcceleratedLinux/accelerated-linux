#ifndef NET_SNMP_PDU_API_H
#define NET_SNMP_PDU_API_H

    /**
     *  Library API routines concerned with SNMP PDUs.
     */

#include <net-snmp/types.h>

#ifdef __cplusplus
extern          "C" {
#endif

netsnmp_pdu    *snmp_pdu_create(int type);
netsnmp_pdu    *snmp_clone_pdu(netsnmp_pdu *pdu);
netsnmp_pdu    *snmp_fix_pdu(  netsnmp_pdu *pdu, int idx);
void            snmp_free_pdu( netsnmp_pdu *pdu);

#ifdef __cplusplus
}
#endif

    /*
     *  For the initial release, this will just refer to the
     *  relevant UCD header files.
     *    In due course, the routines relevant to this area of the
     *  API will be identified, and listed here directly.
     *
     *  But for the time being, this header file is a placeholder,
     *  to allow application writers to adopt the new header file names.
     */

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_client.h>
#include <net-snmp/library/asn1.h>

#endif                          /* NET_SNMP_PDU_API_H */
