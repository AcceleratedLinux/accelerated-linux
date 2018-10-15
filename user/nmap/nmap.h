
/***************************************************************************
 * nmap.h -- Currently handles some of Nmap's port scanning features as    *
 * well as the command line user interface.  Note that the actual main()   *
 * function is in main.c                                                   *
 *                                                                         *
 ***********************IMPORTANT NMAP LICENSE TERMS************************
 *                                                                         *
 * The Nmap Security Scanner is (C) 1996-2006 Insecure.Com LLC. Nmap is    *
 * also a registered trademark of Insecure.Com LLC.  This program is free  *
 * software; you may redistribute and/or modify it under the terms of the  *
 * GNU General Public License as published by the Free Software            *
 * Foundation; Version 2 with the clarifications and exceptions described  *
 * below.  This guarantees your right to use, modify, and redistribute     *
 * this software under certain conditions.  If you wish to embed Nmap      *
 * technology into proprietary software, we sell alternative licenses      *
 * (contact sales@insecure.com).  Dozens of software vendors already       *
 * license Nmap technology such as host discovery, port scanning, OS       *
 * detection, and version detection.                                       *
 *                                                                         *
 * Note that the GPL places important restrictions on "derived works", yet *
 * it does not provide a detailed definition of that term.  To avoid       *
 * misunderstandings, we consider an application to constitute a           *
 * "derivative work" for the purpose of this license if it does any of the *
 * following:                                                              *
 * o Integrates source code from Nmap                                      *
 * o Reads or includes Nmap copyrighted data files, such as                *
 *   nmap-os-fingerprints or nmap-service-probes.                          *
 * o Executes Nmap and parses the results (as opposed to typical shell or  *
 *   execution-menu apps, which simply display raw Nmap output and so are  *
 *   not derivative works.)                                                * 
 * o Integrates/includes/aggregates Nmap into a proprietary executable     *
 *   installer, such as those produced by InstallShield.                   *
 * o Links to a library or executes a program that does any of the above   *
 *                                                                         *
 * The term "Nmap" should be taken to also include any portions or derived *
 * works of Nmap.  This list is not exclusive, but is just meant to        *
 * clarify our interpretation of derived works with some common examples.  *
 * These restrictions only apply when you actually redistribute Nmap.  For *
 * example, nothing stops you from writing and selling a proprietary       *
 * front-end to Nmap.  Just distribute it by itself, and point people to   *
 * http://insecure.org/nmap/ to download Nmap.                             *
 *                                                                         *
 * We don't consider these to be added restrictions on top of the GPL, but *
 * just a clarification of how we interpret "derived works" as it applies  *
 * to our GPL-licensed Nmap product.  This is similar to the way Linus     *
 * Torvalds has announced his interpretation of how "derived works"        *
 * applies to Linux kernel modules.  Our interpretation refers only to     *
 * Nmap - we don't speak for any other GPL products.                       *
 *                                                                         *
 * If you have any questions about the GPL licensing restrictions on using *
 * Nmap in non-GPL works, we would be happy to help.  As mentioned above,  *
 * we also offer alternative license to integrate Nmap into proprietary    *
 * applications and appliances.  These contracts have been sold to dozens  *
 * of software vendors, and generally include a perpetual license as well  *
 * as providing for priority support and updates as well as helping to     *
 * fund the continued development of Nmap technology.  Please email        *
 * sales@insecure.com for further information.                             *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included Copying.OpenSSL file, and distribute linked      *
 * combinations including the two. You must obey the GNU GPL in all        *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         *
 * If you received these files with a written license agreement or         *
 * contract stating terms other than the terms above, then that            *
 * alternative license agreement takes precedence over these comments.     *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to fyodor@insecure.org for possible incorporation into the main         *
 * distribution.  By sending these changes to Fyodor or one the            *
 * Insecure.Org development mailing lists, it is assumed that you are      *
 * offering Fyodor and Insecure.Com LLC the unlimited, non-exclusive right *
 * to reuse, modify, and relicense the code.  Nmap will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).  We also occasionally relicense the    *
 * code to third parties as discussed above.  If you wish to specify       *
 * special license conditions of your contributions, just say so when you  *
 * send them.                                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details at                              *
 * http://www.gnu.org/copyleft/gpl.html , or in the COPYING file included  *
 * with Nmap.                                                              *
 *                                                                         *
 ***************************************************************************/

