/*
 * dsock.c - Linux socket processing functions for /proc-based lsof
 */


/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright 1997 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dsock.c,v 1.30 2005/08/29 10:23:33 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#define	INOBUCKS	128		/* inode hash bucket count */
#define INOHASH(ino)	((int)((ino * 31415) >> 3) & (INOBUCKS - 1))


/*
 * Local structures
 */

struct ax25sin {			/* AX25 socket information */
	char *da;			/* destination address */
	char *dev_ch;			/* device characters */
	char *sa;			/* source address */
	INODETYPE inode;
	unsigned long sq, rq;		/* send and receive queue values */
	unsigned char sqs, rqs;		/* send and receive queue states */
	int state;
	struct ax25sin *next;
};

struct ipxsin {				/* IPX socket information */
	INODETYPE inode;
	char *la;			/* local address */
	char *ra;			/* remote address */
	int state;
	unsigned long txq, rxq;		/* transmit and receive queue values */
	struct ipxsin *next;
};

struct rawsin {				/* raw socket information */
	INODETYPE inode;
	char *la;			/* local address */
	char *ra;			/* remote address */
	char *sp;			/* state characters */
	MALLOC_S lal;			/* strlen(la) */
	MALLOC_S ral;			/* strlen(ra) */
	MALLOC_S spl;			/* strlen(sp) */
	struct rawsin *next;
};

struct tcp_udp {			/* IPv4 TCP and UDP socket
					 * information */
	INODETYPE inode;
	unsigned long faddr, laddr;	/* foreign & local IPv6 addresses */
	int fport, lport;		/* foreign & local ports */
	unsigned long txq, rxq;		/* transmit & receive queue values */
	int proto;			/* 0 = TCP, 1 = UDP */
	int state;			/* protocol state */
	struct tcp_udp *next;
};

#if	defined(HASIPv6)
struct tcp_udp6 {			/* IPv6 TCP and UDP socket
					 * information */
	INODETYPE inode;
	struct in6_addr faddr, laddr;	/* foreign and local IPv6 addresses */
	int fport, lport;		/* foreign & local ports */
	unsigned long txq, rxq;		/* transmit & receive queue values */
	int proto;			/* 0 = TCP, 1 = UDP */
	int state;			/* protocol state */
	struct tcp_udp6 *next;
};
#endif	/* defined(HASIPv6) */

struct uxsin {				/* UNIX socket information */
	INODETYPE inode;
	char *pcb;
	char *path;
	unsigned char sb_def;
	dev_t sb_dev;
	INODETYPE sb_ino;
	dev_t sb_rdev;
	struct uxsin *next;
};


/*
 * Local static values
 */

static char *AX25path = (char *)NULL;	/* path to AX25 /proc information */
static struct ax25sin **AX25sin = (struct ax25sin **)NULL;
					/* AX25 socket info, hashed by inode */
static char *ax25st[] = {
	"LISTENING",			/* 0 */
	"SABM SENT",			/* 1 */
	"DISC SENT",			/* 2 */
	"ESTABLISHED",			/* 3 */
	"RECOVERY"			/* 4 */
};
#define NAX25ST	(sizeof(ax25st) / sizeof(char *))
static char *Ipxpath = (char *)NULL;	/* path to IPX /proc information */
static struct ipxsin **Ipxsin = (struct ipxsin **)NULL;
					/* IPX socket info, hashed by inode */
static char *Rawpath = (char *)NULL;	/* path to raw socket /proc
					 * information */
static struct rawsin **Rawsin = (struct rawsin **)NULL;
					/* raw socket info, hashed by inode */
static char *TCPpath = (char *)NULL;	/* path to TCP /proc information */
static struct tcp_udp **TcpUdp = (struct tcp_udp **)NULL;
					/* IPv4 TCP & UDP info, hashed by
					 * inode */

#if	defined(HASIPv6)
static char *Raw6path = (char *)NULL;	/* path to raw IPv6 /proc information */
static struct rawsin **Rawsin6 = (struct rawsin **)NULL;
					/* IPv6 raw socket info, hashed by
					 * inode */
static char *TCP6path = (char *)NULL;	/* path to IPv6 TCP /proc information */
static struct tcp_udp6 **TcpUdp6 = (struct tcp_udp6 **)NULL;
					/* IPv6 TCP & UDP info, hashed by
					 * inode */
static char *UDP6path = (char *)NULL;	/* path to IPv6 UDP /proc information */
#endif	/* defined(HASIPv6) */

static char *UDPpath = (char *)NULL;	/* path to UDP /proc information */
static char *UNIXpath = (char *)NULL;	/* path to UNIX /proc information */
static struct uxsin **Uxsin = (struct uxsin **)NULL;
					/* UNIX socket info, hashed by inode */


/*
 * Local function prototypes
 */

_PROTOTYPE(static struct ax25sin *check_ax25,(INODETYPE i));
_PROTOTYPE(static struct ipxsin *check_ipx,(INODETYPE i));
_PROTOTYPE(static struct rawsin *check_raw,(INODETYPE i));
_PROTOTYPE(static struct tcp_udp *check_tcpudp,(INODETYPE i, char **p));
_PROTOTYPE(static struct uxsin *check_unix,(INODETYPE i));
_PROTOTYPE(static void get_ax25,(char *p));
_PROTOTYPE(static void get_ipx,(char *p));
_PROTOTYPE(static void get_raw,(char *p));
_PROTOTYPE(static void get_tcpudp,(char *p, int pr, int clr));
_PROTOTYPE(static void get_unix,(char *p));
_PROTOTYPE(static void print_ax25info,(struct ax25sin *ap));
_PROTOTYPE(static void print_ipxinfo,(struct ipxsin *ip));

#if	defined(HASIPv6)
_PROTOTYPE(static struct rawsin *check_raw6,(INODETYPE i));
_PROTOTYPE(static struct tcp_udp6 *check_tcpudp6,(INODETYPE i, char **p));
_PROTOTYPE(static void get_raw6,(char *p));
_PROTOTYPE(static void get_tcpudp6,(char *p, int pr, int clr));
_PROTOTYPE(static int net6a2in6,(char *as, struct in6_addr *ad));
#endif	/* defined(HASIPv6) */



/*
 * check_ax25() - check for AX25 socket file
 */

static struct ax25sin *
check_ax25(i)
	INODETYPE i;			/* socket file's inode number */
{
	struct ax25sin *ap;
	int h;

	h = INOHASH(i);
	for (ap = AX25sin[h]; ap; ap = ap->next) {
	    if (i == ap->inode)
		return(ap);
	}
	return((struct ax25sin *)NULL);
}


/*
 * check_ipx() - check for IPX socket file
 */

static struct ipxsin *
check_ipx(i)
	INODETYPE i;			/* socket file's inode number */
{
	int h;
	struct ipxsin *ip;

	h = INOHASH(i);
	for (ip = Ipxsin[h]; ip; ip = ip->next) {
	    if (i == ip->inode)
		return(ip);
	}
	return((struct ipxsin *)NULL);
}


/*
 * check_raw() - check for raw socket file
 */

static struct rawsin *
check_raw(i)
	INODETYPE i;			/* socket file's inode number */
{
	int h;
	struct rawsin *rp;

	h = INOHASH(i);
	for (rp = Rawsin[h]; rp; rp = rp->next) {
	    if (i == rp->inode)
		return(rp);
	}
	return((struct rawsin *)NULL);
}


/*
 * check_tcpudp() - check for IPv4 TCP or UDP socket file
 */

static struct tcp_udp *
check_tcpudp(i, p)
	INODETYPE i;			/* socket file's inode number */
	char **p;			/* protocol return */
{
	int h;
	struct tcp_udp *tp;

	h = INOHASH(i);
	for (tp = TcpUdp[h]; tp; tp = tp->next) {
	    if (i == tp->inode) {
		*p = tp->proto ? "UDP" : "TCP";
		return(tp);
	    }
	}
	return((struct tcp_udp *)NULL);
}


#if	defined(HASIPv6)
/*
 * check_raw6() - check for raw IPv6 socket file
 */

