/*
 * lcp.h - Link Control Protocol definitions.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: lcp.h,v 1.1.1.1 1999-11-22 03:47:54 christ Exp $
 */

/*
 * Options.
 */
#define CI_MRU		1	/* Maximum Receive Unit */
#define CI_ASYNCMAP	2	/* Async Control Character Map */
#define CI_AUTHTYPE	3	/* Authentication Type */
#define CI_QUALITY	4	/* Quality Protocol */
#define CI_MAGICNUMBER	5	/* Magic Number */
#define CI_PCOMPRESSION	7	/* Protocol Field Compression */
#define CI_ACCOMPRESSION 8	/* Address/Control Field Compression */
#define CI_CALLBACK	13	/* callback */

/*
 * LCP-specific packet types.
 */
#define PROTREJ		8	/* Protocol Reject */
#define ECHOREQ		9	/* Echo Request */
#define ECHOREP		10	/* Echo Reply */
#define DISCREQ		11	/* Discard Request */
#define CBCP_OPT	6	/* Use callback control protocol */

/*
 * The state of options is described by an lcp_options structure.
 */
typedef struct lcp_options {
    bool passive;		/* Don't die if we don't get a response */
    bool silent;		/* Wait for the other end to start first */
    bool restart;		/* Restart vs. exit after close */
    bool neg_mru;		/* Negotiate the MRU? */
    bool neg_asyncmap;		/* Negotiate the async map? */
    bool neg_upap;		/* Ask for UPAP authentication? */
    bool neg_chap;		/* Ask for CHAP authentication? */
    bool neg_magicnumber;	/* Ask for magic number? */
    bool neg_pcompression;	/* HDLC Protocol Field Compression? */
    bool neg_accompression;	/* HDLC Address/Control Field Compression? */
    bool neg_lqr;		/* Negotiate use of Link Quality Reports */
    bool neg_cbcp;		/* Negotiate use of CBCP */
    bool use_digest;
    bool use_chapms;
    bool use_chapms_v2;
    int  mru;			/* Value of MRU */
    u_char chap_mdtype;		/* which MD type (hashing algorithm) */
    u_int32_t asyncmap;		/* Value of async map */
    u_int32_t magicnumber;
    int numloops;		/* Number of loops during magic number neg. */
    u_int32_t lqr_period;	/* Reporting period for LQR 1/100ths second */
} lcp_options;

extern fsm lcp_fsm[];
extern lcp_options lcp_wantoptions[];
extern lcp_options lcp_gotoptions[];
extern lcp_options lcp_allowoptions[];
extern lcp_options lcp_hisoptions[];
extern u_int32_t xmit_accm[][8];

#define DEFMRU	1500		/* Try for this */
#define MINMRU	128		/* No MRUs below this */
#define MAXMRU	16384		/* Normally limit MRU to this */

void lcp_open __P((int));
void lcp_close __P((int, char *));
void lcp_lowerup __P((int));
void lcp_lowerdown __P((int));
void lcp_sprotrej __P((int, u_char *, int));	/* send protocol reject */

extern struct protent lcp_protent;

/* Default number of times we receive our magic number from the peer
   before deciding the link is looped-back. */
#define DEFLOOPBACKFAIL	10