/* $Id: nmap.h 4018 2006-09-30 21:08:04Z fyodor $ */

#ifndef NMAP_H
#define NMAP_H

/************************INCLUDES**********************************/

#ifdef WIN32
#include "mswin32\winclude.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifdef WIN32
#include "nmap_winconfig.h"
#endif /* WIN32 */
#endif /* HAVE_CONFIG_H */

#ifdef __amigaos__
#include "nmap_amigaos.h"
#endif

#include <nbase.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#else
void *malloc();
void *realloc();
#endif

#if STDC_HEADERS || HAVE_STRING_H
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
#include <memory.h>
#endif
#endif
#if HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif

#include <ctype.h>
#include <sys/types.h>

#ifndef WIN32	/* from nmapNT -- seems to work */
#include <sys/wait.h>
#endif /* !WIN32 */

#ifdef HAVE_SYS_PARAM_H   
#include <sys/param.h> /* Defines MAXHOSTNAMELEN on BSD*/
#endif

/* Linux uses these defines in netinet/ip.h and netinet/tcp.h to
   use the correct struct ip and struct tcphdr */
#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif
#ifndef __USE_BSD
#define __USE_BSD
#endif
#ifndef __BSD_SOURCE
#define __BSD_SOURCE
#endif

/* BSDI needs this to insure the correct struct ip */
#undef _IP_VHL

#include <stdio.h>

#if HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
 
#include <sys/stat.h>

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <errno.h>

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <fcntl.h>
#include <stdarg.h>

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifndef NETINET_IN_SYSTEM_H  /* why does OpenBSD not do this? */
#include <netinet/in_systm.h> /* defines n_long needed for netinet/ip.h */
#define NETINET_IN_SYSTEM_H
#endif
#ifndef NETINET_IP_H  /* why does OpenBSD not do this? */
#include <netinet/ip.h> 
#define NETINET_IP_H
#endif
// #include <netinet/ip_icmp.h> 

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <math.h>
#include <assert.h>
#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif
#ifndef NETINET_TCP_H  /* why does OpenBSD not do this? */
#include <netinet/tcp.h>          /*#include <netinet/ip_tcp.h>*/
#define NETINET_TCP_H
#endif

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/*#include <net/if_arp.h> *//* defines struct arphdr needed for if_ether.h */
// #if HAVE_NET_IF_H
// #ifndef NET_IF_H  /* why doesn't OpenBSD do this?! */
// #include <net/if.h>
// #define NET_IF_H
// #endif
// #endif
// #if HAVE_NETINET_IF_ETHER_H 
// #ifndef NETINET_IF_ETHER_H
// #include <netinet/if_ether.h>
// #define NETINET_IF_ETHER_H
// #endif /* NETINET_IF_ETHER_H */
// #endif /* HAVE_NETINET_IF_ETHER_H */

/*******  DEFINES  ************/

/* User configurable #defines: */
#ifndef VERSION
#define VERSION "1.60-Beta"
#endif
#ifndef DEBUGGING
#define DEBUGGING 0
#endif
#define MAX_PROBE_PORTS 10     /* How many TCP probe ports are allowed ? */
/* Default number of ports in parallel.  Doesn't always involve actual 
   sockets.  Can also adjust with the -M command line option.  */
#define MAX_SOCKETS 36 
/* How many hosts do we ping in parallel to see if they are up? Note that this is
   divided by the num probes per host */
#define LOOKAHEAD 30
 /*How many seconds before we give up on a host being alive? */

