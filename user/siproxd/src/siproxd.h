/*
    Copyright (C) 2002-2005  Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.
    
    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

/* $Id: siproxd.h,v 1.53 2005/04/19 19:00:23 hb9xar Exp $ */

#ifdef DMALLOC
 #include <dmalloc.h>
#endif


/*
 * table to hold the client registrations
 */
struct urlmap_s {
   int  active;
   int  expires;
   osip_uri_t *true_url;	// true URL of UA  (inbound URL)
   osip_uri_t *masq_url;	// masqueraded URL (outbound URL)
   osip_uri_t *reg_url;		// registered URL  (masq URL as wished by UA)
};
/*
 * the difference between masq_url and reg_url is, 
 * the reg URL *always* holds the url registered by the UA.
 * the masq_url may contain a different URL due to an additional
 * masquerading feature (mask_host, masked_host config options)
 */


/*
 * Array of strings - used within configuration store
 */
#define CFG_STRARR_SIZE		128	/* max 128 entries in array */
typedef struct {
   int  used;
   char *string[CFG_STRARR_SIZE];
} stringa_t;

/*
 * configuration option table
 */
struct siproxd_config {
   int debuglevel;
   int debugport;
   char *inbound_if;
   char *outbound_if;
   char *outbound_host;
   int sip_listen_port;
   int daemonize;
   int silence_log;
   int rtp_port_low;
   int rtp_port_high;
   int rtp_timeout;
   int rtp_dscp;
   int rtp_proxy_enable;
   char *user;
   char *chrootjail;
   char *hosts_allow_reg;
   char *hosts_allow_sip;
   char *hosts_deny_sip;
   char *proxy_auth_realm;
   char *proxy_auth_passwd;
   char *proxy_auth_pwfile;
   stringa_t mask_host;
   stringa_t masked_host;
   char *outbound_proxy_host;
   int  outbound_proxy_port;
   stringa_t outbound_proxy_domain_name;
   stringa_t outbound_proxy_domain_host;
   stringa_t outbound_proxy_domain_port;
   char *registrationfile;
   int  log_calls;
   char *pid_file;
   int  default_expires;
   int  autosave_registrations;
};

/*
 * SIP ticket
 */
typedef struct {
   osip_message_t *sipmsg;	/* SIP */
   struct sockaddr_in from;	/* received from */
#define PROTO_UNKN -1
#define PROTO_UDP  1
#define PROTO_TCP  2
   int protocol;		/* received by protocol */
#define REQTYP_INCOMING		1
#define REQTYP_OUTGOING		2
#define RESTYP_INCOMING		3
#define RESTYP_OUTGOING		4
   int direction;		/* direction as determined by proxy */
} sip_ticket_t;



/*
 * Function prototypes
 */

/*				function returns STS_* status values     vvv */

/* sock.c */
int sipsock_listen(void);						/*X*/
int sipsock_wait(void);
int sipsock_read(void *buf, size_t bufsize,
                 struct sockaddr_in *from, int *protocol);
int sipsock_send(struct in_addr addr, int port,	int protocol,			/*X*/
                 char *buffer, int size);
int sockbind(struct in_addr ipaddr, int localport, int errflg);

/* register.c */
void register_init(void);
void register_save(void);
int  register_client(sip_ticket_t *ticket, int force_lcl_masq);		/*X*/
void register_agemap(void);
int  register_response(sip_ticket_t *ticket, int flag);			/*X*/

/* proxy.c */
int proxy_request (sip_ticket_t *ticket);				/*X*/
int proxy_response (sip_ticket_t *ticket);				/*X*/
int proxy_rewrite_invitation_body(osip_message_t *m, int direction);    /*X*/
int proxy_rewrite_request_uri(osip_message_t *mymsg, int idx);		/*X*/

/* route_preprocessing.c */
int route_preprocess(sip_ticket_t *ticket);				/*X*/
int route_add_recordroute(sip_ticket_t *ticket);			/*X*/
int route_purge_recordroute(sip_ticket_t *ticket);			/*X*/
int route_postprocess(sip_ticket_t *ticket);				/*X*/
int route_determine_nexthop(sip_ticket_t *ticket,
                            struct in_addr *dest, int *port);		/*X*/

/* utils.c */
int  get_ip_by_host(char *hostname, struct in_addr *addr);		/*X*/
void secure_enviroment (void);
int  get_ip_by_ifname(char *ifname, struct in_addr *retaddr);		/*X*/
int  get_interface_ip(int interface, struct in_addr *retaddr);		/*X*/
char *utils_inet_ntoa(struct in_addr in);
int  utils_inet_aton(const char *cp, struct in_addr *inp);

/* sip_utils.c */
osip_message_t * msg_make_template_reply (sip_ticket_t *ticket, int code);
int  check_vialoop (sip_ticket_t *ticket);				/*X*/
int  is_via_local (osip_via_t *via);					/*X*/
int  compare_url(osip_uri_t *url1, osip_uri_t *url2);			/*X*/
int  compare_callid(osip_call_id_t *cid1, osip_call_id_t *cid2);	/*X*/
int  is_sipuri_local (sip_ticket_t *ticket);				/*X*/
int  check_rewrite_rq_uri (osip_message_t *sip);			/*X*/
int  sip_gen_response(sip_ticket_t *ticket, int code);			/*X*/
int  sip_add_myvia (sip_ticket_t *ticket, int interface);		/*X*/
int  sip_del_myvia (sip_ticket_t *ticket);				/*X*/
int  sip_rewrite_contact (sip_ticket_t *ticket, int direction);		/*X*/
int  sip_calculate_branch_id (sip_ticket_t *ticket, char *id);		/*X*/
int  sip_find_outbound_proxy(sip_ticket_t *ticket, struct in_addr *addr,
                             int *port);				/*X*/

