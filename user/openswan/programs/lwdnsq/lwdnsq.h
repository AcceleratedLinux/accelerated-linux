/*
 * DNS KEY lookup global definitions
 * Copyright (C) 2002 Michael Richardson <mcr@freeswan.org>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef POLLIN
#include <poll.h>
#endif

#ifndef FS_BOOL_DEFINED
#define FS_BOOL_DEFINED 1
typedef int bool;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#endif

#ifndef LWDNSQ_CMDBUF_LEN
#define LWDNSQ_CMDBUF_LEN      1024
#define LWDNSQ_RESULT_LEN_MAX  4096
#endif

#ifdef __gcc__
# define UNUSED __attribute__ ((unused))
#else
# define UNUSED /* ignore */
#endif


/*
 * a base-64 encoded 2192 bit key takes:
 *   2192/8 * 4/3 = 365 bytes.
 *
 * a base-64 encoded 16384 bit key takes:
 *   16384/8*4/3  = 2730 bytes.
 *
 * so, we pick 4096 bytes as the maximum.
 *
 * Note that TXT records may have an introducer (X-IPsec) and an ID which
 * is either an IP address or @FQDN that preceeds the base64 encoded key.
 *
 */

enum dkl_state {
	dkl_start,      /* no work yet none - initial state */
	dkl_first,      /* sent first DNS request. */
	dkl_cname,      /* sent request for CNAME record */
	dkl_second,     /* sent request for thing CNAME pointed to */
	dkl_done        /* done */
};

typedef struct dnskey_lookup dnskey_lookup;

/* should be tunable somehow */
#define LWDNSQ_RETRANSMIT_INTERVAL 20
#define LWDNSQ_RETRANSMIT_TRIES    20

struct dnskey_lookup {
	struct lwres_async_state las;
	dnskey_lookup           *next;
	char                    *tracking_id;
	enum dkl_state           step;
	/* lwres_context_t         *ctx; */
	char                    *wantedtype_name;
	dns_rdatatype_t          wantedtype;
	char                    *fqdn;
	int                      cname_count;
	int                      last_cname_used;
	dns_name_t               last_cname;
	int                      retry_count;
	time_t                   resend_time;
};

typedef struct dnskey_glob {
	int debug;
	bool prompt;
	bool promptnow;
	bool concurrent;
	bool done;
        bool regress;                  /* if 1, then we are doing regression testing */
	struct pollfd   l_fds[5];     /* array of input sources */
	unsigned int    l_nfds;       /* number of relevant entries */
	unsigned int    cmdloc;
	char            cmdbuf[LWDNSQ_CMDBUF_LEN];
	FILE           *cmdproto_out;
	dnskey_lookup  *dns_outstanding;
	int             dns_inflight;
	lwres_context_t *lwctx;
	isc_mem_t       *iscmem;
	isc_buffer_t    *iscbuf;
} dnskey_glob;

/* in cmds.c */
extern void lookup_key(dnskey_glob *gs,int, char **);
extern void lookup_key4(dnskey_glob *gs,int, char **);
extern void lookup_key6(dnskey_glob *gs,int, char **);
extern void lookup_txt(dnskey_glob *gs,int, char **);
extern void lookup_txt4(dnskey_glob *gs,int, char **);
extern void lookup_txt6(dnskey_glob *gs,int, char **);
extern void lookup_ipseckey(dnskey_glob *gs,int, char **);
extern void lookup_ipseckey4(dnskey_glob *gs,int, char **);
extern void lookup_ipseckey6(dnskey_glob *gs,int, char **);
extern void lookup_oe4(dnskey_glob *gs,int, char **);
extern void lookup_oe6(dnskey_glob *gs,int, char **);
extern void lookup_a(dnskey_glob *gs,int, char **);
extern void lookup_aaaa(dnskey_glob *gs,int, char **);
extern void output_transaction_line(dnskey_glob *gs,
				    const char *id,
				    int ttl,
				    const char *cmd,
				    const char *data);
extern void output_transaction_line_limited(dnskey_glob *gs,
					    const char *id,
					    int ttl,
					    const char *cmd,
					    int   max,
					    const char *data);

extern char *xstrdup(const char *s);



/* lookup code */
extern void process_dns_reply(dnskey_glob *gs);
extern void lookup_thing(dnskey_glob *gs,
			 dns_rdatatype_t wantedtype,
			 const char *wantedtype_name,
			 char *id,
			 char *fqdn);
extern void dns_rexmit(dnskey_glob *gs, dnskey_lookup *dl, int force);

/*
 *
 * Local variables:
 * c-file-style: "linux"
 * c-basic-offset: 2
 * End:
 *
 */