#define FAKE_ARGV "pine" /* What ps and w should show if you use -q */
/* How do we want to log into ftp sites for */ 
#define FTPUSER "anonymous"
#define FTPPASS "-wwwuser@"
#define FTP_RETRIES 2 /* How many times should we relogin if we lose control
                         connection? */
#define MAX_TIMEOUTS MAX_SOCKETS   /* How many timed out connection attempts 
				      in a row before we decide the host is 
				      dead? */
#define DEFAULT_TCP_PROBE_PORT 80 /* The port TCP probes go to if unspecified
				      by user -- uber hackers change this
				      to 113 */
#define DEFAULT_UDP_PROBE_PORT 31338 /* The port UDP probes (esp. "ping" probes) go to if unspecified
				     by user */

#define MAX_DECOYS 128 /* How many decoys are allowed? */

#define MAXFALLBACKS 20 /* How many comma separated fallbacks are allowed in the service-probes file? */

/* Default maximum send delay between probes to the same host */
#ifndef MAX_TCP_SCAN_DELAY
#define MAX_TCP_SCAN_DELAY 1000
#endif

#ifndef MAX_UDP_SCAN_DELAY
#define MAX_UDP_SCAN_DELAY 1000
#endif

/* Maximum number of extra hostnames, OSs, and devices, we
   consider when outputing the extra service info fields */
#define MAX_SERVICE_INFO_FIELDS 5

/* We wait at least 100 ms for a response by default - while that
   seems aggressive, waiting too long can cause us to fail to detect
   drops until many probes later on extremely low-latency
   networks (such as localhost scans).  */
#ifndef MIN_RTT_TIMEOUT
#define MIN_RTT_TIMEOUT 100 
#endif

#ifndef MAX_RTT_TIMEOUT
#define MAX_RTT_TIMEOUT 10000 /* Never allow more than 10 secs for packet round
				 trip */
#endif

#define INITIAL_RTT_TIMEOUT 1000 /* Allow 1 second initially for packet responses */

#ifndef MAX_RETRANSMISSIONS
#define MAX_RETRANSMISSIONS 10    /* 11 probes to port at maximum */
#endif

/* If nmap is called with one of the names below, it will start up in interactive mode -- alternatively, you can rename Nmap any of the following names to have it start up interactivey by default.  */
#define INTERACTIVE_NAMES { "BitchX", "Calendar", "X", "awk", "bash", "bash2", "calendar", "cat", "csh", "elm", "emacs", "ftp", "fvwm", "g++", "gcc", "gimp", "httpd", "irc", "man", "mutt", "nc", "ncftp", "netscape", "perl", "pine", "ping", "sleep", "slirp", "ssh", "sshd", "startx", "tcsh", "telnet", "telnetd", "tia", "top", "vi", "vim", "xdvi", "xemacs", "xterm", "xv" }

/* Number of hosts we pre-ping and then scan.  We do a lot more if
   randomize_hosts is set.  Every one you add to this leads to ~1K of
   extra always-resident memory in nmap */
#define PING_GROUP_SZ 2048

/* DO NOT change stuff after this point */
#define UC(b)   (((int)b)&0xff)
#define SA    struct sockaddr  /*Ubertechnique from R. Stevens */

#define HOST_UP 1
#define HOST_DOWN 2 
#define HOST_FIREWALLED 4 
#define HOST_BROADCAST 8 /* use the wierd_responses member of hoststruct instead */

#define PINGTYPE_UNKNOWN 0
#define PINGTYPE_NONE 1
#define PINGTYPE_ICMP_PING 2
#define PINGTYPE_ICMP_MASK 4
#define PINGTYPE_ICMP_TS 8
#define PINGTYPE_TCP  16
#define PINGTYPE_TCP_USE_ACK 32
#define PINGTYPE_TCP_USE_SYN 64
#define PINGTYPE_RAWTCP 128
#define PINGTYPE_CONNECTTCP 256
#define PINGTYPE_UDP  512
#define PINGTYPE_ARP 1024

