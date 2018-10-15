/*
 * Header file for Transport Security Model support
 */

#ifndef SNMPTSM_H
#define SNMPTSM_H

#ifdef __cplusplus
extern          "C" {
#endif

    int             tsm_rgenerate_out_msg(struct
                                          snmp_secmod_outgoing_params *);
    int             tsm_process_in_msg(struct snmp_secmod_incoming_params
                                       *);
    void            init_usm(void);

    #define NETSNMP_TM_SAME_SECURITY_NOT_REQUIRED 0
    #define NETSNMP_TM_USE_SAME_SECURITY          1

    /* basically we store almost nothing else but a tm ref */
    typedef struct netsnmp_tsmSecurityReference_s {
       netsnmp_tmStateReference *tmStateRef;
       int securityLevel;
    } netsnmp_tsmSecurityReference;

#ifdef __cplusplus
}
#endif
#endif                          /* SNMPTSM_H */