static struct rawsin *
check_raw6(i)
	INODETYPE i;			/* socket file's inode number */
{
	int h;
	struct rawsin *rp;

	h = INOHASH(i);
	for (rp = Rawsin6[h]; rp; rp = rp->next) {
	    if (i == rp->inode)
		return(rp);
	}
	return((struct rawsin *)NULL);
}


/*
 * check_tcpudp6() - check for IPv6 TCP or UDP socket file
 */

static struct tcp_udp6 *
check_tcpudp6(i, p)
	INODETYPE i;			/* socket file's inode number */
	char **p;			/* protocol return */
{
	int h;
	struct tcp_udp6 *tp6;

	h = INOHASH(i);
	for (tp6 = TcpUdp6[h]; tp6; tp6 = tp6->next) {
	    if (i == tp6->inode) {
		*p = tp6->proto ? "UDP" : "TCP";
		return(tp6);
	    }
	}
	return((struct tcp_udp6 *)NULL);
}
#endif	/* defined(HASIPv6) */


/*
 * check_unix() - check for UNIX domain socket
 */

static struct uxsin *
check_unix(i)
	INODETYPE i;			/* socket file's inode number */
{
	int h;
	struct uxsin *up;

	h = INOHASH(i);
	for (up = Uxsin[h]; up; up = up->next) {
	    if (i == up->inode)
		return(up);
	}
	return((struct uxsin *)NULL);
}


/*
 * get_ax25() - get /proc/net/ax25 info
 */