#define DEFAULT_PING_TYPES PINGTYPE_TCP|PINGTYPE_TCP_USE_ACK|PINGTYPE_ICMP_PING

/* OS scan */
#define OS_SCAN_DEFAULT 9
#define OS_SCAN_SYS_1_ONLY 1
#define OS_SCAN_SYS_2_ONLY 2

/* How many syn packets do we send to TCP sequence a host? */
#define NUM_SEQ_SAMPLES 6

/* The max length of each line of the subject fingerprint when
   wrapped. */
#define FP_RESULT_WRAP_LINE_LEN 74

/* TCP/IP ISN sequence prediction classes */
#define SEQ_UNKNOWN 0
#define SEQ_64K 1
#define SEQ_TD 2
#define SEQ_RI 4
#define SEQ_TR 8
#define SEQ_i800 16
#define SEQ_CONSTANT 32

/* TCP Timestamp Sequence */
#define TS_SEQ_UNKNOWN 0
#define TS_SEQ_ZERO 1 /* At least one of the timestamps we received back was 0 */
#define TS_SEQ_2HZ 2
#define TS_SEQ_100HZ 3
#define TS_SEQ_1000HZ 4
#define TS_SEQ_OTHER_NUM 5
#define TS_SEQ_UNSUPPORTED 6 /* System didn't send back a timestamp */

#define IPID_SEQ_UNKNOWN 0
#define IPID_SEQ_INCR 1  /* simple increment by one each time */
#define IPID_SEQ_BROKEN_INCR 2 /* Stupid MS -- forgot htons() so it 
                                  counts by 256 on little-endian platforms */
#define IPID_SEQ_RPI 3 /* Goes up each time but by a "random" positive 
                          increment */
#define IPID_SEQ_RD 4 /* Appears to select IPID using a "random" distributions (meaning it can go up or down) */
#define IPID_SEQ_CONSTANT 5 /* Contains 1 or more sequential duplicates */
#define IPID_SEQ_ZERO 6 /* Every packet that comes back has an IP.ID of 0 (eg Linux 2.4 does this) */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifndef recvfrom6_t
#  define recvfrom6_t int
#endif

/********************** LOCAL INCLUDES *****************************/

#include "output.h"
#include "portlist.h"
#include "tcpip.h"
#include "global_structures.h"
#include "nmap_error.h"
#include "utils.h"
#include "services.h"
#include "protocols.h"
#include "nmap_rpc.h"
#include "targets.h"
#include "Target.h"
#include "TargetGroup.h"
#include "service_scan.h"

/***********************STRUCTURES**********************************/

/* Moved to global_structures.h */

/***********************PROTOTYPES**********************************/

/* print Interactive usage information */
void printinteractiveusage();

int ftp_anon_connect(struct ftpinfo *ftp);

/* port manipulators */
struct scan_lists *getpts(char *expr); /* someone stole the name getports()! */
void free_scan_lists(struct scan_lists *ports);

/* socket manipulation functions */
void init_socket(int sd);

/* Renamed main so that interactive mode could preprocess when neccessary */
int nmap_main(int argc, char *argv[]);

void nmap_free_mem();

/* general helper functions */
int parse_targets(struct targets *targets, char *h);
char *statenum2str(int state);
char *scantype2str(stype scantype);
void sigdie(int signo);
void reaper(int signo);
char *seqreport(struct seq_info *seq);
char *seqreport1(struct seq_info *seq);
char *seqclass2ascii(int clas);
char *ipidclass2ascii(int seqclass);
char *tsseqclass2ascii(int seqclass);

/* Convert a TCP sequence prediction difficulty index like 1264386
   into a difficulty string like "Worthy Challenge */
const char *seqidx2difficultystr(unsigned long idx);
const char *seqidx2difficultystr1(unsigned long idx);
int nmap_fetchfile(char *filename_returned, int bufferlen, char *file);
int gather_logfile_resumption_state(char *fname, int *myargc, char ***myargv);

#endif /* NMAP_H */
