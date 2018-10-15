#ifndef NET_SNMP_TYPES_H
#define NET_SNMP_TYPES_H

    /**
     *  Definitions of data structures, used within the library API.
     */

#include <stdio.h>

#ifndef NET_SNMP_CONFIG_H
#error "Please include <net-snmp/net-snmp-config.h> before this file"
#endif

                        /*
                         * For 'timeval' 
                         */
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <sys/types.h>
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>		/* For definition of in_addr_t */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_SUBID               /* temporary - duplicate definition protection */
#ifndef EIGHTBIT_SUBIDS
typedef u_long  oid;
#define MAX_SUBID   0xFFFFFFFF
#else
typedef u_char  oid;
#define MAX_SUBID   0xFF
#endif
#endif

#ifndef HAVE_SOCKLEN_T
typedef u_int socklen_t;
#endif

#ifndef HAVE_IN_ADDR_T
typedef u_int in_addr_t;
#endif

#ifndef HAVE_SSIZE_T
#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int ssize_t;
#else
typedef long ssize_t;
#endif
#endif

#ifndef HAVE_NFDS_T
typedef unsigned long int nfds_t;
#endif

    /*
     *  For the initial release, this will just refer to the
     *  relevant UCD header files.
     *    In due course, the types and structures relevant to the
     *  Net-SNMP API will be identified, and defined here directly.
     *
     *  But for the time being, this header file is primarily a placeholder,
     *  to allow application writers to adopt the new header file names.
     */

typedef union {
   long           *integer;
   u_char         *string;
   oid            *objid;
   u_char         *bitstring;
   struct counter64 *counter64;
#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
   float          *floatVal;
   double         *doubleVal;
   /*
    * t_union *unionVal; 
    */
#endif                          /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */
} netsnmp_vardata;


#define MAX_OID_LEN	    128 /* max subid's in an oid */

/** @typedef struct variable_list netsnmp_variable_list
 * Typedefs the variable_list struct into netsnmp_variable_list */
/** @struct variable_list
 * The netsnmp variable list binding structure, it's typedef'd to
 * netsnmp_variable_list.
 */
typedef struct variable_list {
   /** NULL for last variable */
   struct variable_list *next_variable;    
   /** Object identifier of variable */
   oid            *name;   
   /** number of subid's in name */
   size_t          name_length;    
   /** ASN type of variable */
   u_char          type;   
   /** value of variable */
    netsnmp_vardata val;
   /** the length of the value to be copied into buf */
   size_t          val_len;
   /** 90 percentile < 24. */
   oid             name_loc[MAX_OID_LEN];  
   /** 90 percentile < 40. */
   u_char          buf[40];
   /** (Opaque) hook for additional data */
   void           *data;
   /** callback to free above */
   void            (*dataFreeHook)(void *);    
   int             index;
} netsnmp_variable_list;


/** @typedef struct snmp_pdu to netsnmp_pdu
 * Typedefs the snmp_pdu struct into netsnmp_pdu */
/** @struct snmp_pdu
 * The snmp protocol data unit.
 */	
typedef struct snmp_pdu {

#define non_repeaters	errstat
#define max_repetitions errindex

    /*
     * Protocol-version independent fields
     */
    /** snmp version */
    long            version;
    /** Type of this PDU */	
    int             command;
    /** Request id - note: not incremented on retries */
    long            reqid;  
    /** Message id for V3 messages note: incremented for each retry */
    long            msgid;
    /** Unique ID for incoming transactions */
    long            transid;
    /** Session id for AgentX messages */
    long            sessid;
    /** Error status (non_repeaters in GetBulk) */
    long            errstat;
    /** Error index (max_repetitions in GetBulk) */
    long            errindex;       
    /** Uptime */
    u_long          time;   
    u_long          flags;

    int             securityModel;
    /** noAuthNoPriv, authNoPriv, authPriv */
    int             securityLevel;  
    int             msgParseModel;

    /**
     * Transport-specific opaque data.  This replaces the IP-centric address
     * field.  
     */
    
    void           *transport_data;
    int             transport_data_length;

    /**
     * The actual transport domain.  This SHOULD NOT BE FREE()D.  
     */

    const oid      *tDomain;
    size_t          tDomainLen;

    netsnmp_variable_list *variables;


    /*
     * SNMPv1 & SNMPv2c fields
     */
    /** community for outgoing requests. */
    u_char         *community;
    /** length of community name. */
    size_t          community_len;  

    /*
     * Trap information
     */
    /** System OID */
    oid            *enterprise;     
    size_t          enterprise_length;
    /** trap type */
    long            trap_type;
    /** specific type */
    long            specific_type;
    /** This is ONLY used for v1 TRAPs  */
    unsigned char   agent_addr[4];  

    /*
     *  SNMPv3 fields
     */
    /** context snmpEngineID */
    u_char         *contextEngineID;
    /** Length of contextEngineID */
    size_t          contextEngineIDLen;     
    /** authoritative contextName */
    char           *contextName;
    /** Length of contextName */
    size_t          contextNameLen;
    /** authoritative snmpEngineID for security */
    u_char         *securityEngineID;
    /** Length of securityEngineID */
    size_t          securityEngineIDLen;    
    /** on behalf of this principal */
    char           *securityName;
    /** Length of securityName. */
    size_t          securityNameLen;        
    
    /*
     * AgentX fields
     *      (also uses SNMPv1 community field)
     */
    int             priority;
    int             range_subid;
    
    void           *securityStateRef;
} netsnmp_pdu;