static void
get_ax25(p)
	char *p;			/* /proc/net/ipx path */
{
	struct ax25sin *ap, *np;
	FILE *as;
	char buf[MAXPATHLEN], *da, *dev_ch, *ep, **fp, *sa;
	int h, nf;
	unsigned long inode, rq, sq, state;
	MALLOC_S len;
	unsigned char rqs, sqs;
/*
 * Do second time cleanup or first time setup.
 */
	if (AX25sin) {
	    for (h = 0; h < INOBUCKS; h++) {
		for (ap = AX25sin[h]; ap; ap = np) {
		    np = ap->next;
		    if (ap->da)
			(void) free((FREE_P *)ap->da);
		    if (ap->dev_ch)
			(void) free((FREE_P *)ap->dev_ch);
		    if (ap->sa)
			(void) free((FREE_P *)ap->sa);
		    (void) free((FREE_P *)ap);
		}
		AX25sin[h] = (struct ax25sin *)NULL;
	    }
	} else {
	    AX25sin = (struct ax25sin **)calloc(INOBUCKS,
					      sizeof(struct ax25sin *));
	    if (!AX25sin) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d AX25 hash pointer bytes\n",
		    Pn, INOBUCKS * sizeof(struct ax25sin *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net/ax25 file.  Store AX25 socket info in the
 * AX25sin[] hash buckets.
 */
	if (!(as = fopen(p, "r")))
	    return;
	while (fgets(buf, sizeof(buf) - 1, as)) {
	    if ((nf = get_fields(buf, (char *)NULL, &fp, (int *)NULL, 0)) < 24)
		continue;
	/*
	 * /proc/next/ac25 has no title line, a very poor deficiency in its
	 * implementation.
	 *
	 * The ax25_get_info() function in kern mnodule .../net/ax25/af_ax25.c
	 * says the format of the lines in the file is:
	 *
	 *     magic dev src_addr dest_addr,digi1,digi2,.. st vs vr va t1 t1 \
	 *     t2  t2 t3 t3 idle idle n2 n2 rtt window paclen Snd-Q Rcv-Q \
	 *     inode
	 *
	 * The code in this function is forced to assume that format is in
	 * effect..
	 */

	/*
	 * Assemble the inode number and see if it has already been recorded.
	 * If it has, skip this line.
	 */
	    ep = (char *)NULL;
	    if (!fp[23] || !*fp[23]
	    ||  (inode = strtoul(fp[23], &ep, 0)) == ULONG_MAX
	    ||  !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (ap = AX25sin[h]; ap; ap = ap->next) {
		if ((INODETYPE)inode == ap->inode)
		    break;
	    }
	    if (ap)
		continue;
	/*
	 * Assemble the send and receive queue values and the state.
	 */
	    rq = sq = (unsigned long)0;
	    rqs = sqs = (unsigned char)0;
	    ep = (char *)NULL;
	    if (!fp[21] || !*fp[21]
	    ||  (sq = strtoul(fp[21], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	    sqs = (unsigned char)1;
	    ep = (char *)NULL;
	    if (!fp[22] || !*fp[22]
	    ||  (rq = strtoul(fp[22], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	    rqs = (unsigned char)1;
	    ep = (char *)NULL;
	    if (!fp[4] || !*fp[4]
	    ||  (state = strtoul(fp[4], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Allocate space for the destination address.
	 */
	    if (!fp[3] || !*fp[3])
		da = (char *)NULL;
	    else if ((len = strlen(fp[3]))) {
		if (!(da = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
		      "%s: can't allocate %d destination AX25 addr bytes: %s\n",
		      Pn, len + 1, fp[3]);
		    Exit(1);
		}
		(void) snpf(da, len + 1, "%s", fp[3]);
	    } else
		da = (char *)NULL;
	/*
	 * Allocate space for the source address.
	 */
	    if (!fp[2] || !*fp[2])
		sa = (char *)NULL;
	    else if ((len = strlen(fp[2]))) {
		if (!(sa = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d source AX25 address bytes: %s\n",
			Pn, len + 1, fp[2]);
		    Exit(1);
		}
		(void) snpf(sa, len + 1, "%s", fp[2]);
	    } else
		sa = (char *)NULL;
	/*
	 * Allocate space for the device characters.
	 */
	    if (!fp[1] || !*fp[1])
		dev_ch = (char *)NULL;
	    else if ((len = strlen(fp[1]))) {
		if (!(dev_ch = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
		      "%s: can't allocate %d destination AX25 dev bytes: %s\n",
		      Pn, len + 1, fp[1]);
		    Exit(1);
		}
		(void) snpf(dev_ch, len + 1, "%s", fp[1]);
	    } else
		dev_ch = (char *)NULL;
	/*
	 * Allocate space for an ax25sin entry, fill it, and link it to its
	 * hash bucket.
	 */
	    if (!(ap = (struct ax25sin *)malloc(sizeof(struct ax25sin)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d byte ax25sin structure\n",
		    Pn, sizeof(struct ax25sin));
		Exit(1);
	    }
	    ap->da = da;
	    ap->dev_ch = dev_ch;
	    ap->inode = (INODETYPE)inode;
	    ap->rq = rq;
	    ap->rqs = rqs;
	    ap->sa = sa;
	    ap->sq = sq;
	    ap->sqs = sqs;
	    ap->state = (int)state;
	    ap->next = AX25sin[h];
	    AX25sin[h] = ap;
	}
	(void) fclose(as);
}


/*
 * get_ipx() - get /proc/net/ipx info
 */

static void
get_ipx(p)
	char *p;			/* /proc/net/ipx path */
{
	char buf[MAXPATHLEN], *ep, **fp, *la, *ra;
	int fl = 1;
	int h;
	unsigned long inode, rxq, state, txq;
	struct ipxsin *ip, *np;
	MALLOC_S len;
	FILE *xs;
/*
 * Do second time cleanup or first time setup.
 */
	if (Ipxsin) {
	    for (h = 0; h < INOBUCKS; h++) {
		for (ip = Ipxsin[h]; ip; ip = np) {
		    np = ip->next;
		    if (ip->la)
			(void) free((FREE_P *)ip->la);
		    if (ip->ra)
			(void) free((FREE_P *)ip->ra);
		    (void) free((FREE_P *)ip);
		}
		Ipxsin[h] = (struct ipxsin *)NULL;
	    }
	} else {
	    Ipxsin = (struct ipxsin **)calloc(INOBUCKS,
					      sizeof(struct ipxsin *));
	    if (!Ipxsin) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d IPX hash pointer bytes\n",
		    Pn, INOBUCKS * sizeof(struct ipxsin *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net/ipx file.  Store IPX socket info in the
 * Ipxsin[] hash buckets.
 */
	if (!(xs = fopen(p, "r")))
	    return;
	while (fgets(buf, sizeof(buf) - 1, xs)) {
	    if (get_fields(buf, (char *)NULL, &fp, (int *)NULL, 0) < 7)
		continue;
	    if (fl) {

	    /*
	     * Check the column labels in the first line.
	     */
		if (!fp[0] || strcmp(fp[0], "Local_Address")
		||  !fp[1] || strcmp(fp[1], "Remote_Address")
		||  !fp[2] || strcmp(fp[2], "Tx_Queue")
		||  !fp[3] || strcmp(fp[3], "Rx_Queue")
		||  !fp[4] || strcmp(fp[4], "State")
		||  !fp[5] || strcmp(fp[5], "Uid")
		||  !fp[6] || strcmp(fp[6], "Inode"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		fl = 0;
		continue;
	    }
	/*
	 * Assemble the inode number and see if the inode is already
	 * recorded.
	 */
	    ep = (char *)NULL;
	    if (!fp[6] || !*fp[6]
	    ||  (inode = strtoul(fp[6], &ep, 0)) == ULONG_MAX
	    ||  !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (ip = Ipxsin[h]; ip; ip = ip->next) {
		if ((INODETYPE)inode == ip->inode)
		    break;
	    }
	    if (ip)
		continue;
	/*
	 * Assemble the transmit and receive queue values and the state.
	 */
	    ep = (char *)NULL;
	    if (!fp[2] || !*fp[2]
	    ||  (txq = strtoul(fp[2], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[3] || !*fp[3]
	    ||  (rxq = strtoul(fp[3], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[4] || !*fp[4]
	    ||  (state = strtoul(fp[4], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Allocate space for the local address, unless it is "Not_Connected".
	 */
	    if (!fp[0] || !*fp[0] || strcmp(fp[0], "Not_Connected") == 0)
		la = (char *)NULL;
	    else if ((len = strlen(fp[0]))) {
		if (!(la = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d local IPX address bytes: %s\n",
			Pn, len + 1, fp[0]);
		    Exit(1);
		}
		(void) snpf(la, len + 1, "%s", fp[0]);
	    } else
		la = (char *)NULL;
	/*
	 * Allocate space for the remote address, unless it is "Not_Connected".
	 */
	    if (!fp[1] || !*fp[1] || strcmp(fp[1], "Not_Connected") == 0)
		ra = (char *)NULL;
	    else if ((len = strlen(fp[1]))) {
		if (!(ra = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d remote IPX address bytes: %s\n",
			Pn, len + 1, fp[1]);
		    Exit(1);
		}
		(void) snpf(ra, len + 1, "%s", fp[1]);
	    } else
		ra = (char *)NULL;
	/*
	 * Allocate space for an ipxsin entry, fill it, and link it to its
	 * hash bucket.
	 */
	    if (!(ip = (struct ipxsin *)malloc(sizeof(struct ipxsin)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d byte ipxsin structure\n",
		    Pn, sizeof(struct ipxsin));
		Exit(1);
	    }
	    ip->inode = (INODETYPE)inode;
	    ip->la = la;
	    ip->ra = ra;
	    ip->txq = txq;
	    ip->rxq = rxq;
	    ip->state = (int)state;
	    ip->next = Ipxsin[h];
	    Ipxsin[h] = ip;
	}
	(void) fclose(xs);
}


/*
 * get_raw() - get /proc/net/raw info
 */

static void
get_raw(p)
	char *p;			/* /proc/net/raw path */
{
	char buf[MAXPATHLEN], *ep, **fp, *la, *ra, *sp;
	int h;
	unsigned long inode;
	int nf = 12;
	struct rawsin *np, *rp;
	MALLOC_S lal, ral, spl;
	FILE *xs;
/*
 * Do second time cleanup or first time setup.
 */
	if (Rawsin) {
	    for (h = 0; h < INOBUCKS; h++) {
		for (rp = Rawsin[h]; rp; rp = np) {
		    np = rp->next;
		    if (rp->la)
			(void) free((FREE_P *)rp->la);
		    if (rp->ra)
			(void) free((FREE_P *)rp->ra);
		    (void) free((FREE_P *)rp);
		}
		Rawsin[h] = (struct rawsin *)NULL;
	    }
	} else {
	    Rawsin = (struct rawsin **)calloc(INOBUCKS,
					      sizeof(struct rawsin *));
	    if (!Rawsin) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d raw hash pointer bytes\n",
		    Pn, INOBUCKS * sizeof(struct rawsin *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net/raw file.  Store raw socket info in the
 * Rawsin[] hash buckets.
 */
	if (!(xs = fopen(p, "r")))
	    return;
	while (fgets(buf, sizeof(buf) - 1, xs)) {
	    if (get_fields(buf, (char *)NULL, &fp, (int *)NULL, 0) < nf)
		continue;
	    if (nf == 12) {

	    /*
	     * Check the column labels in the first line.
	     */
		if (!fp[1]  || strcmp(fp[1],  "local_address")
		||  !fp[2]  || strcmp(fp[2],  "rem_address")
		||  !fp[3]  || strcmp(fp[3],  "st")
		||  !fp[11] || strcmp(fp[11], "inode"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		nf = 10;
		continue;
	    }
	/*
	 * Assemble the inode number and see if the inode is already
	 * recorded.
	 */
	    ep = (char *)NULL;
	    if (!fp[9] || !*fp[9]
	    ||  (inode = strtoul(fp[9], &ep, 0)) == ULONG_MAX
	    ||  !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (rp = Rawsin[h]; rp; rp = rp->next) {
		if ((INODETYPE)inode == rp->inode)
		    break;
	    }
	    if (rp)
		continue;
	/*
	 * Save the local address, remote address, and state.
	 */
	    if (!fp[1] || !*fp[1] || (lal = strlen(fp[1])) < 1) {
		la = (char *)NULL;
		lal = (MALLOC_S)0;
	    } else {
		if (!(la = (char *)malloc(lal + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d local raw address bytes: %s\n",
			Pn, lal + 1, fp[1]);
		    Exit(1);
		}
		(void) snpf(la, lal + 1, "%s", fp[1]);
	    }
	    if (!fp[2] || !*fp[2] || (ral = strlen(fp[2])) < 1) {
		ra = (char *)NULL;
		ral = (MALLOC_S)0;
	    } else {
		if (!(ra = (char *)malloc(ral + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d remote raw address bytes: %s\n",
			Pn, ral + 1, fp[2]);
		    Exit(1);
		}
		(void) snpf(ra, ral + 1, "%s", fp[2]);
	    }
	    if (!fp[3] || !*fp[3] || (spl = strlen(fp[3])) < 1) {
		sp = (char *)NULL;
		spl = (MALLOC_S)0;
	    } else {
		if (!(sp = (char *)malloc(spl + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d remote raw state bytes: %s\n",
			Pn, spl + 1, fp[2]);
		    Exit(1);
		}
		(void) snpf(sp, spl + 1, "%s", fp[3]);
	    }
	/*
	 * Allocate space for an rawsin entry, fill it, and link it to its
	 * hash bucket.
	 */
	    if (!(rp = (struct rawsin *)malloc(sizeof(struct rawsin)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d byte rawsin structure\n",
		    Pn, sizeof(struct rawsin));
		Exit(1);
	    }
	    rp->inode = (INODETYPE)inode;
	    rp->la = la;
	    rp->lal = lal;
	    rp->ra = ra;
	    rp->ral = ral;
	    rp->sp = sp;
	    rp->spl = spl;
	    rp->next = Rawsin[h];
	    Rawsin[h] = rp;
	}
	(void) fclose(xs);
}


/*
 * get_tcpudp() - get IPv4 TCP or UDP IPv4 net info
 */

static void
get_tcpudp(p, pr, clr)
	char *p;			/* /proc/net/{tcp,udp} path */
	int pr;				/* protocol: 0 = TCP, 1 = UDP */
	int clr;			/* 1 == clear the table */
{
	char buf[MAXPATHLEN], *ep, **fp;
	unsigned long faddr, fport, inode, laddr, lport, rxq, state, txq;
	int h, nf;
	FILE *fs;
	struct tcp_udp *np, *tp;
/*
 * Delete previous table contents.  Allocate a table for the first time.
 */
	if (TcpUdp) {
	    if (clr) {
		for (h = 0; h < INOBUCKS; h++) {
		    for (tp = TcpUdp[h]; tp; tp = np) {
			np = tp->next;
			(void) free((FREE_P *)tp);
		    }
		    TcpUdp[h] = (struct tcp_udp *)NULL;
		}
	    }
	} else {
	    if (!(TcpUdp = (struct tcp_udp **)calloc(INOBUCKS,
						     sizeof(struct tcp_udp *))))
	    {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for TCP&UDP hash buckets\n",
		    Pn, INOBUCKS * sizeof(struct tcp_udp *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net file.
 */
	if (!(fs = fopen(p, "r")))
	    return;
	nf = 12;
	while(fgets(buf, sizeof(buf) - 1, fs)) {
	    if (get_fields(buf,
			   (nf == 12) ? (char *)NULL : ":",
			   &fp, (int *)NULL, 0)
	    < nf)
		continue;
	    if (nf == 12) {
		if (!fp[1]  || strcmp(fp[1],  "local_address")
		||  !fp[2]  || strcmp(fp[2],  "rem_address")
		||  !fp[3]  || strcmp(fp[3],  "st")
		||  !fp[4]  || strcmp(fp[4],  "tx_queue")
		||  !fp[5]  || strcmp(fp[5],  "rx_queue")
		||  !fp[11] || strcmp(fp[11], "inode"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		nf = 14;
		continue;
	    }
	/*
	 * Get the local and remote addresses.
	 */
	    ep = (char *)NULL;
	    if (!fp[1] || !*fp[1]
	    ||  (laddr = strtoul(fp[1], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[2] || !*fp[2]
	    ||  (lport = strtoul(fp[2], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[3] || !*fp[3]
	    ||  (faddr = strtoul(fp[3], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[4] || !*fp[4]
	    ||  (fport = strtoul(fp[4], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Get the state, queue sizes, and inode.
	 */
	    ep = (char *)NULL;
	    if (!fp[5] || !*fp[5]
	    ||  (state = strtoul(fp[5], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[6] || !*fp[6]
	    ||  (txq = strtoul(fp[6], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[7] || !*fp[7]
	    ||  (rxq = strtoul(fp[7], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Get the inode and use it for hashing and searching.
	 */
	    ep = (char *)NULL;
	    if (!fp[13] || !*fp[13]
	    ||  (inode = strtoul(fp[13], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (tp = TcpUdp[h]; tp; tp = tp->next) {
		if (tp->inode == (INODETYPE)inode)
		    break;
	    }
	    if (tp)
		continue;
	/*
	 * Create a new entry and link it to its hash bucket.
	 */
	    if (!(tp = (struct tcp_udp *)malloc(sizeof(struct tcp_udp)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for tcp_udp struct\n",
		    Pn, sizeof(struct tcp_udp));
		Exit(1);
	    }
	    tp->inode = (INODETYPE)inode;
	    tp->faddr = faddr;
	    tp->fport = (int)(fport & 0xffff);
	    tp->laddr = laddr;
	    tp->lport = (int)(lport & 0xffff);
	    tp->txq = txq;
	    tp->rxq = rxq;
	    tp->proto = pr;
	    tp->state = (int)state;
	    tp->next = TcpUdp[h];
	    TcpUdp[h] = tp;
	}
	(void) fclose(fs);
}


#if	defined(HASIPv6)
/*
 * get_raw6() - get /proc/net/raw6 info
 */

static void
get_raw6(p)
	char *p;			/* /proc/net/raw path */
{
	char buf[MAXPATHLEN], *ep, **fp, *la, *ra, *sp;
	int h;
	unsigned long inode;
	int nf = 12;
	struct rawsin *np, *rp;
	MALLOC_S lal, ral, spl;
	FILE *xs;
/*
 * Do second time cleanup or first time setup.
 */
	if (Rawsin6) {
	    for (h = 0; h < INOBUCKS; h++) {
		for (rp = Rawsin6[h]; rp; rp = np) {
		    np = rp->next;
		    if (rp->la)
			(void) free((FREE_P *)rp->la);
		    if (rp->ra)
			(void) free((FREE_P *)rp->ra);
		    (void) free((FREE_P *)rp);
		}
		Rawsin6[h] = (struct rawsin *)NULL;
	    }
	} else {
	    Rawsin6 = (struct rawsin **)calloc(INOBUCKS,
					       sizeof(struct rawsin *));
	    if (!Rawsin6) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d raw6 hash pointer bytes\n",
		    Pn, INOBUCKS * sizeof(struct rawsin *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net/raw6 file.  Store raw6 socket info in the
 * Rawsin6[] hash buckets.
 */
	if (!(xs = fopen(p, "r")))
	    return;
	while (fgets(buf, sizeof(buf) - 1, xs)) {
	    if (get_fields(buf, (char *)NULL, &fp, (int *)NULL, 0) < nf)
		continue;
	    if (nf == 12) {

	    /*
	     * Check the column labels in the first line.
	     */
		if (!fp[1]  || strcmp(fp[1],  "local_address")
		||  !fp[2]  || strcmp(fp[2],  "remote_address")
		||  !fp[3]  || strcmp(fp[3],  "st")
		||  !fp[11] || strcmp(fp[11], "inode"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		nf = 10;
		continue;
	    }
	/*
	 * Assemble the inode number and see if the inode is already
	 * recorded.
	 */
	    ep = (char *)NULL;
	    if (!fp[9] || !*fp[9]
	    ||  (inode = strtoul(fp[9], &ep, 0)) == ULONG_MAX
	    ||  !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (rp = Rawsin6[h]; rp; rp = rp->next) {
		if ((INODETYPE)inode == rp->inode)
		    break;
	    }
	    if (rp)
		continue;
	/*
	 * Save the local address, remote address, and state.
	 */
	    if (!fp[1] || !*fp[1] || (lal = strlen(fp[1])) < 1) {
		la = (char *)NULL;
		lal = (MALLOC_S)0;
	    } else {
		if (!(la = (char *)malloc(lal + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d local raw6 address bytes: %s\n",
			Pn, lal + 1, fp[1]);
		    Exit(1);
		}
		(void) snpf(la, lal + 1, "%s", fp[1]);
	    }
	    if (!fp[2] || !*fp[2] || (ral = strlen(fp[2])) < 1) {
		ra = (char *)NULL;
		ral = (MALLOC_S)0;
	    } else {
		if (!(ra = (char *)malloc(ral + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d remote raw6 address bytes: %s\n",
			Pn, ral + 1, fp[2]);
		    Exit(1);
		}
		(void) snpf(ra, ral + 1, "%s", fp[2]);
	    }
	    if (!fp[3] || !*fp[3] || (spl = strlen(fp[3])) < 1) {
		sp = (char *)NULL;
		spl = (MALLOC_S)0;
	    } else {
		if (!(sp = (char *)malloc(spl + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d remote raw6 state bytes: %s\n",
			Pn, spl + 1, fp[2]);
		    Exit(1);
		}
		(void) snpf(sp, spl + 1, "%s", fp[3]);
	    }
	/*
	 * Allocate space for an rawsin entry, fill it, and link it to its
	 * hash bucket.
	 */
	    if (!(rp = (struct rawsin *)malloc(sizeof(struct rawsin)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d byte rawsin structure for IPv6\n",
		    Pn, sizeof(struct rawsin));
		Exit(1);
	    }
	    rp->inode = (INODETYPE)inode;
	    rp->la = la;
	    rp->lal = lal;
	    rp->ra = ra;
	    rp->ral = ral;
	    rp->sp = sp;
	    rp->spl = spl;
	    rp->next = Rawsin6[h];
	    Rawsin6[h] = rp;
	}
	(void) fclose(xs);
}


/*
 * get_tcpudp6() - get IPv6 TCP or UDP IPv4 net info
 */

static void
get_tcpudp6(p, pr, clr)
	char *p;			/* /proc/net/{tcp,udp} path */
	int pr;				/* protocol: 0 = TCP, 1 = UDP */
	int clr;			/* 1 == clear the table */
{
	char buf[MAXPATHLEN], *ep, **fp;
	struct in6_addr faddr, laddr;
	unsigned long fport, inode, lport, rxq, state, txq;
	int h, nf;
	FILE *fs;
	struct tcp_udp6 *np6, *tp6;
/*
 * Delete previous table contents.  Allocate a table for the first time.
 */
	if (TcpUdp6) {
	    if (clr) {
		for (h = 0; h < INOBUCKS; h++) {
		    for (tp6 = TcpUdp6[h]; tp6; tp6 = np6) {
			np6 = tp6->next;
			(void) free((FREE_P *)tp6);
		    }
		    TcpUdp6[h] = (struct tcp_udp6 *)NULL;
		}
	    }
	} else {
	    if (!(TcpUdp6 = (struct tcp_udp6 **)calloc(INOBUCKS,
						sizeof(struct tcp_udp6 *))))
	    {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for TCP6&UDP6 hash buckets\n",
		    Pn, INOBUCKS * sizeof(struct tcp_udp6 *));
		Exit(1);
	    }
	}
/*
 * Open and read the /proc/net file.
 */
	if (!(fs = fopen(p, "r")))
	    return;
	nf = 12;
	while(fgets(buf, sizeof(buf) - 1, fs)) {
	    if (get_fields(buf,
			   (nf == 12) ? (char *)NULL : ":",
			   &fp, (int *)NULL, 0)
	    < nf)
		continue;
	    if (nf == 12) {
		if (!fp[1]  || strcmp(fp[1],  "local_address")
		||  !fp[2]  || strcmp(fp[2],  "remote_address")
		||  !fp[3]  || strcmp(fp[3],  "st")
		||  !fp[4]  || strcmp(fp[4],  "tx_queue")
		||  !fp[5]  || strcmp(fp[5],  "rx_queue")
		||  !fp[11] || strcmp(fp[11], "inode"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		nf = 14;
		continue;
	    }
	/*
	 * Get the local and remote addresses.
	 */
	    if (!fp[1] || !*fp[1] || net6a2in6(fp[1], &laddr))
		continue;
	    ep = (char *)NULL;
	    if (!fp[2] || !*fp[2]
	    ||  (lport = strtoul(fp[2], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    if (!fp[3] || !*fp[3] || net6a2in6(fp[3], &faddr))
		continue;
	    ep = (char *)NULL;
	    if (!fp[4] || !*fp[4]
	    ||  (fport = strtoul(fp[4], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Get the state, queue sizes, and inode.
	 */
	    ep = (char *)NULL;
	    if (!fp[5] || !*fp[5]
	    ||  (state = strtoul(fp[5], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[6] || !*fp[6]
	    ||  (txq = strtoul(fp[6], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[7] || !*fp[7]
	    ||  (rxq = strtoul(fp[7], &ep, 16)) == ULONG_MAX || !ep || *ep)
		continue;
	/*
	 * Get the inode and use it for hashing and searching.
	 */
	    ep = (char *)NULL;
	    if (!fp[13] || !*fp[13]
	    ||  (inode = strtoul(fp[13], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (tp6 = TcpUdp6[h]; tp6; tp6 = tp6->next) {
		if (tp6->inode == (INODETYPE)inode)
		    break;
	    }
	    if (tp6)
		continue;
	/*
	 * Create a new entry and link it to its hash bucket.
	 */
	    if (!(tp6 = (struct tcp_udp6 *)malloc(sizeof(struct tcp_udp6)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for tcp_udp6 struct\n",
		    Pn, sizeof(struct tcp_udp6));
		Exit(1);
	    }
	    tp6->inode = (INODETYPE)inode;
	    tp6->faddr = faddr;
	    tp6->fport = (int)(fport & 0xffff);
	    tp6->laddr = laddr;
	    tp6->lport = (int)(lport & 0xffff);
	    tp6->txq = txq;
	    tp6->rxq = rxq;
	    tp6->proto = pr;
	    tp6->state = (int)state;
	    tp6->next = TcpUdp6[h];
	    TcpUdp6[h] = tp6;
	}
	(void) fclose(fs);
}
#endif	/* defined(HASIPv6) */


/*
 * get_unix() - get UNIX net info
 */

static void
get_unix(p)
	char *p;			/* /proc/net/unix path */
{
	char buf[MAXPATHLEN], *ep, **fp, *path, *pcb;
	int fl = 1;
	int h, nf;
	unsigned long inode;
	MALLOC_S len;
	struct uxsin *np, *up;
	FILE *us;
/*
 * Do second time cleanup or first time setup.
 */
	if (Uxsin) {
	    for (h = 0; h < INOBUCKS; h++) {
		for (up = Uxsin[h]; up; up = np) {
		    np = up->next;
		    if (up->path)
			(void) free((FREE_P *)up->path);
		    (void) free((FREE_P *)up);
		}
		Uxsin[h] = (struct uxsin *)NULL;
	    }
	} else {
	    Uxsin = (struct uxsin **)calloc(INOBUCKS, sizeof(struct uxsin *));
	    if (!Uxsin) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for Unix socket info\n",
		    Pn, INOBUCKS * sizeof(struct uxsin *));
	    }
	}
/*
 * Open /proc/net/unix, read its contents, and add them to the Uxsin hash
 * buckets.
 */
	if (!(us = fopen(p, "r")))
	    return;
	while (fgets(buf, sizeof(buf) - 1, us)) {
	    if ((nf = get_fields(buf, ":", &fp, (int *)NULL, 0)) < 7)
		continue;
	    if (fl) {

	    /*
	     * Check the first line for header words.
	     */
		if (!fp[0] || strcmp(fp[0], "Num")
		||  !fp[1] || strcmp(fp[1], "RefCount")
		||  !fp[2] || strcmp(fp[2], "Protocol")
		||  !fp[3] || strcmp(fp[3], "Flags")
		||  !fp[4] || strcmp(fp[4], "Type")
		||  !fp[5] || strcmp(fp[5], "St")
		||  !fp[6] || strcmp(fp[6], "Inode")
		||  nf < 8
		||  !fp[7] || strcmp(fp[7], "Path"))
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: WARNING: unsupported format: %s\n",
			    Pn, p);
		    }
		    break;
		}
		fl = 0;
		continue;
	    }
	/*
	 * Assemble PCB address, inode number, and path name.  If this
	 * inode is already represented in Uxsin, skip it.
	 */
	    ep = (char *)NULL;
	    if (!fp[6] || !*fp[6]
	    ||  (inode = strtoul(fp[6], &ep, 0)) == ULONG_MAX || !ep || *ep)
		continue;
	    h = INOHASH((INODETYPE)inode);
	    for (up = Uxsin[h]; up; up = up->next) {
		if ((INODETYPE)inode == up->inode)
		    break;
	    }
	    if (up)
		continue;
	    if (!fp[0] || !*fp[0])
		pcb = (char *)NULL;
	    else {
		len = strlen(fp[0]) + 2;
		if (!(pcb = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for UNIX PCB: %s\n",
			Pn, len + 1, fp[0]);
		    Exit(1);
		}
		(void) snpf(pcb, len + 1, "0x%s", fp[0]);
	    }
	    if (nf >= 8
	    &&  fp[7] && *fp[7] && *fp[7] != '@'
	    &&  (len = strlen(fp[7]))) {
		if (!(path = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for UNIX path \"%s\"\n",
			Pn, len + 1, fp[7]);
		    Exit(1);
		}
		(void) snpf(path, len + 1, "%s", fp[7]);
	    } else
		path = (char *)NULL;
	/*
	 * Allocate and fill a Unix socket info structure; link it to its
	 * hash bucket.
	 */
	    if (!(up = (struct uxsin *)malloc(sizeof(struct uxsin)))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for uxsin struct\n",
		    Pn, sizeof(struct uxsin));
		Exit(1);
	    }
	    up->inode = (INODETYPE)inode;
	    up->pcb = pcb;
	    up->sb_def = 0;
	    if ((up->path = path) && (*path == '/')) {

	    /*
	     * If an absolute path (i.e., one that begins with a '/') exists
	     * for the line, attempt to stat(2) it and save the device and
	     * node numbers reported in the stat buffer.
	     */
		struct stat sb;
		int sr;

		if (HasNFS)
		    sr = statsafely(path, &sb);
		else
		    sr = stat(path, &sb);
		if (sr && ((sb.st_mode & S_IFMT) == S_IFSOCK)) {
		    up->sb_def = 1;
		    up->sb_dev = sb.st_dev;
		    up->sb_ino = (INODETYPE)sb.st_ino;
		    up->sb_rdev = sb.st_rdev;
		}
	    }
	    up->next = Uxsin[h];
	    Uxsin[h] = up;
	}
	(void) fclose(us);
}


#if	defined(HASIPv6)
/*
 * net6a2in6() - convert ASCII IPv6 address in /proc/net/{tcp,udp} form to
 *		 an in6_addr
 */

static int
net6a2in6(as, ad)
	char *as;			/* address source */
	struct in6_addr *ad;		/* address destination */
{
	char buf[9], *ep;
	int i;
	size_t len;
/*
 * Assemble four uint32_t's from 4 X 8 hex digits into s6_addr32[].
 */
	for (i = 0, len = strlen(as);
	     (i < 4) && (len >= 8);
	     as += 8, i++, len -= 8)
	{
	    (void) strncpy(buf, as, 8);
	    buf[8] = '\0';
	    ep = (char *)NULL;
	    if ((ad->s6_addr32[i] = (uint32_t)strtoul(buf, &ep, 16))
	    ==  (uint32_t)UINT32_MAX || !ep || *ep)
		break;
	}
	return((*as || (i != 4) || len) ? 1 : 0);
}
#endif	/* defined(HASIPv6) */


/*
 * print_ax25info() - print AX25 socket info
 */

static void
print_ax25info(ap)
	struct ax25sin *ap;		/* AX25 socket info */
{
	char *cp, pbuf[1024];
	int ds;
	MALLOC_S pl = (MALLOC_S)0;

	if (Lf->nma)
	    return;
	if (ap->sa) {
	    ds = (ap->da && strcmp(ap->da, "*")) ? 1 : 0;
	    (void) snpf(&pbuf[pl], sizeof(pbuf) - pl, "%s%s%s ", ap->sa,
		ds ? "->" : "",
		ds ? ap->da : "");
	    pl = strlen(pbuf);
	}
	if (ap->sqs) {
	    (void) snpf(&pbuf[pl], sizeof(pbuf) - pl, "(Sq=%lu ", ap->sq);
	    pl = strlen(pbuf);
	    cp = "";
	} else 
	    cp = "(";
	if (ap->rqs) {
	    (void) snpf(&pbuf[pl], sizeof(pbuf) - pl, "%sRq=%lu ", cp, ap->rq);
	    pl = strlen(pbuf);
	    cp = "";
	}
	(void) snpf(&pbuf[pl], sizeof(pbuf) - pl, "%sState=%d", cp, ap->state);
	pl = strlen(pbuf);
	if ((ap->state >= 0) && (ap->state < NAX25ST))
	    cp = ax25st[ap->state];
	else
	    cp = NULL;
	(void) snpf(&pbuf[pl], sizeof(pbuf) - pl, "%s%s)",
	    cp ? ", " : "",
	    cp ? cp : "");
	pl = strlen(pbuf);
	if (!(cp = (char *)malloc(pl + 1))) {
	    (void) fprintf(stderr,
		"%s: can't allocate %d bytes for AX25 sock state, PID: %d\n",
		Pn, pl + 1, Lp->pid);
	    Exit(1);
	}
	(void) snpf(cp, pl + 1, "%s", pbuf);
	Lf->nma = cp;
}


/*
 * print_ipxinfo() - print IPX socket info
 */

static void
print_ipxinfo(ip)
	struct ipxsin *ip;		/* IPX socket info */
{
	char *cp, pbuf[256];
	MALLOC_S pl;

	if (Lf->nma)
	    return;
	(void) snpf(pbuf, sizeof(pbuf), "(Tx=%lx Rx=%lx State=%02x)",
	    ip->txq, ip->rxq, ip->state);
	pl = strlen(pbuf);
	if (!(cp = (char *)malloc(pl + 1))) {
	    (void) fprintf(stderr,
		"%s: can't allocate %d bytes for IPX sock state, PID: %d\n",
		Pn, pl + 1, Lp->pid);
	    Exit(1);
	}
	(void) snpf(cp, pl + 1, "%s", pbuf);
	Lf->nma = cp;
}


/*
 * print_tcptpi() - print TCP/TPI state
 */

void
print_tcptpi(nl)
	int nl;				/* 1 == '\n' required */
{
	char buf[128];
	char *cp = (char *)NULL;
	int ps = 0;
	int s;

	if ((Ftcptpi & TCPTPI_STATE) && Lf->lts.type == 0) {
	    switch ((s = Lf->lts.state.i)) {
	    case TCP_ESTABLISHED:
		cp = "ESTABLISHED";
		break;
	    case TCP_SYN_SENT:
		cp = "SYN_SENT";
		break;
	    case TCP_SYN_RECV:
		cp = "SYN_RECV";
		break;
	    case TCP_FIN_WAIT1:
		cp = "FIN_WAIT1";
		break;
	    case TCP_FIN_WAIT2:
		cp = "FIN_WAIT2";
		break;
	    case TCP_TIME_WAIT:
		cp = "TIME_WAIT";
		break;
	    case TCP_CLOSE:
		cp = "CLOSE";
		break;
	    case TCP_CLOSE_WAIT:
		cp = "CLOSE_WAIT";
		break;
	    case TCP_LAST_ACK:
		cp = "LAST_ACK";
		break;
	    case TCP_LISTEN:
		cp = "LISTEN";
		break;
	    case TCP_CLOSING:
		cp = "CLOSING";
		break;
	    case 0:
		cp = "CLOSED";
		break;
	    default:
		(void) snpf(buf, sizeof(buf), "UNKNOWN_TCP_STATE_%d", s);
		cp = buf;
    	    }
	    if (cp) {
		if (Ffield)
		    (void) printf("%cST=%s%c", LSOF_FID_TCPTPI, cp, Terminator);
		else {
		    putchar('(');
		    (void) fputs(cp, stdout);
		}
		ps++;
	    }
	}

# if	defined(HASTCPTPIQ)
	if (Ftcptpi & TCPTPI_QUEUES) {
	    if (Lf->lts.rqs) {
		if (Ffield)
		    putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("QR=%lu", Lf->lts.rq);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	    if (Lf->lts.sqs) {
		if (Ffield)
		    putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("QS=%lu", Lf->lts.sq);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	}
# endif	/* defined(HASTCPTPIQ) */

# if	defined(HASTCPTPIW)
	if (Ftcptpi & TCPTPI_WINDOWS) {
	    if (Lf->lts.rws) {
		if (Ffield)
		    putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("WR=%lu", Lf->lts.rw);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	    if (Lf->lts.wws) {
		if (Ffield)
		    putchar(LSOF_FID_TCPTPI);
		else {
		    if (ps)
			putchar(' ');
		    else
			putchar('(');
		}
		(void) printf("WW=%lu", Lf->lts.ww);
		if (Ffield)
		    putchar(Terminator);
		ps++;
	    }
	}
# endif	/* defined(HASTCPTPIW) */

	if (!Ffield && ps)
	    putchar(')');
	if (nl)
	    putchar('\n');
}


/*
 * process_proc_sock() - process /proc-based socket
 */

void
process_proc_sock(p, s, ss, l, lss)
	char *p;			/* node's readlink() path */
	struct stat *s;			/* stat() result for path */
	int ss;				/* *s status -- i.e, SB_* values */
	struct stat *l;			/* lstat() result for FD (NULL for
					 * others) */
	int lss;				/* *l status -- i.e, SB_* values */
{
	struct ax25sin *ap;
	char *cp, dev_ch[32], *path;
	unsigned char *fa, *la;
	struct in_addr fs, ls;
	struct ipxsin *ip;
	int len, nl;
	char *pr;
	struct rawsin *rp;
	struct tcp_udp *tp;
	struct uxsin *up;

#if	defined(HASIPv6)
	int af;
	struct tcp_udp6 *tp6;
#endif	/* defined(HASIPv6) */

/*
 * Enter offset, if possible.
 */
	if (Foffset || !Fsize) {
	    if (l && (lss & SB_SIZE)) {
		Lf->off = (SZOFFTYPE)l->st_size;
		Lf->off_def = 1;
	    }
	}
/*
 * Check for socket's inode presence in the protocol info caches.
 */
	if (AX25path) {
	    (void) get_ax25(AX25path);
	    (void) free((FREE_P *)AX25path);
	    AX25path = (char *)NULL;
	}
	if ((ss & SB_INO)
	&&  (ap = check_ax25((INODETYPE)s->st_ino))
	) {
	
	/*
	 * The inode is connected to an AX25 /proc record.
	 *
	 * Set the type to "ax25"; save the device name; save the inode number;
	 * save the destination and source addresses; save the send and receive
	 * queue sizes; and save the connection state.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "ax25");
	    if (ap->dev_ch)
		(void) enter_dev_ch(ap->dev_ch);
	    Lf->inode = ap->inode;
	    Lf->inp_ty = 1;
	    print_ax25info(ap);
	    return;
	}
	if (Ipxpath) {
	    (void) get_ipx(Ipxpath);
	    (void) free((FREE_P *)Ipxpath);
	    Ipxpath = (char *)NULL;
	}
	if ((ss & SB_INO)
	&&  (ip = check_ipx((INODETYPE)s->st_ino))
	) {

	/*
	 * The inode is connected to an IPX /proc record.
	 *
	 * Set the type to "ipx"; enter the inode and device numbers; store
	 * the addresses, queue sizes, and state in the NAME column.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "ipx");
	    if (ss & SB_INO) {
		Lf->inode = (INODETYPE)s->st_ino;
		Lf->inp_ty = 1;
	    }
	    if (ss & SB_DEV) {
		Lf->dev = s->st_dev;
		Lf->dev_def = 1;
	    }
	    cp = Namech;
	    nl = MAXPATHLEN - 2;
	    if (ip->la && nl) {

	    /*
	     * Store the local IPX address.
	     */
		len = strlen(ip->la);
		if (len > nl)
		    len = nl;
		(void) strncpy(cp, ip->la, len);
		cp += len;
		*cp = '\0';
		nl -= len;
	    }
	    if (ip->ra && nl) {

	    /*
	     * Store the remote IPX address, prefixed with "->".
	     */
		if (nl > 2) {
		    (void) snpf(cp, nl, "->");
		    cp += 2;
		    nl -= 2;
		}
		if (nl) {
		    (void) snpf(cp, nl, "%s", ip->ra);
		    cp += len;
		    nl -= len;
		}
	    }
	    (void) print_ipxinfo(ip);
	    if (Namech[0])
		enter_nm(Namech);
	    return;
	}
	if (Rawpath) {
	    (void) get_raw(Rawpath);
	    (void) free((FREE_P *)Rawpath);
	    Rawpath = (char *)NULL;
	}
	if ((ss & SB_INO)
	&&  (rp = check_raw((INODETYPE)s->st_ino))
	) {

	/*
	 * The inode is connected to a raw /proc record.
	 *
	 * Set the type to "raw"; enter the inode number; store the local
	 * address, remote address, and state in the NAME column.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "raw");
	    if (ss & SB_INO) {
		Lf->inode = (INODETYPE)s->st_ino;
		Lf->inp_ty = 1;
	    }
	    cp = Namech;
	    nl = MAXPATHLEN - 2;
	    if (rp->la && rp->lal) {

	    /*
	     * Store the local raw address.
	     */
		if (nl > rp->lal) {
		    (void) snpf(cp, nl, "%s", rp->la);
		    cp += rp->lal;
		    *cp = '\0';
		    nl -= rp->lal;
		}
	    }
	    if (rp->ra && rp->ral) {

	    /*
	     * Store the remote raw address, prefixed with "->".
	     */
		if (nl > (rp->ral + 2)) {
		    (void) snpf(cp, nl, "->%s", rp->ra);
		    cp += (rp->ral + 2);
		    nl -= (rp->ral + 2);
		}
	    }
	    if (rp->sp && rp->spl) {

	    /*
	     * Store the state, optionally prefixed by a space, in the
	     * form "st=x...x".
	     */
	    
		if (nl > (len = ((cp == Namech) ? 0 : 1) + 3 + rp->spl)) {
		    (void) snpf(cp, nl, "%sst=%s",
			(cp == Namech) ? "" : " ", rp->sp);
		    cp += len;
		    *cp = '\0';
		    nl -= len;
		}
	    }
	    if (Namech[0])
		enter_nm(Namech);
	    return;
	}
	if (UNIXpath) {
	    (void) get_unix(UNIXpath);
	    (void) free((FREE_P *)UNIXpath);
	    UNIXpath = (char *)NULL;
	}
	if ((ss & SB_INO)
	&&  (up = check_unix((INODETYPE)s->st_ino))
	) {

	/*
	 * The inode is connected to a UNIX /proc record.
	 *
	 * Set the type to "unix"; enter the PCB address in the DEVICE column;
	 * enter the inode number; and save the optional path.
	 */
	    if (Funix)
		Lf->sf |= SELUNX;
	    (void) snpf(Lf->type, sizeof(Lf->type), "unix");
	    if (up->pcb)
		enter_dev_ch(up->pcb);
	    if (ss & SB_INO) {
		Lf->inode = (INODETYPE)s->st_ino;
		Lf->inp_ty = 1;
	    }
	    path = up->path ? up->path : p;
	    (void) enter_nm(path);
	    if (Sfile) {
	    
	    /*
	     * See if this UNIX domain socket was specified as a search
	     * argument.
	     *
	     * Search first by device and node numbers, if that is possible;
	     * then search by name.
	     */
		unsigned char f = 0;		/* file-found flag */

		if (up->sb_def) {

		/*
		 * If the UNIX socket information includes stat(2) results, do
		 * a device and node number search.
		 *
		 * Note: that requires the saving, temporary modification and
		 *	 restoration of some *Lf values.
		 */
		    unsigned char sv_dev_def;	/* saved dev_def */
		    unsigned char sv_inp_ty;	/* saved inp_ty */
		    unsigned char sv_rdev_def;	/* saved rdev_def */
		    dev_t sv_dev;		/* saved dev */
		    INODETYPE sv_inode;		/* saved inode */
		    dev_t sv_rdev;		/* saved rdev */

		    sv_dev_def = Lf->dev_def;
		    sv_dev = Lf->dev;
		    sv_inode = Lf->inode;
		    sv_inp_ty = Lf->inp_ty;
		    sv_rdev_def = Lf->rdev_def;
		    sv_rdev = Lf->rdev;
		    Lf->dev_def = Lf->inp_ty = Lf->rdev_def = 1;
		    Lf->dev = up->sb_dev;
		    Lf->inode = up->sb_ino;
		    Lf->rdev = up->sb_rdev;
		    if (is_file_named((char *)NULL, 0)) {
			f = 1;
			Lf->sf |= SELNM;
		    }
		    Lf->dev_def = sv_dev_def;
		    Lf->dev = sv_dev;
		    Lf->inode = sv_inode;
		    Lf->inp_ty = sv_inp_ty;
		    Lf->rdev_def = sv_rdev_def;
		    Lf->rdev = sv_rdev;
		}
		if (!f && (ss & SB_MODE)) {

		/*
		 * If the file has not yet been found and the stat buffer has
		 * st_mode, search for the file by full path.
		 */
		    if (is_file_named(path,
			((s->st_mode & S_IFMT) == S_IFCHR)) ? 1 : 0)
		    {
			Lf->sf |= SELNM;
		    }
		}
	    }
	    return;
	}

#if	defined(HASIPv6)
	if (Raw6path) {
	    if (!Fxopt)
		(void) get_raw6(Raw6path);
	    (void) free((FREE_P *)Raw6path);
	    Raw6path = (char *)NULL;
	}
	if (!Fxopt && (ss & SB_INO)
	&&  (rp = check_raw6((INODETYPE)s->st_ino))
	) {

	/*
	 * The inode is connected to a raw IPv6 /proc record.
	 *
	 * Set the type to "raw6"; enter the inode number; store the local
	 * address, remote address, and state in the NAME column.
	 */
	    (void) snpf(Lf->type, sizeof(Lf->type), "raw6");
	    if (ss & SB_INO) {
		Lf->inode = (INODETYPE)s->st_ino;
		Lf->inp_ty = 1;
	    }
	    cp = Namech;
	    nl = MAXPATHLEN - 2;
	    if (rp->la && rp->lal) {

	    /*
	     * Store the local raw IPv6 address.
	     */
		if (nl > rp->lal) {
		    (void) snpf(cp, nl, "%s", rp->la);
		    cp += rp->lal;
		    *cp = '\0';
		    nl -= rp->lal;
		}
	    }
	    if (rp->ra && rp->ral) {

	    /*
	     * Store the remote raw address, prefixed with "->".
	     */
		if (nl > (rp->ral + 2)) {
		    (void) snpf(cp, nl, "->%s", rp->ra);
		    cp += (rp->ral + 2);
		    nl -= (rp->ral + 2);
		}
	    }
	    if (rp->sp && rp->spl) {

	    /*
	     * Store the state, optionally prefixed by a space, in the
	     * form "st=x...x".
	     */
	    
		if (nl > (len = ((cp == Namech) ? 0 : 1) + 3 + rp->spl)) {
		    (void) snpf(cp, nl, "%sst=%s",
			(cp == Namech) ? "" : " ", rp->sp);
		    cp += len;
		    *cp = '\0';
		    nl -= len;
		}
	    }
	    if (Namech[0])
		enter_nm(Namech);
	    return;
	}
	if (TCP6path) {
	    if (!Fxopt)
		(void) get_tcpudp6(TCP6path, 0, 1);
	    (void) free((FREE_P *)TCP6path);
	    TCP6path = (char *)NULL;
	}
	if (UDP6path) {
	    if (!Fxopt)
		(void) get_tcpudp6(UDP6path, 1, 0);
	    (void) free((FREE_P *)UDP6path);
	    UDP6path = (char *)NULL;
	}
	if (!Fxopt && (ss & SB_INO)
	&&  (tp6 = check_tcpudp6((INODETYPE)s->st_ino, &pr))
	) {

	/*
	 * The inode is connected to an IPv6 TCP or UDP /proc record.
	 *
	 * Set the type to "IPv6"; enter the protocol; put the inode number
	 * in the DEVICE column in lieu of the PCB address; save the local
	 * and foreign IPv6 addresses; save the type and protocol; and
	 * (optionally) save the queue sizes.
	 */
	    if (Fnet && (FnetTy != 4))
		Lf->sf |= SELNET;
	    (void) snpf(Lf->type, sizeof(Lf->type), "IPv6");
	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "%.*s", IPROTOL-1, pr);
	    Lf->inp_ty = 2;
	    if (ss && SB_INO) {
		(void) snpf(dev_ch, sizeof(dev_ch), InodeFmt_d,
		    (INODETYPE)s->st_ino);
		enter_dev_ch(dev_ch);
	    }
	    af = AF_INET6;
	    if (!IN6_IS_ADDR_UNSPECIFIED(&tp6->faddr) || tp6->fport)
		fa = (unsigned char *)&tp6->faddr;
	    else
		fa = (unsigned char *)NULL;
	    if (!IN6_IS_ADDR_UNSPECIFIED(&tp6->laddr) || tp6->lport)
		la = (unsigned char *)&tp6->laddr;
	    else
		la = (unsigned char *)NULL;
	    if ((fa && IN6_IS_ADDR_V4MAPPED(&tp6->faddr))
	    ||  (la && IN6_IS_ADDR_V4MAPPED(&tp6->laddr))) {
		af = AF_INET;
		if (fa)
		    fa += 12;
		if (la)
		    la += 12;
	    }
	    ent_inaddr(la, tp6->lport, fa, tp6->fport, af);
	    Lf->lts.type = tp6->proto;
	    Lf->lts.state.i = tp6->state;

#if     defined(HASTCPTPIQ)
	    Lf->lts.rq = tp6->rxq;
	    Lf->lts.sq = tp6->txq;
	    Lf->lts.rqs = Lf->lts.sqs = 1;
#endif  /* defined(HASTCPTPIQ) */

	    return;
	}
#endif	/* defined(HASIPv6) */

	if (TCPpath) {
	    if (!Fxopt)
		(void) get_tcpudp(TCPpath, 0, 1);
	    (void) free((FREE_P *)TCPpath);
	    TCPpath = (char *)NULL;
	}
	if (UDPpath) {
	    if (!Fxopt)
		(void) get_tcpudp(UDPpath, 1, 0);
	    (void) free((FREE_P *)UDPpath);
	    UDPpath = (char *)NULL;
	}
	if (!Fxopt && (ss & SB_INO)
	&&  (tp = check_tcpudp((INODETYPE)s->st_ino, &pr))
	) {

	/*
	 * The inode is connected to an IPv4 TCP or UDP /proc record.
	 *
	 * Set the type to "inet" or "IPv4"; enter the protocol; put the
	 * inode number in the DEVICE column in lieu of the PCB address;
	 * save the local and foreign IPv4 addresses; save the type and
	 * protocol; and (optionally) save the queue sizes.
	 */
	    if (Fnet && (FnetTy != 6))
		Lf->sf |= SELNET;

#if	defined(HASIPv6)
	    (void) snpf(Lf->type, sizeof(Lf->type), "IPv4");
#else	/* !defined(HASIPv6) */
	    (void) snpf(Lf->type, sizeof(Lf->type), "inet");
#endif	/* defined(HASIPv6) */

	    (void) snpf(Lf->iproto, sizeof(Lf->iproto), "%.*s", IPROTOL-1, pr);
	    Lf->inp_ty = 2;
	    if (ss & SB_INO) {
		(void) snpf(dev_ch, sizeof(dev_ch), InodeFmt_d,
		    (INODETYPE)s->st_ino);
		enter_dev_ch(dev_ch);
	    }
	    if (tp->faddr || tp->fport) {
		fs.s_addr = tp->faddr;
		fa = (unsigned char *)&fs;
	    } else
		fa = (unsigned char *)NULL;
	    if (tp->laddr || tp->lport) {
		ls.s_addr = tp->laddr;
		la = (unsigned char *)&ls;
	    } else
		la = (unsigned char *)NULL;
	    ent_inaddr(la, tp->lport, fa, tp->fport, AF_INET);
	    Lf->lts.type = tp->proto;
	    Lf->lts.state.i = tp->state;

#if     defined(HASTCPTPIQ)
	    Lf->lts.rq = tp->rxq;
	    Lf->lts.sq = tp->txq;
	    Lf->lts.rqs = Lf->lts.sqs = 1;
#endif  /* defined(HASTCPTPIQ) */

	    return;
	}
/*
 * The socket's protocol can't be identified.
 */
	(void) snpf(Lf->type, sizeof(Lf->type), "sock");
	if (ss & SB_INO) {
	    Lf->inode = (INODETYPE)s->st_ino;
	    Lf->inp_ty = 1;
	}
	if (ss & SB_DEV) {
	    Lf->dev = s->st_dev;
	    Lf->dev_def = 1;
	}
	enter_nm(Fxopt ? "can't identify protocol (-X specified)"
		       : "can't identify protocol");
}


/*
 * set_net_paths() - set /proc/net paths
 */

void
set_net_paths(p, pl)
	char *p;			/* path to /proc/net/ */
	int pl;				/* strlen(p) */
{
	int pathl;

	pathl = 0;
	(void) make_proc_path(p, pl, &AX25path, &pathl, "ax25");
	pathl = 0;
	(void) make_proc_path(p, pl, &Ipxpath, &pathl, "ipx");
	pathl = 0;
	(void) make_proc_path(p, pl, &Rawpath, &pathl, "raw");
	pathl = 0;
	(void) make_proc_path(p, pl, &TCPpath, &pathl, "tcp");
	pathl = 0;
	(void) make_proc_path(p, pl, &UDPpath, &pathl, "udp");

#if	defined(HASIPv6)
	pathl = 0;
	(void) make_proc_path(p, pl, &Raw6path, &pathl, "raw6");
	pathl = 0;
	(void) make_proc_path(p, pl, &TCP6path, &pathl, "tcp6");
	pathl = 0;
	(void) make_proc_path(p, pl, &UDP6path, &pathl, "udp6");
#endif	/* defined(HASIPv6) */

	pathl = 0;
	(void) make_proc_path(p, pl, &UNIXpath, &pathl, "unix");
}