/* readconf.c */
int read_config(char *name, int search);				/*X*/
int make_default_config(void);						/*X*/

/* rtpproxy.c */
int  rtpproxy_init( void );						/*X*/
int  rtp_start_fwd (osip_call_id_t *callid, char *client_id,            /*X*/
                    int direction, int media_stream_no,
                    struct in_addr outbound_ipaddr, int *outboundport,
                    struct in_addr lcl_client_ipaddr, int lcl_clientport);
int  rtp_stop_fwd (osip_call_id_t *callid, int direction);		/*X*/
void rtpproxy_kill( void );						/*X*/

/* accessctl.c */
int  accesslist_check(struct sockaddr_in from);

/* security.c */
int  security_check_raw(char *sip_buffer, int size);			/*X*/
int  security_check_sip(sip_ticket_t *ticket);				/*X*/

/* auth.c */
int  authenticate_proxy(sip_ticket_t *ticket);				/*X*/
int  auth_include_authrq(sip_ticket_t *ticket);				/*X*/
void CvtHex(char *hash, char *hashstring);

/* fwapi.c */
int fwapi_start_rtp(int rtp_direction,
                    struct in_addr local_ipaddr, int local_port,
                    struct in_addr remote_ipaddr, int remote_port);
int fwapi_stop_rtp(int rtp_direction,
                   struct in_addr local_ipaddr, int local_port,
                   struct in_addr remote_ipaddr, int remote_port);

/* sip_layer.c */
int sip_message_parse(osip_message_t * sip, const char *buf, int len);
int sip_message_to_str(osip_message_t * sip, char **dest, int *len);
int sip_body_to_str(const osip_body_t * body, char **dest, int *len);
int sip_message_set_body(osip_message_t * sip, const char *buf, int len);


/*
 * some constant definitions
 */
#define SIP_PORT	5060	/* default port to listen */
#define DEFAULT_MAXFWD	70	/* default Max-Forward count */
#define DEFAULT_EXPIRES	3600	/* default Expires timeout */

#define URLMAP_SIZE	32	/* number of URL mapping table entries	*/
#define RTPPROXY_SIZE	64	/* number of rtp proxy entries		*/

#define BUFFER_SIZE	8196	/* input buffer for read from socket	*/
#define RTP_BUFFER_SIZE	512	/* max size of an RTP frame		*/
#define URL_STRING_SIZE	128	/* max size of an URL/URI string	*/
#define STATUSCODE_SIZE 5	/* size of string representation of status */
#define DNS_CACHE_SIZE  32	/* number of entries in internal DNS cache */
#define DNS_MAX_AGE	60	/* maximum age of an cache entry (sec)	*/
#define IFADR_CACHE_SIZE 32	/* number of entries in internal IFADR cache */
#define IFADR_MAX_AGE	5	/* max. age of the IF address cache (sec) */
#define IFNAME_SIZE	16	/* max string length of a interface name */
#define HOSTNAME_SIZE	64	/* max string length of a hostname	*/
#define USERNAME_SIZE	64	/* max string length of a username (auth) */
#define PASSWORD_SIZE	64	/* max string length of a password (auth) */
#define VIA_BRANCH_SIZE 64	/* max string length for via branch param */
				/* scratch buffer for gethostbyname_r() */
#if defined(PR_NETDB_BUF_SIZE)
   #define GETHOSTBYNAME_BUFLEN PR_NETDB_BUF_SIZE 
#else
   #define GETHOSTBYNAME_BUFLEN 1024
#endif

/* constants for security testing */
#define SEC_MINLEN	16	/* minimum received length */
#define SEC_MAXLINELEN	1024	/* maximum acceptable length of one line
				   in the SIP telegram (security check)
				   Careful: Proxy-Authorization lines may
				   get quite long */

/* symbols for access control */
#define ACCESSCTL_SIP	1	/* for access control - SIP allowed	*/
#define ACCESSCTL_REG	2	/* --"--              - registr. allowed */

/* symbolic return stati */
#define STS_SUCCESS	0	/* SUCCESS				*/
#define STS_TRUE	0	/* TRUE					*/
#define STS_FAILURE	1	/* FAILURE				*/
#define STS_FALSE	1	/* FALSE				*/
#define STS_NEED_AUTH	1001	/* need authentication			*/

/* symbolic direction of data */
#define DIR_INCOMING	1
#define DIR_OUTGOING	2

/* Interfaces */
#define IF_OUTBOUND 0
#define IF_INBOUND  1

/* various */
#ifndef satoi
#define satoi atoi  /* used in libosips MSG_TEST_CODE macro ... */
#endif

// Lets make sure the pidfilename gets all the way to utils.c
int createpidfile(char* pidfilename);
extern char* pidfilename;