/** @typedef struct snmp_session netsnmp_session
 * Typedefs the snmp_session struct intonetsnmp_session */
        struct snmp_session;
typedef struct snmp_session netsnmp_session;

#define USM_AUTH_KU_LEN     32
#define USM_PRIV_KU_LEN     32

typedef int        (*snmp_callback) (int, netsnmp_session *, int,
                                          netsnmp_pdu *, void *);
typedef int     (*netsnmp_callback) (int, netsnmp_session *, int,
                                          netsnmp_pdu *, void *);
/** @struct snmp_session
 * The snmp session structure.
 */
struct snmp_session {
    /*
     * Protocol-version independent fields
     */
    /** snmp version */
    long            version;
    /** Number of retries before timeout. */
    int             retries;
    /** Number of uS until first timeout, then exponential backoff */
    long            timeout;        
    u_long          flags;
    struct snmp_session *subsession;
    struct snmp_session *next;

    /** name or address of default peer (may include transport specifier and/or port number) */
    char           *peername;
    /** UDP port number of peer. (NO LONGER USED - USE peername INSTEAD) */
    u_short         remote_port;
    /** My Domain name or dotted IP address, 0 for default */
    char           *localname;
    /** My UDP port number, 0 for default, picked randomly */
    u_short         local_port;     
    /**
     * Authentication function or NULL if null authentication is used 
     */
    u_char         *(*authenticator) (u_char *, size_t *, u_char *, size_t);
    /** Function to interpret incoming data */
    netsnmp_callback callback;      
    /**
     * Pointer to data that the callback function may consider important 
     */
    void           *callback_magic;
    /** copy of system errno */
    int             s_errno;
    /** copy of library errno */
    int             s_snmp_errno;   
    /** Session id - AgentX only */
    long            sessid; 

    /*
     * SNMPv1 & SNMPv2c fields
     */
    /** community for outgoing requests. */
    u_char         *community;
    /** Length of community name. */
    size_t          community_len;  
    /**  Largest message to try to receive.  */
    size_t          rcvMsgMaxSize;
    /**  Largest message to try to send.  */
    size_t          sndMsgMaxSize;  

    /*
     * SNMPv3 fields
     */
    /** are we the authoritative engine? */
    u_char          isAuthoritative;
    /** authoritative snmpEngineID */
    u_char         *contextEngineID;
    /** Length of contextEngineID */
    size_t          contextEngineIDLen;     
    /** initial engineBoots for remote engine */
    u_int           engineBoots;
    /** initial engineTime for remote engine */
    u_int           engineTime;
    /** authoritative contextName */
    char           *contextName;
    /** Length of contextName */
    size_t          contextNameLen;
    /** authoritative snmpEngineID */
    u_char         *securityEngineID;
    /** Length of contextEngineID */
    size_t          securityEngineIDLen;    
    /** on behalf of this principal */
    char           *securityName;
    /** Length of securityName. */
    size_t          securityNameLen;

    /** auth protocol oid */
    oid            *securityAuthProto;
    /** Length of auth protocol oid */
    size_t          securityAuthProtoLen;
    /** Ku for auth protocol XXX */
    u_char          securityAuthKey[USM_AUTH_KU_LEN];       
    /** Length of Ku for auth protocol */
    size_t          securityAuthKeyLen;
    /** Kul for auth protocol */
    u_char          *securityAuthLocalKey;       
    /** Length of Kul for auth protocol XXX */
    size_t          securityAuthLocalKeyLen;       

    /** priv protocol oid */
    oid            *securityPrivProto;
    /** Length of priv protocol oid */
    size_t          securityPrivProtoLen;
    /** Ku for privacy protocol XXX */
    u_char          securityPrivKey[USM_PRIV_KU_LEN];       
    /** Length of Ku for priv protocol */
    size_t          securityPrivKeyLen;
    /** Kul for priv protocol */
    u_char          *securityPrivLocalKey;       
    /** Length of Kul for priv protocol XXX */
    size_t          securityPrivLocalKeyLen;       

    /** snmp security model, v1, v2c, usm */
    int             securityModel;
    /** noAuthNoPriv, authNoPriv, authPriv */
    int             securityLevel;  
    /** target param name */
    char           *paramName;

    /**
     * security module specific 
     */
    void           *securityInfo;

    /**
     * use as you want data 
     *
     *     used by 'SNMP_FLAGS_RESP_CALLBACK' handling in the agent
     * XXX: or should we add a new field into this structure?
     */
    void           *myvoid;
};


#include <net-snmp/library/types.h>
#include <net-snmp/definitions.h>
#include <net-snmp/library/snmp_api.h>

#ifdef __cplusplus
}
#endif

#endif                          /* NET_SNMP_TYPES_H */
