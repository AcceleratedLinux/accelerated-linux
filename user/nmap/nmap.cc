
/***************************************************************************
 * nmap.cc -- Currently handles some of Nmap's port scanning features as   *
 * well as the command line user interface.  Note that the actual main()   *
 * function is in main.cc                                                  *
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

/* $Id: nmap.cc 4069 2006-10-14 06:02:43Z fyodor $ */

#include "nmap.h"
#include "osscan.h"
#include "osscan2.h"
#include "scan_engine.h"
#include "idle_scan.h"
#include "timing.h"
#include "NmapOps.h"
#include "MACLookup.h"
#include "nmap_tty.h"
#include "nmap_dns.h"
#ifdef WIN32
#include "winfix.h"
#endif

using namespace std;

/* global options */
extern char *optarg;
extern int optind;
extern NmapOps o;  /* option structure */

/* parse the --scanflags argument.  It can be a number >=0 or a string consisting of TCP flag names like "URGPSHFIN".  Returns -1 if the argument is invalid. */
static int parse_scanflags(char *arg) {
  int flagval = 0;
  char *end = NULL;

  if (isdigit(arg[0])) {
    flagval = strtol(arg, &end, 0);
    if (*end || flagval < 0 || flagval > 255) return -1;
  } else {
    if (strcasestr(arg, "FIN")) {
      flagval |= TH_FIN;
    } 
    if (strcasestr(arg, "SYN")) {
      flagval |= TH_SYN;
    } 
    if (strcasestr(arg, "RST") || strcasestr(arg, "RESET")) {
      flagval |= TH_RST;
    } 
    if (strcasestr(arg, "PSH") || strcasestr(arg, "PUSH")) {
      flagval |= TH_PUSH;
    } 
    if (strcasestr(arg, "ACK")) {
      flagval |= TH_ACK;
    } 
    if (strcasestr(arg, "URG")) {
      flagval |= TH_URG;
    } 
  }
  return flagval;
}

/* parse a URL stype ftp string of the form user:pass@server:portno */
static int parse_bounce_argument(struct ftpinfo *ftp, char *url) {
  char *p = url,*q, *s;

  if ((q = strrchr(url, '@'))) { /* we have user and/or pass */
    *q++ = '\0';

    if ((s = strchr(p, ':'))) { /* we have user AND pass */
      *s++ = '\0';
      strncpy(ftp->pass, s, 255);
    } else { /* we ONLY have user */
      log_write(LOG_STDOUT, "Assuming %s is a username, and using the default password: %s\n",
		p, ftp->pass);
    }

    strncpy(ftp->user, p, 63);
  } else {
    q = url;
  }

  /* q points to beginning of server name */
  if ((s = strchr(q, ':'))) { /* we have portno */
    *s++ = '\0';
    ftp->port = atoi(s);
  }

  strncpy(ftp->server_name, q, MAXHOSTNAMELEN);

  ftp->user[63] = ftp->pass[255] = ftp->server_name[MAXHOSTNAMELEN] = 0;

  return 1;
}

static void printusage(char *name, int rc) {

printf("%s %s ( %s )\n"
       "Usage: nmap [Scan Type(s)] [Options] {target specification}\n"
       "TARGET SPECIFICATION:\n"
       "  Can pass hostnames, IP addresses, networks, etc.\n"
       "  Ex: scanme.nmap.org, microsoft.com/24, 192.168.0.1; 10.0.0-255.1-254\n"
       "  -iL <inputfilename>: Input from list of hosts/networks\n"
       "  -iR <num hosts>: Choose random targets\n"
       "  --exclude <host1[,host2][,host3],...>: Exclude hosts/networks\n"
       "  --excludefile <exclude_file>: Exclude list from file\n"
       "HOST DISCOVERY:\n"
       "  -sL: List Scan - simply list targets to scan\n"
       "  -sP: Ping Scan - go no further than determining if host is online\n"
       "  -P0: Treat all hosts as online -- skip host discovery\n"
       "  -PS/PA/PU [portlist]: TCP SYN/ACK or UDP discovery to given ports\n"
       "  -PE/PP/PM: ICMP echo, timestamp, and netmask request discovery probes\n"
       "  -n/-R: Never do DNS resolution/Always resolve [default: sometimes]\n"
       "  --dns-servers <serv1[,serv2],...>: Specify custom DNS servers\n"
       "  --system-dns: Use OS's DNS resolver\n"
       "SCAN TECHNIQUES:\n"
       "  -sS/sT/sA/sW/sM: TCP SYN/Connect()/ACK/Window/Maimon scans\n"
       "  -sU: UDP Scan\n"
       "  -sN/sF/sX: TCP Null, FIN, and Xmas scans\n"
       "  --scanflags <flags>: Customize TCP scan flags\n"
       "  -sI <zombie host[:probeport]>: Idlescan\n"
       "  -sO: IP protocol scan\n"
       "  -b <ftp relay host>: FTP bounce scan\n"
       "PORT SPECIFICATION AND SCAN ORDER:\n"
       "  -p <port ranges>: Only scan specified ports\n"
       "    Ex: -p22; -p1-65535; -p U:53,111,137,T:21-25,80,139,8080\n"
       "  -F: Fast - Scan only the ports listed in the nmap-services file)\n"
       "  -r: Scan ports consecutively - don't randomize\n"
       "SERVICE/VERSION DETECTION:\n"
       "  -sV: Probe open ports to determine service/version info\n"
       "  --version-intensity <level>: Set from 0 (light) to 9 (try all probes)\n"
       "  --version-light: Limit to most likely probes (intensity 2)\n"
       "  --version-all: Try every single probe (intensity 9)\n"
       "  --version-trace: Show detailed version scan activity (for debugging)\n"
       "OS DETECTION:\n"
       "  -O: Enable OS detection (try 2nd generation w/fallback to 1st)\n"
       "  -O2: Only use the new OS detection system (no fallback)\n"
       "  -O1: Only use the old (1st generation) OS detection system\n"
       "  --osscan-limit: Limit OS detection to promising targets\n"
       "  --osscan-guess: Guess OS more aggressively\n"
       "TIMING AND PERFORMANCE:\n"
       "  Options which take <time> are in milliseconds, unless you append 's'\n"
       "  (seconds), 'm' (minutes), or 'h' (hours) to the value (e.g. 30m).\n"
       "  -T[0-5]: Set timing template (higher is faster)\n"
       "  --min-hostgroup/max-hostgroup <size>: Parallel host scan group sizes\n"
       "  --min-parallelism/max-parallelism <time>: Probe parallelization\n"
       "  --min-rtt-timeout/max-rtt-timeout/initial-rtt-timeout <time>: Specifies\n"
       "      probe round trip time.\n"
       "  --max-retries <tries>: Caps number of port scan probe retransmissions.\n"
       "  --host-timeout <time>: Give up on target after this long\n"
       "  --scan-delay/--max-scan-delay <time>: Adjust delay between probes\n"
       "FIREWALL/IDS EVASION AND SPOOFING:\n"
       "  -f; --mtu <val>: fragment packets (optionally w/given MTU)\n"
       "  -D <decoy1,decoy2[,ME],...>: Cloak a scan with decoys\n"
       "  -S <IP_Address>: Spoof source address\n"
       "  -e <iface>: Use specified interface\n"
       "  -g/--source-port <portnum>: Use given port number\n"
       "  --data-length <num>: Append random data to sent packets\n"
       "  --ip-options <options>: Send packets with specified ip options\n"
       "  --ttl <val>: Set IP time-to-live field\n"
       "  --spoof-mac <mac address/prefix/vendor name>: Spoof your MAC address\n"
       "  --badsum: Send packets with a bogus TCP/UDP checksum\n"
       "OUTPUT:\n"
       "  -oN/-oX/-oS/-oG <file>: Output scan in normal, XML, s|<rIpt kIddi3,\n"
       "     and Grepable format, respectively, to the given filename.\n"
       "  -oA <basename>: Output in the three major formats at once\n"
       "  -v: Increase verbosity level (use twice for more effect)\n"
       "  -d[level]: Set or increase debugging level (Up to 9 is meaningful)\n"
       "  --open: Only show open (or possibly open) ports\n"
       "  --packet-trace: Show all packets sent and received\n"
       "  --iflist: Print host interfaces and routes (for debugging)\n"
       "  --log-errors: Log errors/warnings to the normal-format output file\n"
       "  --append-output: Append to rather than clobber specified output files\n"
       "  --resume <filename>: Resume an aborted scan\n"
       "  --stylesheet <path/URL>: XSL stylesheet to transform XML output to HTML\n"
       "  --webxml: Reference stylesheet from Insecure.Org for more portable XML\n"
       "  --no-stylesheet: Prevent associating of XSL stylesheet w/XML output\n"
       "MISC:\n"
       "  -6: Enable IPv6 scanning\n"
       "  -A: Enables OS detection and Version detection\n"
       "  --datadir <dirname>: Specify custom Nmap data file location\n"
       "  --send-eth/--send-ip: Send using raw ethernet frames or IP packets\n"
       "  --privileged: Assume that the user is fully privileged\n"
       "  --unprivileged: Assume the user lacks raw socket privileges\n"
       "  -V: Print version number\n"
       "  -h: Print this help summary page.\n"
       "EXAMPLES:\n"
       "  nmap -v -A scanme.nmap.org\n"
       "  nmap -v -sP 192.168.0.0/16 10.0.0.0/8\n"
       "  nmap -v -iR 10000 -P0 -p 80\n"
       "SEE THE MAN PAGE FOR MANY MORE OPTIONS, DESCRIPTIONS, AND EXAMPLES\n", NMAP_NAME, NMAP_VERSION, NMAP_URL);
  exit(rc);
}

/**
 * Returns 1 if this is a reserved IP address, where "reserved" means
 * either a private address, non-routable address, or even a non-reserved
 * but unassigned address which has an extremely high probability of being
 * black-holed.
 *
 * We try to optimize speed when ordering the tests. This optimization
 * assumes that all byte values are equally likely in the input.
 *
 * Warning: This function could easily become outdated if the IANA
 * starts to assign some more IPv4 ranges to RIPE, etc. as they have
 * started doing this year (2001), for example 80.0.0.0/4 used to be
 * completely unassigned until they gave 80.0.0.0/7 to RIPE in April
 * 2001 (www.junk.org is an example of a new address in this range).
 *
 * Check <http://www.iana.org/assignments/ipv4-address-space> for
 * the most recent assigments and
 * <http://www.cymru.com/Documents/bogon-bn-nonagg.txt> for bogon
 * netblocks.
 */
static int ip_is_reserved(struct in_addr *ip)
{
  char *ipc = (char *) &(ip->s_addr);
  unsigned char i1 = ipc[0], i2 = ipc[1], i3 = ipc[2], i4 = ipc[3];

  /* do all the /7's and /8's with a big switch statement, hopefully the
   * compiler will be able to optimize this a little better using a jump table
   * or what have you
   */
  switch (i1)
    {
    case 0:         /* 000/8 is IANA reserved       */
    case 1:         /* 001/8 is IANA reserved       */
    case 2:         /* 002/8 is IANA reserved       */
    case 5:         /* 005/8 is IANA reserved       */
    case 6:         /* USA Army ISC                 */
    case 7:         /* used for BGP protocol        */
    case 10:        /* the infamous 10.0.0.0/8      */
    case 23:        /* 023/8 is IANA reserved       */
    case 27:        /* 027/8 is IANA reserved       */
    case 31:        /* 031/8 is IANA reserved       */
    case 36:        /* 036/8 is IANA reserved       */
    case 37:        /* 037/8 is IANA reserved       */
    case 39:        /* 039/8 is IANA reserved       */
    case 42:        /* 042/8 is IANA reserved       */
    case 49:        /* 049/8 is IANA reserved       */
    case 50:        /* 050/8 is IANA reserved       */
    case 55:        /* misc. U.S.A. Armed forces    */
    case 127:       /* 127/8 is reserved for loopback */
    case 197:       /* 197/8 is IANA reserved       */
    case 223:       /* 223/8 is IANA reserved       */
      return 1;
    default:
      break;
    }


  /* 077-079/8 is IANA reserved */
  if (i1 >= 77 && i1 <= 79)
    return 1;

  /* 092-123/8 is IANA reserved */
  if (i1 >= 92 && i1 <= 123)
    return 1;

  /* 172.16.0.0/12 is reserved for private nets by RFC1819 */
  if (i1 == 172 && i2 >= 16 && i2 <= 31)
    return 1;

  /* 173-187/8 is IANA reserved */
  if (i1 >= 173 && i1 <= 187)
    return 1;

  /* 192.168.0.0/16 is reserved for private nets by RFC1819 */
  /* 192.0.2.0/24 is reserved for documentation and examples */
  /* 192.88.99.0/24 is used as 6to4 Relay anycast prefix by RFC3068 */
  if (i1 == 192) {
    if (i2 == 168)
      return 1;
    if (i2 == 0 && i3 == 2)
      return 1;
    if (i2 == 88 && i3 == 99)
      return 1;
  }

  /* 198.18.0.0/15 is used for benchmark tests by RFC2544 */
  if (i1 == 198 && i2 == 18 && i3 >= 1 && i3 <= 64) {
    return 1;
  }

  /* reserved for DHCP clients seeking addresses, not routable outside LAN */
  if (i1 == 169 && i2 == 254)
    return 1;

  /* believe it or not, 204.152.64.0/23 is some bizarre Sun proprietary
   * clustering thing */
  if (i1 == 204 && i2 == 152 && (i3 == 64 || i3 == 65))
    return 1;

  /* 224-239/8 is all multicast stuff */
  /* 240-255/8 is IANA reserved */
  if (i1 >= 224)
    return 1;

  /* 255.255.255.255, note we already tested for i1 in this range */
  if (i2 == 255 && i3 == 255 && i4 == 255)
    return 1;

  return 0;
}

static char *grab_next_host_spec(FILE *inputfd, int argc, char **fakeargv) {
  static char host_spec[1024];
  unsigned int host_spec_index;
  int ch;
  struct in_addr ip;

  if (o.generate_random_ips) {
    do {
      ip.s_addr = get_random_u32();
    } while (ip_is_reserved(&ip));
    Strncpy(host_spec, inet_ntoa(ip), sizeof(host_spec));
  } else if (!inputfd) {
    return( (optind < argc)?  fakeargv[optind++] : NULL);
  } else { 
    host_spec_index = 0;
    while((ch = getc(inputfd)) != EOF) {
      if (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t' || ch == '\0') {
	if (host_spec_index == 0) continue;
	host_spec[host_spec_index] = '\0';
	return host_spec;
      } else if (host_spec_index < sizeof(host_spec) / sizeof(char) -1) {
	host_spec[host_spec_index++] = (char) ch;
      } else fatal("One of the host_specifications from your input file is too long (> %d chars)", (int) sizeof(host_spec));
    }
    host_spec[host_spec_index] = '\0';
  }
  if (!*host_spec) return NULL;
  return host_spec;
}

int nmap_main(int argc, char *argv[]) {
  char *p, *q;
  int i, arg;
  long l;
  unsigned int targetno;
  FILE *inputfd = NULL, *excludefd = NULL;
  char *host_spec = NULL, *exclude_spec = NULL;
  short fastscan=0, randomize=1;
  short quashargv = 0;
  char **host_exp_group;
  char *idleProxy = NULL; /* The idle host used to "Proxy" an Idlescan */
  int num_host_exp_groups = 0;
  char *machinefilename = NULL, *kiddiefilename = NULL, 
    *normalfilename = NULL, *xmlfilename = NULL;
  HostGroupState *hstate = NULL;
  char *endptr = NULL;
  struct scan_lists *ports = NULL;
  TargetGroup *exclude_group = NULL;
  char myname[MAXHOSTNAMELEN + 1];
#if (defined(IN_ADDR_DEEPSTRUCT) || defined( SOLARIS))
  /* Note that struct in_addr in solaris is 3 levels deep just to store an
   * unsigned int! */
  struct ftpinfo ftp = { FTPUSER, FTPPASS, "",  { { { 0 } } } , 21, 0};
#else
  struct ftpinfo ftp = { FTPUSER, FTPPASS, "", { 0 }, 21, 0};
#endif
  struct hostent *target = NULL;
  char **fakeargv;
  Target *currenths;
  vector<Target *> Targets;
  char *portlist = NULL; /* Ports list specified by user */
  char *proberr;
  int sourceaddrwarning = 0; /* Have we warned them yet about unguessable
				source addresses? */
  unsigned int ideal_scan_group_sz = 0;
  char hostname[MAXHOSTNAMELEN + 1] = "";
  const char *spoofmac = NULL;
  time_t timep;
  char mytime[128];
  struct sockaddr_storage ss;
  size_t sslen;
  int option_index;
  bool iflist = false;

  // Pre-specified timing parameters.
  // These are stored here during the parsing of the arguments so that we can
  // set the defaults specified by any timing template options (-T2, etc) BEFORE
  // any of these. In other words, these always take precedence over the templates.
  int pre_max_parallelism=-1, pre_scan_delay=-1, pre_max_scan_delay=-1;
  int pre_init_rtt_timeout=-1, pre_min_rtt_timeout=-1, pre_max_rtt_timeout=-1;
  int pre_max_retries=-1;
  long pre_host_timeout=-1;

  struct option long_options[] =
    {
      {"version", no_argument, 0, 'V'},
      {"verbose", no_argument, 0, 'v'},
      {"datadir", required_argument, 0, 0},
      {"debug", optional_argument, 0, 'd'},
      {"help", no_argument, 0, 'h'},
      {"iflist", no_argument, 0, 0},
      {"release_memory", no_argument, 0, 0},
      {"release-memory", no_argument, 0, 0},
      {"max_os_tries", required_argument, 0, 0},
      {"max-os-tries", required_argument, 0, 0},
      {"max_parallelism", required_argument, 0, 'M'},
      {"max-parallelism", required_argument, 0, 'M'},
      {"min_parallelism", required_argument, 0, 0},
      {"min-parallelism", required_argument, 0, 0},
      {"timing", required_argument, 0, 'T'},
      {"max_rtt_timeout", required_argument, 0, 0},
      {"max-rtt-timeout", required_argument, 0, 0},
      {"min_rtt_timeout", required_argument, 0, 0},
      {"min-rtt-timeout", required_argument, 0, 0},
      {"initial_rtt_timeout", required_argument, 0, 0},
      {"initial-rtt-timeout", required_argument, 0, 0},
      {"excludefile", required_argument, 0, 0},
      {"exclude", required_argument, 0, 0},
      {"max_hostgroup", required_argument, 0, 0},
      {"max-hostgroup", required_argument, 0, 0},
      {"min_hostgroup", required_argument, 0, 0},
      {"min-hostgroup", required_argument, 0, 0},
      {"open", no_argument, 0, 0},
      {"scanflags", required_argument, 0, 0},
      {"defeat_rst_ratelimit", no_argument, 0, 0},
      {"defeat-rst-ratelimit", no_argument, 0, 0},
      {"host_timeout", required_argument, 0, 0},
      {"host-timeout", required_argument, 0, 0},
      {"scan_delay", required_argument, 0, 0},
      {"scan-delay", required_argument, 0, 0},
      {"max_scan_delay", required_argument, 0, 0},
      {"max-scan-delay", required_argument, 0, 0},
      {"max_retries", required_argument, 0, 0},
      {"max-retries", required_argument, 0, 0},
      {"oA", required_argument, 0, 0},  
      {"oN", required_argument, 0, 0},
      {"oM", required_argument, 0, 0},  
      {"oG", required_argument, 0, 0},  
      {"oS", required_argument, 0, 0},
      {"oH", required_argument, 0, 0},  
      {"oX", required_argument, 0, 0},  
      {"iL", required_argument, 0, 'i'},  
      {"iR", required_argument, 0, 0},
      {"sI", required_argument, 0, 0},  
      {"source_port", required_argument, 0, 'g'},
      {"source-port", required_argument, 0, 'g'},
      {"randomize_hosts", no_argument, 0, 0},
      {"randomize-hosts", no_argument, 0, 0},
      {"osscan_limit", no_argument, 0, 0}, /* skip OSScan if no open ports */
      {"osscan-limit", no_argument, 0, 0}, /* skip OSScan if no open ports */
      {"osscan_guess", no_argument, 0, 0}, /* More guessing flexability */
      {"osscan-guess", no_argument, 0, 0}, /* More guessing flexability */
      {"fuzzy", no_argument, 0, 0}, /* Alias for osscan_guess */
      {"packet_trace", no_argument, 0, 0}, /* Display all packets sent/rcv */
      {"packet-trace", no_argument, 0, 0}, /* Display all packets sent/rcv */
      {"version_trace", no_argument, 0, 0}, /* Display -sV related activity */
      {"version-trace", no_argument, 0, 0}, /* Display -sV related activity */
      {"data_length", required_argument, 0, 0},
      {"data-length", required_argument, 0, 0},
      {"send_eth", no_argument, 0, 0},
      {"send-eth", no_argument, 0, 0},
      {"send_ip", no_argument, 0, 0},
      {"send-ip", no_argument, 0, 0},
      {"stylesheet", required_argument, 0, 0},
      {"no_stylesheet", no_argument, 0, 0},
      {"no-stylesheet", no_argument, 0, 0},
      {"webxml", no_argument, 0, 0},
      {"rH", no_argument, 0, 0},
      {"vv", no_argument, 0, 0},
      {"ff", no_argument, 0, 0},
      {"privileged", no_argument, 0, 0},
      {"unprivileged", no_argument, 0, 0},
      {"mtu", required_argument, 0, 0},
      {"append_output", no_argument, 0, 0},
      {"append-output", no_argument, 0, 0},
      {"noninteractive", no_argument, 0, 0},
      {"spoof_mac", required_argument, 0, 0},
      {"spoof-mac", required_argument, 0, 0},
      {"thc", no_argument, 0, 0},  
      {"badsum", no_argument, 0, 0},  
      {"ttl", required_argument, 0, 0}, /* Time to live */
      {"allports", no_argument, 0, 0},
      {"version_intensity", required_argument, 0, 0},
      {"version-intensity", required_argument, 0, 0},
      {"version_light", no_argument, 0, 0},
      {"version-light", no_argument, 0, 0},
      {"version_all", no_argument, 0, 0},
      {"version-all", no_argument, 0, 0},
      {"system_dns", no_argument, 0, 0},
      {"system-dns", no_argument, 0, 0},
      {"log_errors", no_argument, 0, 0},
      {"log-errors", no_argument, 0, 0},
      {"dns_servers", required_argument, 0, 0},
      {"dns-servers", required_argument, 0, 0},
      {"ip_options", required_argument, 0, 0},
      {"ip-options", required_argument, 0, 0},
      {0, 0, 0, 0}
    };

  /* argv faking silliness */
  fakeargv = (char **) safe_malloc(sizeof(char *) * (argc + 1));
  for(i=0; i < argc; i++) {
    fakeargv[i] = strdup(argv[i]);
  }
  fakeargv[argc] = NULL;

  if (argc < 2 ) printusage(argv[0], -1);
  Targets.reserve(100);
#ifdef WIN32
  win_pre_init();
#endif

  /* OK, lets parse these args! */
  optind = 1; /* so it can be called multiple times */
  while((arg = getopt_long_only(argc,fakeargv,"6Ab:D:d::e:Ffg:hIi:M:m:nO::o:P:p:qRrS:s:T:Vv", long_options, &option_index)) != EOF) {
    switch(arg) {
    case 0:
      if (optcmp(long_options[option_index].name, "max-os-tries") == 0) {
	l = tval2msecs(optarg);
	if (l < 1 || l > 50) 
	  fatal("Bogus --max-os-tries argument specified, must be between 1 and 50 (inclusive)");
	o.setMaxOSTries(l);
      } else if (optcmp(long_options[option_index].name, "max-rtt-timeout") == 0) {
	l = tval2msecs(optarg);
	if (l < 5) fatal("Bogus --max-rtt-timeout argument specified, must be at least 5");
        if (l < 20) {
	  error("WARNING: You specified a round-trip time timeout (%ld ms) that is EXTRAORDINARILY SMALL.  Accuracy may suffer.", l);
	}
        pre_max_rtt_timeout = l;
      } else if (optcmp(long_options[option_index].name, "min-rtt-timeout") == 0) {
	l = tval2msecs(optarg);
	if (l < 0) fatal("Bogus --min-rtt-timeout argument specified");
	if (l > 50000) {
	  error("Warning:  min-rtt-timeout is given in milliseconds, your value seems pretty large.");
	}
        pre_min_rtt_timeout = l;
      } else if (optcmp(long_options[option_index].name, "initial-rtt-timeout") == 0) {
	l = tval2msecs(optarg);
	if (l <= 0) fatal("Bogus --initial-rtt-timeout argument specified.  Must be positive");
        pre_init_rtt_timeout = l;
      } else if (strcmp(long_options[option_index].name, "excludefile") == 0) {
	if (exclude_spec)
	  fatal("--excludefile and --exclude options are mutually exclusive.");
        excludefd = fopen(optarg, "r");
        if (!excludefd) {
          fatal("Failed to open exclude file %s for reading", optarg);
        }
      } else if (strcmp(long_options[option_index].name, "exclude") == 0) {
	if (excludefd)
	  fatal("--excludefile and --exclude options are mutually exclusive.");
	exclude_spec = strdup(optarg);
      } else if (optcmp(long_options[option_index].name, "max-hostgroup") == 0) {
	o.setMaxHostGroupSz(atoi(optarg));
      } else if (optcmp(long_options[option_index].name, "min-hostgroup") == 0) {
	o.setMinHostGroupSz(atoi(optarg));
	if (atoi(optarg) > 100)
	  error("Warning: You specified a highly aggressive --min-hostgroup.");
      } else if (optcmp(long_options[option_index].name, "open") == 0) {
	o.setOpenOnly(true);
      } else if (strcmp(long_options[option_index].name, "scanflags") == 0) {
	o.scanflags = parse_scanflags(optarg);
	if (o.scanflags < 0) {
	  fatal("--scanflags option must be a number between 0 and 255 (inclusive) or a string like \"URGPSHFIN\".");
	}
      } else if (strcmp(long_options[option_index].name, "iflist") == 0 ) {
	iflist = true;
      } else if (strcmp(long_options[option_index].name, "release-memory") == 0 ) {
	o.release_memory = true;
      } else if (optcmp(long_options[option_index].name, "min-parallelism") == 0 ) {
	o.min_parallelism = atoi(optarg); 
	if (o.min_parallelism < 1) fatal("Argument to --min-parallelism must be at least 1!");
	if (o.min_parallelism > 100) {
	  error("Warning: Your --min-parallelism option is pretty high!  This can hurt reliability.");
	}
      } else if (optcmp(long_options[option_index].name, "host-timeout") == 0) {	l = tval2msecs(optarg);
	if (l <= 1500) fatal("--host-timeout is specified in milliseconds unless you qualify it by appending 's', 'm', 'h', or 'd'.  The value must be greater than 1500 milliseconds");
	pre_host_timeout = l;
	if (l < 15000) {
	  error("host-timeout is given in milliseconds, so you specified less than 15 seconds (%lims). This is allowed but not recommended.", o.host_timeout);
	}
      } else if (strcmp(long_options[option_index].name, "ttl") == 0) {
	o.ttl = atoi(optarg);
	if (o.ttl < 0 || o.ttl > 255) {
	  fatal("ttl option must be a number between 0 and 255 (inclusive)");
	}
      } else if (strcmp(long_options[option_index].name, "datadir") == 0) {
	o.datadir = strdup(optarg);
      } else if (optcmp(long_options[option_index].name, "append-output") == 0) {
	o.append_output = 1;
      } else if (strcmp(long_options[option_index].name, "noninteractive") == 0) {
	o.noninteractive = true;
      } else if (optcmp(long_options[option_index].name, "spoof-mac") == 0) {
	/* I need to deal with this later, once I'm sure that I have output
	   files set up, --datadir, etc. */
	spoofmac = optarg;
      } else if (strcmp(long_options[option_index].name, "allports") == 0) {
        o.override_excludeports = 1;
      } else if (optcmp(long_options[option_index].name, "version-intensity") == 0) {
	o.version_intensity = atoi(optarg);
	if (o.version_intensity < 0 || o.version_intensity > 9)
		fatal("version-intensity must be between 0 and 9");
      } else if (optcmp(long_options[option_index].name, "version-light") == 0) {
        o.version_intensity = 2;
      } else if (optcmp(long_options[option_index].name, "version-all") == 0) {
        o.version_intensity = 9;
      } else if (optcmp(long_options[option_index].name, "scan-delay") == 0) {
	l = tval2msecs(optarg);
	if (l < 0) fatal("Bogus --scan-delay argument specified.");
	pre_scan_delay = l;
      } else if (optcmp(long_options[option_index].name, "defeat-rst-ratelimit") == 0) {
        o.defeat_rst_ratelimit = 1;
      } else if (optcmp(long_options[option_index].name, "max-scan-delay") == 0) {
	l = tval2msecs(optarg);
	if (l < 0) fatal("--max-scan-delay cannot be negative.");
	pre_max_scan_delay = l;
      } else if (optcmp(long_options[option_index].name, "max-retries") == 0) {
        pre_max_retries = atoi(optarg);
        if (pre_max_retries < 0)
          fatal("max-retries must be positive");
      } else if (optcmp(long_options[option_index].name, "randomize-hosts") == 0
		 || strcmp(long_options[option_index].name, "rH") == 0) {
	o.randomize_hosts = 1;
	o.ping_group_sz = PING_GROUP_SZ * 4;
      } else if (optcmp(long_options[option_index].name, "osscan-limit")  == 0) {
	o.osscan_limit = 1;
      } else if (optcmp(long_options[option_index].name, "osscan-guess")  == 0
                 || strcmp(long_options[option_index].name, "fuzzy") == 0) {
	o.osscan_guess = 1;
      } else if (optcmp(long_options[option_index].name, "packet-trace") == 0) {
	o.setPacketTrace(true);
      } else if (optcmp(long_options[option_index].name, "version-trace") == 0) {
	o.setVersionTrace(true);
	o.debugging++;
      } else if (optcmp(long_options[option_index].name, "data-length") == 0) {
	o.extra_payload_length = atoi(optarg);
	if (o.extra_payload_length < 0) {
	  fatal("data-length must be greater than 0");
	} else if (o.extra_payload_length > 0) {
	  o.extra_payload = (char *) safe_malloc(o.extra_payload_length);
	  get_random_bytes(o.extra_payload, o.extra_payload_length);
	}
      } else if (optcmp(long_options[option_index].name, "send-eth") == 0) {
	o.sendpref = PACKET_SEND_ETH_STRONG;
      } else if (optcmp(long_options[option_index].name, "send-ip") == 0) {
	o.sendpref = PACKET_SEND_IP_STRONG;
      } else if (strcmp(long_options[option_index].name, "stylesheet") == 0) {
	o.setXSLStyleSheet(optarg);
      } else if (optcmp(long_options[option_index].name, "no-stylesheet") == 0) {
	o.setXSLStyleSheet(NULL);
      } else if (optcmp(long_options[option_index].name, "system-dns") == 0) {
        o.mass_dns = false;
      } else if (optcmp(long_options[option_index].name, "dns-servers") == 0) {
        o.dns_servers = strdup(optarg);
      } else if (optcmp(long_options[option_index].name, "log-errors") == 0) {
        o.log_errors = 1;
      } else if (strcmp(long_options[option_index].name, "webxml") == 0) {
	o.setXSLStyleSheet("http://www.insecure.org/nmap/data/nmap.xsl");
      } else if (strcmp(long_options[option_index].name, "oN") == 0) {
	normalfilename = optarg;
      } else if (strcmp(long_options[option_index].name, "oG") == 0 ||
		 strcmp(long_options[option_index].name, "oM") == 0) {
	machinefilename = optarg;
      } else if (strcmp(long_options[option_index].name, "oS") == 0) {
	kiddiefilename = optarg;
      } else if (strcmp(long_options[option_index].name, "oH") == 0) {
	fatal("HTML output is not directly supported, though Nmap includes an XSL for transforming XML output into HTML.  See the man page.");
      } else if (strcmp(long_options[option_index].name, "oX") == 0) {
	xmlfilename = optarg;
      } else if (strcmp(long_options[option_index].name, "oA") == 0) {
	char buf[MAXPATHLEN];
	snprintf(buf, sizeof(buf), "%s.nmap", optarg);
	normalfilename = strdup(buf);
	snprintf(buf, sizeof(buf), "%s.gnmap", optarg);
	machinefilename = strdup(buf);
	snprintf(buf, sizeof(buf), "%s.xml", optarg);
	xmlfilename = strdup(buf);
      } else if (strcmp(long_options[option_index].name, "thc") == 0) {
	printf("!!Greets to Van Hauser, Plasmoid, Skyper and the rest of THC!!\n");
	exit(0);
      } else if (strcmp(long_options[option_index].name, "badsum") == 0) {
	o.badsum = 1;
      } else if (strcmp(long_options[option_index].name, "iR") == 0) {
	o.generate_random_ips = 1;
	o.max_ips_to_scan = strtoul(optarg, &endptr, 10);
	if (*endptr != '\0') {
	  fatal("ERROR: -iR argument must be the maximum number of random IPs you wish to scan (use 0 for unlimited)");
	}
      } else if (strcmp(long_options[option_index].name, "sI") == 0) {
	o.idlescan = 1;
	idleProxy = optarg;
      } else if (strcmp(long_options[option_index].name, "vv") == 0) {
	/* Compatability hack ... ugly */
	o.verbose += 2;
      } else if (strcmp(long_options[option_index].name, "ff") == 0) {
	o.fragscan += 16; 
      } else if (strcmp(long_options[option_index].name, "privileged") == 0) {
	o.isr00t = 1;
      } else if (strcmp(long_options[option_index].name, "unprivileged") == 0) {
	o.isr00t = 0;
      } else if (strcmp(long_options[option_index].name, "mtu") == 0) {
        o.fragscan = atoi(optarg);
        if (o.fragscan <= 0 || o.fragscan % 8 != 0)
            fatal("Data payload MTU must be >0 and multiple of 8");
      } else if (strcmp(long_options[option_index].name, "ip-options") == 0){
        o.ipoptions    = (u8*) safe_malloc(4*10+1);
        o.ipoptionslen = parse_ip_options(optarg, o.ipoptions, 4*10+1, &o.ipopt_firsthop, &o.ipopt_lasthop);
        if(o.ipoptionslen > 4*10)
          fatal("Ip options can't be more than 40 bytes long");
        if(o.ipoptionslen %4 != 0)
          fatal("Ip options must be multiple of 4 (read length is %i bytes)", o.ipoptionslen);
      } else {
	fatal("Unknown long option (%s) given@#!$#$", long_options[option_index].name);
      }
      break;
    case '6':
#if !HAVE_IPV6
      fatal("I am afraid IPv6 is not available because your host doesn't support it or you chose to compile Nmap w/o IPv6 support.");
#else
      o.setaf(AF_INET6);
#endif /* !HAVE_IPV6 */
      break;
    case 'A':
      o.servicescan = true;
      if (o.isr00t)
		o.osscan = OS_SCAN_DEFAULT;
      break;
    case 'b': 
      o.bouncescan++;
      if (parse_bounce_argument(&ftp, optarg) < 0 ) {
	fprintf(stderr, "Your argument to -b is b0rked. Use the normal url style:  user:pass@server:port or just use server and use default anon login\n  Use -h for help\n");
      }
      break;
    case 'D':
      p = optarg;
      do {    
	q = strchr(p, ',');
	if (q) *q = '\0';
	if (!strcasecmp(p, "me")) {
	  if (o.decoyturn != -1) 
	    fatal("Can only use 'ME' as a decoy once.\n");
	  o.decoyturn = o.numdecoys++;
	} else {      
	  if (o.numdecoys >= MAX_DECOYS -1)
	    fatal("You are only allowed %d decoys (if you need more redefine MAX_DECOYS in nmap.h)", MAX_DECOYS);
	  if (resolve(p, &o.decoys[o.numdecoys])) {
	    o.numdecoys++;
	  } else {
	    fatal("Failed to resolve decoy host: %s (must be hostname or IP address)", p);
	  }
	}
	if (q) {
	  *q = ',';
	  p = q+1;
	}
      } while(q);
      break;
    case 'd': 
      if (optarg)
	o.debugging = o.verbose = atoi(optarg);
      else {
	o.debugging++; o.verbose++;
      }
      break;
    case 'e': 
      Strncpy(o.device, optarg, sizeof(o.device)); break;
    case 'F': fastscan++; break;
    case 'f': o.fragscan += 8; break;
    case 'g': 
      o.magic_port = atoi(optarg);
      o.magic_port_set = 1;
      if (o.magic_port == 0) error("WARNING: a source port of zero may not work on all systems.");
      break;    
    case 'h': printusage(argv[0], 0); break;
    case '?': printusage(argv[0], -1); break;
    case 'I': 
      printf("WARNING: identscan (-I) no longer supported.  Ignoring -I\n");
      break;
      // o.identscan++; break;
    case 'i': 
      if (inputfd) {
	fatal("Only one input filename allowed");
      }
      if (!strcmp(optarg, "-")) {
	inputfd = stdin;
      } else {    
	inputfd = fopen(optarg, "r");
	if (!inputfd) {
	  fatal("Failed to open input file %s for reading", optarg);
	}  
      }
      break;  
    case 'M': 
      pre_max_parallelism = atoi(optarg); 
      if (pre_max_parallelism < 1) fatal("Argument to -M must be at least 1!");
      if (pre_max_parallelism > 900) {
	error("Warning: Your max-parallelism (-M) option is extraordinarily high, which can hurt reliability");
      }
      break;
    case 'm': 
      machinefilename = optarg;
      break;
    case 'n': o.noresolve++; break;
    case 'O': 
      if (!optarg)
		o.osscan = OS_SCAN_DEFAULT;
      else if (*optarg == '1')
		o.osscan = OS_SCAN_SYS_1_ONLY;
	  else if (*optarg == '2')
		o.osscan = OS_SCAN_SYS_2_ONLY;
      else {
	fatal("Use -O for new osscan engine, -O1 for old osscan engine.");
      }
      break;
    case 'o':
      normalfilename = optarg;
      break;
    case 'P': 
      if (*optarg == '\0' || *optarg == 'I' || *optarg == 'E')
	o.pingtype |= PINGTYPE_ICMP_PING;
      else if (*optarg == 'M') 
	o.pingtype |= PINGTYPE_ICMP_MASK;
      else if (*optarg == 'P') 
	o.pingtype |= PINGTYPE_ICMP_TS;
      else if (*optarg == '0' || *optarg == 'N' || *optarg == 'D')      
	o.pingtype = PINGTYPE_NONE;
      else if (*optarg == 'R')
	o.pingtype |= PINGTYPE_ARP;
      else if (*optarg == 'S') {
	o.pingtype |= (PINGTYPE_TCP|PINGTYPE_TCP_USE_SYN);
	if (isdigit((int) *(optarg+1)))
	  {
	    o.num_ping_synprobes = numberlist2array(optarg+1, o.ping_synprobes, sizeof(o.ping_synprobes), &proberr);
	    if (o.num_ping_synprobes < 0) {
	      fatal("Bogus argument to -PS: %s", proberr);
	    }
	  }
	if (o.num_ping_synprobes == 0) {
	  o.num_ping_synprobes = 1;
	  o.ping_synprobes[0] = DEFAULT_TCP_PROBE_PORT;
	}
      }
      else if (*optarg == 'T' || *optarg == 'A') {
	/* NmapOps::ValidateOptions() takes care of changing this
	   to SYN if not root or if IPv6 */
	o.pingtype |= (PINGTYPE_TCP|PINGTYPE_TCP_USE_ACK);
	if (isdigit((int) *(optarg+1))) {
	  o.num_ping_ackprobes = numberlist2array(optarg+1, o.ping_ackprobes, sizeof(o.ping_ackprobes), &proberr);
	  if (o.num_ping_ackprobes < 0) {
	    fatal("Bogus argument to -PB: %s", proberr);
	  }
	}
	if (o.num_ping_ackprobes == 0) {
	  o.num_ping_ackprobes = 1;
	  o.ping_ackprobes[0] = DEFAULT_TCP_PROBE_PORT;
	}
      }
      else if (*optarg == 'U') {
	o.pingtype |= (PINGTYPE_UDP);
	if (isdigit((int) *(optarg+1))) {
	  o.num_ping_udpprobes = numberlist2array(optarg+1, o.ping_udpprobes, sizeof(o.ping_udpprobes), &proberr);
	  if (o.num_ping_udpprobes < 0) {
	    fatal("Bogus argument to -PU: %s", proberr);
	  }
	}
	if (o.num_ping_udpprobes == 0) {
	  o.num_ping_udpprobes = 1;
	  o.ping_udpprobes[0] = DEFAULT_UDP_PROBE_PORT;
	}
      }
      else if (*optarg == 'B') {
	o.pingtype = (PINGTYPE_TCP|PINGTYPE_TCP_USE_ACK|PINGTYPE_ICMP_PING);
	if (isdigit((int) *(optarg+1))) {
	  o.num_ping_ackprobes = numberlist2array(optarg+1, o.ping_ackprobes, sizeof(o.ping_ackprobes), &proberr);
	  if (o.num_ping_ackprobes < 0) {
	    fatal("Bogus argument to -PB: %s", proberr);
	  }
	}
	if (o.num_ping_ackprobes == 0) {
	  o.num_ping_ackprobes = 1;
	  o.ping_ackprobes[0] = DEFAULT_TCP_PROBE_PORT;
	}
      } else if (*optarg == 'O') {
	fatal("-PO (the letter O)? No such option. Perhaps you meant to disable pings with -P0 (Zero).");
      } else { 
	fatal("Illegal Argument to -P, use -P0, -PI, -PB, -PE, -PM, -PP, -PA, -PU, -PT, or -PT80 (or whatever number you want for the TCP probe destination port)"); 
      }
      break;
    case 'p': 
      if (ports || portlist)
	fatal("Only 1 -p option allowed, separate multiple ranges with commas.");
      portlist = strdup(optarg);
      break;
    case 'q': quashargv++; break;
    case 'R': o.resolve_all++; break;
    case 'r': 
      randomize = 0;
      break;
    case 'S': 
      if (o.spoofsource)
	fatal("You can only use the source option once!  Use -D <decoy1> -D <decoy2> etc. for decoys\n");
      if (resolve(optarg, &ss, &sslen, o.af()) == 0) {
	fatal("Failed to resolve/decode supposed %s source address %s. Note that if you are using IPv6, the -6 argument must come before -S", (o.af() == AF_INET)? "IPv4" : "IPv6", optarg);
      }
      o.setSourceSockAddr(&ss, sslen);
      o.spoofsource = 1;
      break;
    case 's': 
      if (!*optarg) {
	fprintf(stderr, "An option is required for -s, most common are -sT (tcp scan), -sS (SYN scan), -sF (FIN scan), -sU (UDP scan) and -sP (Ping scan)");
	printusage(argv[0], -1);
      }
      p = optarg;
      while(*p) {
	switch(*p) {
	case 'A': o.ackscan = 1; break;
	case 'B':  fatal("No scan type 'B', did you mean bounce scan (-b)?");
	  break;
	case 'F':  o.finscan = 1; break;
	case 'L':  o.listscan = 1; o.pingtype = PINGTYPE_NONE; break;
	case 'M':  o.maimonscan = 1; break;
	case 'N':  o.nullscan = 1; break;
	case 'O':  o.ipprotscan = 1; break;
	case 'P':  o.pingscan = 1; break;
	case 'R':  o.rpcscan = 1; break;
	case 'S':  o.synscan = 1; break;	  
	case 'W':  o.windowscan = 1; break;
	case 'T':  o.connectscan = 1; break;
	case 'V':  o.servicescan = 1; break;
	case 'U':  
	  o.udpscan++;
	  break;
	case 'X':  o.xmasscan++;break;
	default:  error("Scantype %c not supported\n",*p); printusage(argv[0], -1); break;
	}
	p++;
      }
      break;
    case 'T':
      if (*optarg == '0' || (strcasecmp(optarg, "Paranoid") == 0)) {
	o.timing_level = 0;
	o.max_parallelism = 1;
	o.scan_delay = 300000;
	o.setInitialRttTimeout(300000);
      } else if (*optarg == '1' || (strcasecmp(optarg, "Sneaky") == 0)) {
	o.timing_level = 1;
	o.max_parallelism = 1;
	o.scan_delay = 15000;
	o.setInitialRttTimeout(15000);
      } else if (*optarg == '2' || (strcasecmp(optarg, "Polite") == 0)) {
	o.timing_level = 2;
	o.max_parallelism = 1;
	o.scan_delay = 400;
      } else if (*optarg == '3' || (strcasecmp(optarg, "Normal") == 0)) {
      } else if (*optarg == '4' || (strcasecmp(optarg, "Aggressive") == 0)) {
	o.timing_level = 4;
	o.setMinRttTimeout(100);
	o.setMaxRttTimeout(1250);
	o.setInitialRttTimeout(500);
        o.setMaxTCPScanDelay(10);
        o.setMaxRetransmissions(6);
      } else if (*optarg == '5' || (strcasecmp(optarg, "Insane") == 0)) {
	o.timing_level = 5;
	o.setMinRttTimeout(50);
	o.setMaxRttTimeout(300);
	o.setInitialRttTimeout(250);
	o.host_timeout = 900000;
        o.setMaxTCPScanDelay(5);
        o.setMaxRetransmissions(2);
      } else {
	fatal("Unknown timing mode (-T argument).  Use either \"Paranoid\", \"Sneaky\", \"Polite\", \"Normal\", \"Aggressive\", \"Insane\" or a number from 0 (Paranoid) to 5 (Insane)");
      }
      break;
    case 'V': 
      printf("\n%s version %s ( %s )\n", NMAP_NAME, NMAP_VERSION, NMAP_URL); 
      exit(0);
      break;
    case 'v': o.verbose++; break;
    }
  }

#ifdef WIN32
    win_init();
#endif

  tty_init(); // Put the keyboard in raw mode

#if HAVE_SIGNAL
  if (!o.debugging)
    signal(SIGSEGV, sigdie); 
#endif

  // After the arguments are fully processed we now make any of the timing
  // tweaks the user might've specified:
  if (pre_max_parallelism != -1) o.max_parallelism = pre_max_parallelism;
  if (pre_scan_delay != -1) {
    o.scan_delay = pre_scan_delay;
    if (o.scan_delay > o.maxTCPScanDelay()) o.setMaxTCPScanDelay(o.scan_delay);
    if (o.scan_delay > o.maxUDPScanDelay()) o.setMaxUDPScanDelay(o.scan_delay);
    o.max_parallelism = 1;
    if(pre_max_parallelism != -1)
      fatal("You can't use --max-parallelism with --scan-delay.");
  }
  if (pre_max_scan_delay != -1) {
    o.setMaxTCPScanDelay(pre_max_scan_delay);
    o.setMaxUDPScanDelay(pre_max_scan_delay);
  }
  if (pre_init_rtt_timeout != -1) o.setInitialRttTimeout(pre_init_rtt_timeout);
  if (pre_min_rtt_timeout != -1) o.setMinRttTimeout(pre_min_rtt_timeout);
  if (pre_max_rtt_timeout != -1) o.setMaxRttTimeout(pre_max_rtt_timeout);
  if (pre_max_retries != -1) o.setMaxRetransmissions(pre_max_retries);
  if (pre_host_timeout != -1) o.host_timeout = pre_host_timeout;


  if (o.osscan == OS_SCAN_SYS_1_ONLY)
    o.reference_FPs1 = parse_fingerprint_reference_file("nmap-os-fingerprints");
  else if (o.osscan == OS_SCAN_DEFAULT || o.osscan == OS_SCAN_SYS_2_ONLY)
    o.reference_FPs = parse_fingerprint_reference_file("nmap-os-db");

  o.ValidateOptions();

  // print ip options
  if((o.debugging || o.packetTrace()) && o.ipoptionslen){
    char buf[256]; // 256 > 5*40
    bintohexstr(buf, sizeof(buf), (char*)o.ipoptions, o.ipoptionslen);
    if(o.ipoptionslen>=8)	// at least one ip address
    log_write(LOG_STDOUT, "Binary ip options to be send:\n%s", buf);
    log_write(LOG_STDOUT, "Parsed ip options to be send:\n%s\n", 
    	print_ip_options(o.ipoptions, o.ipoptionslen));
  }

  /* Open the log files, now that we know whether the user wants them appended
     or overwritten */
  if (normalfilename)
    log_open(LOG_NORMAL, o.append_output, normalfilename);
  if (machinefilename)
    log_open(LOG_MACHINE, o.append_output, machinefilename);
  if (kiddiefilename)
    log_open(LOG_SKID, o.append_output, kiddiefilename);
  if (xmlfilename)
    log_open(LOG_XML, o.append_output, xmlfilename);

  if (!o.interactivemode) {
    char tbuf[128];
    struct tm *tm;
    time_t now = time(NULL);
    if (!(tm = localtime(&now))) 
      fatal("Unable to get current localtime()#!#");
    // ISO 8601 date/time -- http://www.cl.cam.ac.uk/~mgk25/iso-time.html 
    if (strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M %Z", tm) <= 0)
      fatal("Unable to properly format time");
    log_write(LOG_STDOUT|LOG_SKID, "\nStarting %s %s ( %s ) at %s\n", NMAP_NAME, NMAP_VERSION, NMAP_URL, tbuf);
    if (o.verbose && tm->tm_mon == 8 && tm->tm_mday == 1) {
      log_write(LOG_STDOUT|LOG_SKID, "Happy %dth Birthday to Nmap, may it live to be %d!\n", tm->tm_year - 97, tm->tm_year + 3 );
    }
    if (iflist) {
      print_iflist();
      exit(0);
    }
  }

  if ((o.pingscan || o.listscan) && (portlist || fastscan)) {
    fatal("You cannot use -F (fast scan) or -p (explicit port selection) with PING scan or LIST scan");
  }

  if (portlist) {
    ports = getpts(portlist);
    if (!ports)
      fatal("Your port specification string is not parseable");
    free(portlist);
    portlist = NULL;
  }

  if (fastscan && ports) {
    fatal("You can specify fast scan (-F) or explicitly select individual ports (-p), but not both");
  } else if (fastscan && o.ipprotscan) {
    ports = getfastprots();
  } else if (fastscan) {
    ports = getfastports(o.TCPScan(), o.UDPScan());
  }

#ifdef WIN32
  if (o.sendpref & PACKET_SEND_IP) {
	  error("WARNING: raw IP (rather than raw ethernet) packet sending attempted on Windows. This probably won't work.  Consider --send-eth next time.\n");
  }
#endif
  if (spoofmac) {
    u8 mac_data[6];
    int pos = 0; /* Next index of mac_data to fill in */
    char tmphex[3];
    /* A zero means set it all randomly.  Anything that is all digits
       or colons is treated as a prefix, with remaining characters for
       the 6-byte MAC (if any) chosen randomly.  Otherwise, it is
       treated as a vendor string for lookup in nmap-mac-prefixes */
    if (strcmp(spoofmac, "0") == 0) {
      pos = 0;
    } else {
      const char *p = spoofmac;
      while(*p) { 
	if (*p == ':') p++;
	if (isxdigit(*p) && isxdigit(*(p+1))) {
	  if (pos >= 6) fatal("Bogus --spoof-mac value encountered (%s) -- only up to 6 bytes permitted", spoofmac);
	  tmphex[0] = *p; tmphex[1] = *(p+1); tmphex[2] = '\0';
	  mac_data[pos] = (u8) strtol(tmphex, NULL, 16);
	  pos++;
	  p += 2;
	} else break;
      }
      if (*p) {
	/* Failed to parse it as a MAC prefix -- treating as a vendor substring instead */
	if (!MACCorp2Prefix(spoofmac, mac_data))
	  fatal("Could not parse as a prefix nor find as a vendor substring the given --spoof-mac argument: %s.  If you are giving hex digits, there must be an even number of them.", spoofmac);
	pos = 3;
      }
    }
    if (pos < 6) {
      get_random_bytes(mac_data + pos, 6 - pos);
    }
    /* Got the new MAC! */
    const char *vend = MACPrefix2Corp(mac_data);
    log_write(LOG_NORMAL|LOG_SKID|LOG_STDOUT, 
	      "Spoofing MAC address %02X:%02X:%02X:%02X:%02X:%02X (%s)\n",
	      mac_data[0], mac_data[1], mac_data[2], mac_data[3], mac_data[4],
	      mac_data[5], vend? vend : "No registered vendor");
    o.setSpoofMACAddress(mac_data);

    /* If they want to spoof the MAC address, we should at least make
       some effort to actually send raw ethernet frames rather than IP
       packets (which would use the real IP */
    if (o.sendpref != PACKET_SEND_IP_STRONG)
      o.sendpref = PACKET_SEND_ETH_STRONG;
  }

  if (!ports) {
    if (o.ipprotscan) {
      ports = getdefaultprots();
    } else {
      ports = getdefaultports(o.TCPScan(), o.UDPScan());
    }
  }

  /* By now, we've got our port lists.  Give the user a warning if no 
   * ports are specified for the type of scan being requested.  Other things
   * (such as OS ident scan) might break cause no ports were specified,  but
   * we've given our warning...
   */
  if ((o.TCPScan()) && ports->tcp_count == 0)
    error("WARNING: a TCP scan type was requested, but no tcp ports were specified.  Skipping this scan type.");
  if (o.UDPScan() && ports->udp_count == 0)
    error("WARNING: UDP scan was requested, but no udp ports were specified.  Skipping this scan type.");
  if (o.ipprotscan && ports->prot_count == 0)
    error("WARNING: protocol scan was requested, but no protocols were specified to be scanned.  Skipping this scan type.");

  /* Set up our array of decoys! */
  if (o.decoyturn == -1) {
    o.decoyturn = (o.numdecoys == 0)?  0 : get_random_uint() % o.numdecoys; 
    o.numdecoys++;
    for(i=o.numdecoys-1; i > o.decoyturn; i--)
      o.decoys[i] = o.decoys[i-1];
  }

  /* We need to find what interface to route through if:
   * --None have been specified AND
   * --We are root and doing tcp ping OR
   * --We are doing a raw sock scan and NOT pinging anyone */
  if (o.af() == AF_INET && o.v4sourceip() && !*o.device) {
    if (ipaddr2devname(o.device, o.v4sourceip()) != 0) {
      fatal("Could not figure out what device to send the packet out on with the source address you gave me!  If you are trying to sp00f your scan, this is normal, just give the -e eth0 or -e ppp0 or whatever.  Otherwise you can still use -e, but I find it kindof fishy.");
    }
  }

  if (o.af() == AF_INET && *o.device && !o.v4sourceip()) {
    struct sockaddr_in tmpsock;
    memset(&tmpsock, 0, sizeof(tmpsock));
    if (devname2ipaddr(o.device, &(tmpsock.sin_addr)) == -1) {
      fatal("I cannot figure out what source address to use for device %s, does it even exist?", o.device);
    }
    tmpsock.sin_family = AF_INET;
#if HAVE_SOCKADDR_SA_LEN
    tmpsock.sin_len = sizeof(tmpsock);
#endif
    o.setSourceSockAddr((struct sockaddr_storage *) &tmpsock, sizeof(tmpsock));
  }


  /* If he wants to bounce off of an ftp site, that site better damn well be reachable! */
  if (o.bouncescan) {
	  if (!inet_pton(AF_INET, ftp.server_name, &ftp.server)) {
      if ((target = gethostbyname(ftp.server_name)))
	memcpy(&ftp.server, target->h_addr_list[0], 4);
      else {
	fprintf(stderr, "Failed to resolve ftp bounce proxy hostname/IP: %s\n",
		ftp.server_name);
	exit(1);
      } 
    }  else if (o.verbose)
      log_write(LOG_STDOUT, "Resolved ftp bounce attack proxy to %s (%s).\n", 
		ftp.server_name, inet_ntoa(ftp.server)); 
  }
  fflush(stdout);
  fflush(stderr);

  timep = time(NULL);
  
  /* Brief info incase they forget what was scanned */
  Strncpy(mytime, ctime(&timep), sizeof(mytime));
  chomp(mytime);
  char *xslfname = o.XSLStyleSheet();
  char xslline[1024];
  if (xslfname) {
    char *p = xml_convert(xslfname);
    snprintf(xslline, sizeof(xslline), "<?xml-stylesheet href=\"%s\" type=\"text/xsl\"?>\n", p);
    free(p);
  }  else xslline[0] = '\0';
  log_write(LOG_XML, "<?xml version=\"1.0\" ?>\n%s<!-- ", xslline);
  log_write(LOG_NORMAL|LOG_MACHINE, "# ");
  log_write(LOG_NORMAL|LOG_MACHINE|LOG_XML, "%s %s scan initiated %s as: ", NMAP_NAME, NMAP_VERSION, mytime);
  
  for(i=0; i < argc; i++) {
    char *p = xml_convert(fakeargv[i]);
    log_write(LOG_XML,"%s ", p);
    free(p);
    log_write(LOG_NORMAL|LOG_MACHINE,"%s ", fakeargv[i]);
  }
  log_write(LOG_XML, "-->");
  log_write(LOG_NORMAL|LOG_MACHINE|LOG_XML,"\n");  

  log_write(LOG_XML, "<nmaprun scanner=\"nmap\" args=\"");
  for(i=0; i < argc; i++) 
    log_write(LOG_XML, (i == argc-1)? "%s\" " : "%s ", fakeargv[i]);

  log_write(LOG_XML, "start=\"%lu\" startstr=\"%s\" version=\"%s\" xmloutputversion=\"1.01\">\n",
	    (unsigned long) timep, mytime, NMAP_VERSION);

  output_xml_scaninfo_records(ports);

  log_write(LOG_XML, "<verbose level=\"%d\" />\n<debugging level=\"%d\" />\n",
	    o.verbose, o.debugging);

  /* Before we randomize the ports scanned, lets output them to machine 
     parseable output */
  if (o.verbose)
    output_ports_to_machine_parseable_output(ports, o.TCPScan(), o.udpscan, o.ipprotscan);

  /* more fakeargv junk, BTW malloc'ing extra space in argv[0] doesn't work */
  if (quashargv) {
    size_t fakeargvlen = strlen(FAKE_ARGV), argvlen = strlen(argv[0]);
    if (argvlen < fakeargvlen)
      fatal("If you want me to fake your argv, you need to call the program with a longer name.  Try the full pathname, or rename it fyodorssuperdedouperportscanner");
    strncpy(argv[0], FAKE_ARGV, fakeargvlen);
    memset(&argv[0][fakeargvlen], '\0', strlen(&argv[0][fakeargvlen]));
    for(i=1; i < argc; i++)
      memset(argv[i], '\0', strlen(argv[i]));
  }

#if defined(HAVE_SIGNAL) && defined(SIGPIPE)
  signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE so our program doesn't crash because
			       of it, but we really shouldn't get an unsuspected
			       SIGPIPE */
#endif

  if (o.max_parallelism && (i = max_sd()) && i < o.max_parallelism) {
    fprintf(stderr, "WARNING:  Your specified max_parallel_sockets of %d, but your system says it might only give us %d.  Trying anyway\n", o.max_parallelism, i);
  }

  if (o.debugging > 1) log_write(LOG_STDOUT, "The max # of sockets we are using is: %d\n", o.max_parallelism);

  // At this point we should fully know our timing parameters
  if (o.debugging) {
    printf("--------------- Timing report ---------------\n");
    printf("  hostgroups: min %d, max %d\n", o.minHostGroupSz(), o.maxHostGroupSz());
    printf("  rtt-timeouts: init %d, min %d, max %d\n", o.initialRttTimeout(), o.minRttTimeout(), o.maxRttTimeout());
    printf("  msx-scan-delay: TCP %d, UDP %d\n", o.maxTCPScanDelay(), o.maxUDPScanDelay());
    printf("  parallelism: min %d, max %d\n", o.min_parallelism, o.max_parallelism);
    printf("  max-retries: %d, host-timeout: %ld\n", o.getMaxRetransmissions(), o.host_timeout);
    printf("---------------------------------------------\n");
  }

  /* Before we randomize the ports scanned, we must initialize PortList class. */
  if (o.ipprotscan)
    PortList::initializePortMap(IPPROTO_IP,  ports->prots, ports->prot_count);
  if (o.TCPScan())
    PortList::initializePortMap(IPPROTO_TCP, ports->tcp_ports, ports->tcp_count);
  if (o.UDPScan())
    PortList::initializePortMap(IPPROTO_UDP, ports->udp_ports, ports->udp_count);
  
  if  (randomize) {
    if (ports->tcp_count) {
      shortfry(ports->tcp_ports, ports->tcp_count); 
      // move a few more common ports closer to the beginning to speed scan
      random_port_cheat(ports->tcp_ports, ports->tcp_count);
    }
    if (ports->udp_count) 
      shortfry(ports->udp_ports, ports->udp_count); 
    if (ports->prot_count) 
      shortfry(ports->prots, ports->prot_count); 
  }
  
  /* Time to create a hostgroup state object filled with all the requested
     machines */
  host_exp_group = (char **) safe_malloc(o.ping_group_sz * sizeof(char *));

  /* lets load our exclude list */
  if ((NULL != excludefd) || (NULL != exclude_spec)) {
    exclude_group = load_exclude(excludefd, exclude_spec);

    if (o.debugging > 3)
      dumpExclude(exclude_group);

    if ((FILE *)NULL != excludefd)
      fclose(excludefd);
    if ((char *)NULL != exclude_spec)
      free(exclude_spec);
  }

  while(num_host_exp_groups < o.ping_group_sz &&
	(host_spec = grab_next_host_spec(inputfd, argc, fakeargv))) {
    host_exp_group[num_host_exp_groups++] = strdup(host_spec);
    // For purposes of random scan
    if (o.max_ips_to_scan && o.max_ips_to_scan <= o.numhosts_scanned + num_host_exp_groups)
      break;
  }

  if (num_host_exp_groups == 0)
    fatal("No target machines/networks specified!");
  hstate = new HostGroupState(o.ping_group_sz, o.randomize_hosts,
			      host_exp_group, num_host_exp_groups);

  do {
    ideal_scan_group_sz = determineScanGroupSize(o.numhosts_scanned, ports);
    while(Targets.size() < ideal_scan_group_sz) {
      o.current_scantype = HOST_DISCOVERY;
      currenths = nexthost(hstate, exclude_group, ports, &(o.pingtype));
      if (!currenths) {
	/* Try to refill with any remaining expressions */
	/* First free the old ones */
	for(i=0; i < num_host_exp_groups; i++)
	  free(host_exp_group[i]);
	num_host_exp_groups = 0;
	/* Now grab any new expressions */
	while(num_host_exp_groups < o.ping_group_sz && 
	      (!o.max_ips_to_scan ||  o.max_ips_to_scan > o.numhosts_scanned + num_host_exp_groups) &&
	      (host_spec = grab_next_host_spec(inputfd, argc, fakeargv))) {
	  // For purposes of random scan
	  host_exp_group[num_host_exp_groups++] = strdup(host_spec);
	}
	if (num_host_exp_groups == 0)
	  break;
	delete hstate;
	hstate = new HostGroupState(o.ping_group_sz, o.randomize_hosts,
				    host_exp_group, num_host_exp_groups);
      
	/* Try one last time -- with new expressions */
	currenths = nexthost(hstate, exclude_group, ports, &(o.pingtype));
	if (!currenths)
	  break;
      }
      o.numhosts_scanned++;
    
      if (currenths->flags & HOST_UP && !o.listscan) 
	o.numhosts_up++;
    
      if (o.pingscan || o.listscan) {
	/* We're done with the hosts */
	log_write(LOG_XML, "<host>");
	write_host_status(currenths, o.resolve_all);
	printmacinfo(currenths);
	//	if (currenths->flags & HOST_UP)
	//  log_write(LOG_NORMAL|LOG_SKID|LOG_STDOUT,"\n");
	log_write(LOG_XML, "</host>\n");
	log_flush_all();
	delete currenths;
	continue;
      }
    
      if (o.spoofsource) {
	o.SourceSockAddr(&ss, &sslen);
	currenths->setSourceSockAddr(&ss, sslen);
      }
    
      /* I used to check that !currenths->wierd_responses, but in some
	 rare cases, such IPs CAN be port successfully scanned and even connected to */
      if (!(currenths->flags & HOST_UP)) {
	delete currenths;
	continue;
      }

      if (o.af() == AF_INET && o.RawScan()) { 
	if (currenths->SourceSockAddr(NULL, NULL) != 0) {
	  if (o.SourceSockAddr(&ss, &sslen) == 0) {
	    currenths->setSourceSockAddr(&ss, sslen);
	  } else {
	    if (gethostname(myname, MAXHOSTNAMELEN) || 
		resolve(myname, &ss, &sslen, o.af()) == 0)
	      fatal("Cannot get hostname!  Try using -S <my_IP_address> or -e <interface to scan through>\n"); 
	    
	    o.setSourceSockAddr(&ss, sslen);
	    currenths->setSourceSockAddr(&ss, sslen);
	    if (! sourceaddrwarning) {
	      fprintf(stderr, "WARNING:  We could not determine for sure which interface to use, so we are guessing %s .  If this is wrong, use -S <my_IP_address>.\n", inet_socktop(&ss));
	      sourceaddrwarning = 1;
	    }
	  }
	}
	if (!currenths->deviceName())
	  fatal("Do not have appropriate device name for target");
	
	/* Groups should generally use the same device as properties
	   change quite a bit between devices.  Plus dealing with a
	   multi-device group can be a pain programmatically. So if
	   this Target has a different device the rest, we give it
	   back. */
	if (Targets.size() > 0 && 
	    strcmp(Targets[Targets.size() - 1]->deviceName(), currenths->deviceName())) {
	  returnhost(hstate);
	  o.numhosts_scanned--; o.numhosts_up--;
	  break;
	}
	o.decoys[o.decoyturn] = currenths->v4source();
      }
      Targets.push_back(currenths);
    }
    
    if (Targets.size() == 0)
      break; /* Couldn't find any more targets */
    
    // Set the variable for status printing
    o.numhosts_scanning = Targets.size();
    
    // Our source must be set in decoy list because nexthost() call can
    // change it (that issue really should be fixed when possible)
    if (o.af() == AF_INET && o.RawScan())
      o.decoys[o.decoyturn] = Targets[0]->v4source();
    
    /* I now have the group for scanning in the Targets vector */

    // Ultra_scan sets o.scantype for us so we don't have to worry
    if (o.synscan)
      ultra_scan(Targets, ports, SYN_SCAN);
    
    if (o.ackscan)
      ultra_scan(Targets, ports, ACK_SCAN);
    
    if (o.windowscan)
      ultra_scan(Targets, ports, WINDOW_SCAN);
    
    if (o.finscan)
      ultra_scan(Targets, ports, FIN_SCAN);
    
    if (o.xmasscan)
      ultra_scan(Targets, ports, XMAS_SCAN);
    
    if (o.nullscan)
      ultra_scan(Targets, ports, NULL_SCAN);
    
    if (o.maimonscan)
      ultra_scan(Targets, ports, MAIMON_SCAN);
    
    if (o.udpscan)
      ultra_scan(Targets, ports, UDP_SCAN);
    
    if (o.connectscan)
      ultra_scan(Targets, ports, CONNECT_SCAN);
    
    if (o.ipprotscan)
      ultra_scan(Targets, ports, IPPROT_SCAN);
    
    /* These lame functions can only handle one target at a time */
    for(targetno = 0; targetno < Targets.size(); targetno++) {
      currenths = Targets[targetno];
      if (o.idlescan) {
         o.current_scantype = IDLE_SCAN;
         keyWasPressed(); // Check if a status message should be printed
         idle_scan(currenths, ports->tcp_ports, 
				ports->tcp_count, idleProxy);
      }
      if (o.bouncescan) {
         o.current_scantype = BOUNCE_SCAN;
         keyWasPressed(); // Check if a status message should be printed
	if (ftp.sd <= 0) ftp_anon_connect(&ftp);
	if (ftp.sd > 0) bounce_scan(currenths, ports->tcp_ports, 
				    ports->tcp_count, &ftp);
      }
    }

    if (o.servicescan) {
      o.current_scantype = SERVICE_SCAN; 
      keyWasPressed(); // Check if a status message should be printed
      service_scan(Targets);
    }

    if (o.osscan == OS_SCAN_DEFAULT || o.osscan == OS_SCAN_SYS_2_ONLY)
	  os_scan2(Targets);

    for(targetno = 0; targetno < Targets.size(); targetno++) {
      currenths = Targets[targetno];
    
      /* This scantype must be after any TCP or UDP scans since it
       * get's it's port scan list from the open port list of the current
       * host rather than port list the user specified.
       */
      if (o.servicescan || o.rpcscan)  pos_scan(currenths, NULL, 0, RPC_SCAN);

      // Should be host parallelized.  Though rarely takes a huge amt. of time.
      if (o.osscan == OS_SCAN_SYS_1_ONLY) {
	os_scan(currenths);
      }

    /* Now I can do the output and such for each host */
      log_write(LOG_XML, "<host>");
      write_host_status(currenths, o.resolve_all);
      if (currenths->timedOut(NULL)) {
	log_write(LOG_NORMAL|LOG_SKID|LOG_STDOUT,"Skipping host %s due to host timeout\n", 
		  currenths->NameIP(hostname, sizeof(hostname)));
	log_write(LOG_MACHINE,"Host: %s (%s)\tStatus: Timeout", 
		  currenths->targetipstr(), currenths->HostName());
      } else {
	printportoutput(currenths, &currenths->ports);
	printmacinfo(currenths);
	printosscanoutput(currenths);
	printserviceinfooutput(currenths);
      }
    
      if (o.debugging) 
	log_write(LOG_STDOUT, "Final times for host: srtt: %d rttvar: %d  to: %d\n", 
		  currenths->to.srtt, currenths->to.rttvar, currenths->to.timeout);
      log_write(LOG_NORMAL|LOG_SKID|LOG_STDOUT|LOG_MACHINE,"\n");
      log_write(LOG_XML, "</host>\n");
    }
    log_flush_all();
  
    /* Free all of the Targets */
    while(!Targets.empty()) {
      currenths = Targets.back();
      delete currenths;
      Targets.pop_back();
    }
    o.numhosts_scanning = 0;
  } while(!o.max_ips_to_scan || o.max_ips_to_scan > o.numhosts_scanned);
  
  delete hstate;
  if (exclude_group)
    delete[] exclude_group;

  hstate = NULL;

  /* Free host expressions */
  for(i=0; i < num_host_exp_groups; i++)
    free(host_exp_group[i]);
  num_host_exp_groups = 0;
  free(host_exp_group);

  printfinaloutput();

  free_scan_lists(ports);

  eth_close_cached();

  if(o.release_memory || o.interactivemode) {
    /* Free fake argv */
    for(i=0; i < argc; i++)
      free(fakeargv[i]);
    free(fakeargv);

    nmap_free_mem();
  }
  return 0;
}      
      
// Free some global memory allocations.
// This is used for detecting memory leaks.
void nmap_free_mem() {
  PortList::freePortMap();
  cp_free();
  free_dns_servers();
  free_etchosts();
  if(o.reference_FPs){
    free_fingerprint_file(o.reference_FPs);
    o.reference_FPs = NULL;
  }
  AllProbes::service_scan_free();
  
}

/* Reads in a (normal or machine format) Nmap log file and gathers enough
   state to allow Nmap to continue where it left off.  The important things
   it must gather are:
   1) The last host completed
   2) The command arguments
*/
   
int gather_logfile_resumption_state(char *fname, int *myargc, char ***myargv) {
  char *filestr;
  int filelen;
  char nmap_arg_buffer[1024];
  struct in_addr lastip;
  char *p, *q, *found; /* I love C! */
  /* We mmap it read/write since we will change the last char to a newline if it is not already */
  filestr = mmapfile(fname, &filelen, O_RDWR);
  if (!filestr) {
    fatal("Could not mmap() %s read/write", fname);
  }

  if (filelen < 20) {
    fatal("Output file %s is too short -- no use resuming", fname);
  }

  /* For now we terminate it with a NUL, but we will terminate the file with
     a '\n' later */
  filestr[filelen - 1] = '\0';

  /* First goal is to find the nmap args */
  if ((p = strstr(filestr, " as: ")))
    p += 5;
  else fatal("Unable to parse supposed log file %s.  Are you sure this is an Nmap output file?", fname);
  while(*p && !isspace((int) *p))
    p++;
  if (!*p) fatal("Unable to parse supposed log file %s.  Sorry", fname);
  p++; /* Skip the space between program name and first arg */
  if (*p == '\n' || !*p) fatal("Unable to parse supposed log file %s.  Sorry", fname);

  q = strchr(p, '\n');
  if (!q || ((unsigned int) (q - p) >= sizeof(nmap_arg_buffer) - 32))
    fatal("Unable to parse supposed log file %s.  Perhaps the Nmap execution had not finished at least one host?  In that case there is no use \"resuming\"", fname);


  strcpy(nmap_arg_buffer, "nmap --append-output ");
  if ((q-p) + 21 + 1 >= (int) sizeof(nmap_arg_buffer)) fatal("0verfl0w");
  memcpy(nmap_arg_buffer + 21, p, q-p);
  nmap_arg_buffer[21 + q-p] = '\0';

  if (strstr(nmap_arg_buffer, "--randomize-hosts") != NULL) {
    error("WARNING:  You are attempting to resume a scan which used --randomize-hosts.  Some hosts in the last randomized batch may be missed and others may be repeated once");
  }

  *myargc = arg_parse(nmap_arg_buffer, myargv);
  if (*myargc == -1) {  
    fatal("Unable to parse supposed log file %s.  Sorry", fname);
  }
     
  /* Now it is time to figure out the last IP that was scanned */
  q = p;
  found = NULL;
  /* Lets see if its a machine log first */
  while((q = strstr(q, "\nHost: ")))
    found = q = q + 7;

  if (found) {
    q = strchr(found, ' ');
    if (!q) fatal("Unable to parse supposed log file %s.  Sorry", fname);
    *q = '\0';
    if (inet_pton(AF_INET, found, &lastip) == 0)
      fatal("Unable to parse supposed log file %s.  Sorry", fname);
    *q = ' ';
  } else {
    /* OK, I guess (hope) it is a normal log then */
    q = p;
    found = NULL;
    while((q = strstr(q, "\nInteresting ports on ")))
      found = q++;

    /* There may be some later IPs of the form 'All [num] scanned ports on  ([ip]) are: state */
    if (found) q = found;
    if (q) {    
      while((q = strstr(q, "\nAll "))) {
	q+= 5;
	while(isdigit(*q)) q++;
	if (strncmp(q, " scanned ports on", 17) == 0)
	  found = q;
      }
    }

    if (found) {    
      found = strchr(found, '(');
      if (!found) fatal("Unable to parse supposed log file %s.  Sorry", fname);
      found++;
      q = strchr(found, ')');
      if (!q) fatal("Unable to parse supposed log file %s.  Sorry", fname);
      *q = '\0';
      if (inet_pton(AF_INET, found, &lastip) == 0)
	fatal("Unable to parse ip (%s) supposed log file %s.  Sorry", found, fname);
      *q = ')';
    } else {
      error("Warning: You asked for --resume but it doesn't look like any hosts in the log file were successfully scanned.  Starting from the beginning.");
      lastip.s_addr = 0;
    }
  }
  o.resume_ip = lastip;

  /* Ensure the log file ends with a newline */
  filestr[filelen - 1] = '\n';
  munmap(filestr, filelen);
  return 0;
}

/* We set the socket lingering so we will RST connection instead of wasting
   bandwidth with the four step close  */
void init_socket(int sd) {
  struct linger l;
  int res;
  static int bind_failed=0;
  struct sockaddr_storage ss;
  size_t sslen;

  l.l_onoff = 1;
  l.l_linger = 0;

  if (setsockopt(sd, SOL_SOCKET, SO_LINGER,  (const char *) &l, sizeof(struct linger)))
    {
      fprintf(stderr, "Problem setting socket SO_LINGER, errno: %d\n", socket_errno());
      perror("setsockopt");
    }
  if (o.spoofsource && !bind_failed)
    {
      o.SourceSockAddr(&ss, &sslen);
      res=bind(sd, (struct sockaddr*)&ss, sslen);
      if (res<0)
	{
	  fprintf(stderr, "init_socket: Problem binding source address (%s), errno :%d\n", inet_socktop(&ss), socket_errno());
	  perror("bind");
	  bind_failed=1;
	}
    }
}

/* Convert a string like "-100,200-1024,3000-4000,60000-" into an array 
   of port numbers. Note that one trailing comma is OK -- this is actually
   useful for machine generated lists */
struct scan_lists *getpts(char *origexpr) {
  u8 *porttbl;
  int portwarning = 0; /* have we warned idiot about dup ports yet? */
  long rangestart = -2343242, rangeend = -9324423;
  char *current_range;
  char *endptr;
  int i;
  int tcpportcount = 0, udpportcount = 0, protcount = 0;
  struct scan_lists *ports;
  int range_type = 0;

  if (o.TCPScan())
    range_type |= SCAN_TCP_PORT;
  if (o.UDPScan())
    range_type |= SCAN_UDP_PORT;
  if (o.ipprotscan)
    range_type |= SCAN_PROTOCOLS;

  porttbl = (u8 *) safe_zalloc(65536);

  current_range = origexpr;
  do {
    while(isspace((int) *current_range))
      current_range++; /* I don't know why I should allow spaces here, but I will */
    if (*current_range == 'T' && *++current_range == ':') {
	current_range++;
	range_type = SCAN_TCP_PORT;
	continue;
    }
    if (*current_range == 'U' && *++current_range == ':') {
	current_range++;
	range_type = SCAN_UDP_PORT;
	continue;
    }
    if (*current_range == 'P' && *++current_range == ':') {
	current_range++;
	range_type = SCAN_PROTOCOLS;
	continue;
    }
    if (*current_range == '-') {
      rangestart = 1;
    }
    else if (isdigit((int) *current_range)) {
      rangestart = strtol(current_range, &endptr, 10);
      if (rangestart < 0 || rangestart > 65535) {
	fatal("Ports to be scanned must be between 0 and 65535 inclusive");
      }
/*      if (rangestart == 0) {
	error("WARNING:  Scanning \"port 0\" is supported, but unusual.");
      } */
      current_range = endptr;
      while(isspace((int) *current_range)) current_range++;
    } else {
      fatal("Error #485: Your port specifications are illegal.  Example of proper form: \"-100,200-1024,T:3000-4000,U:60000-\"");
    }
    /* Now I have a rangestart, time to go after rangeend */
    if (!*current_range || *current_range == ',') {
      /* Single port specification */
      rangeend = rangestart;
    } else if (*current_range == '-') {
      current_range++;
      if (!*current_range || *current_range == ',') {
	/* Ended with a -, meaning up until the last possible port */
	rangeend = 65535;
      } else if (isdigit((int) *current_range)) {
	rangeend = strtol(current_range, &endptr, 10);
	if (rangeend < 0 || rangeend > 65535) {
	  fatal("Ports to be scanned must be between 0 and 65535 inclusive");
	}
	current_range = endptr;
      } else {
	fatal("Error #486: Your port specifications are illegal.  Example of proper form: \"-100,200-1024,3000-4000,60000-\"");
      }
    } else {
	fatal("Error #487: Your port specifications are illegal.  Example of proper form: \"-100,200-1024,3000-4000,60000-\"");
    }

    /* Now I have a rangestart and a rangeend, so I can add these ports */
    while(rangestart <= rangeend) {
      if (porttbl[rangestart] & range_type) {
	if (!portwarning) {
	  error("WARNING:  Duplicate port number(s) specified.  Are you alert enough to be using Nmap?  Have some coffee or Jolt(tm).");
	  portwarning++;
	} 
      } else {      
	if (range_type & SCAN_TCP_PORT)
	  tcpportcount++;
	if (range_type & SCAN_UDP_PORT)
	  udpportcount++;
	if (range_type & SCAN_PROTOCOLS && rangestart < 256)
	  protcount++;
	porttbl[rangestart] |= range_type;
      }
      rangestart++;
    }
    
    /* Find the next range */
    while(isspace((int) *current_range)) current_range++;
    if (*current_range && *current_range != ',') {
      fatal("Error #488: Your port specifications are illegal.  Example of proper form: \"-100,200-1024,3000-4000,60000-\"");
    }
    if (*current_range == ',')
      current_range++;
  } while(current_range && *current_range);

  if ( 0 == (tcpportcount + udpportcount + protcount))
    fatal("No ports specified -- If you really don't want to scan any ports use ping scan...");

  ports = (struct scan_lists *) safe_zalloc(sizeof(struct scan_lists));

  if (tcpportcount) {
    ports->tcp_ports = (unsigned short *)safe_zalloc(tcpportcount * sizeof(unsigned short));
  }
  if (udpportcount) {
    ports->udp_ports = (unsigned short *)safe_zalloc(udpportcount * sizeof(unsigned short));
  }
  if (protcount) {
    ports->prots = (unsigned short *)safe_zalloc(protcount * sizeof(unsigned short));
  }
  ports->tcp_count = tcpportcount;
  ports->udp_count = udpportcount;
  ports->prot_count = protcount;

  tcpportcount=0;
  udpportcount=0;
  protcount=0;
  for(i=0; i <= 65535; i++) {
    if (porttbl[i] & SCAN_TCP_PORT)
      ports->tcp_ports[tcpportcount++] = i;
    if (porttbl[i] & SCAN_UDP_PORT)
      ports->udp_ports[udpportcount++] = i;
    if (porttbl[i] & SCAN_PROTOCOLS && i < 256)
      ports->prots[protcount++] = i;
  }

  free(porttbl);
  return ports;
}

void free_scan_lists(struct scan_lists *ports) {
  if (ports) {
    free(ports->tcp_ports);
    free(ports->udp_ports);
    free(ports->prots);
    free(ports);
  }
}

void printinteractiveusage() {
  printf(
	 "Nmap Interactive Commands:\n\
n <nmap args> -- executes an nmap scan using the arguments given and\n\
waits for nmap to finish.  Results are printed to the\n\
screen (of course you can still use file output commands).\n\
! <command>   -- runs shell command given in the foreground\n\
x             -- Exit Nmap\n\
f [--spoof <fakeargs>] [--nmap-path <path>] <nmap args>\n\
-- Executes nmap in the background (results are NOT\n\
printed to the screen).  You should generally specify a\n\
file for results (with -oX, -oG, or -oN).  If you specify\n\
fakeargs with --spoof, Nmap will try to make those\n\
appear in ps listings.  If you wish to execute a special\n\
version of Nmap, specify --nmap-path.\n\
n -h          -- Obtain help with Nmap syntax\n\
h             -- Prints this help screen.\n\
Examples:\n\
n -sS -O -v example.com/24\n\
f --spoof \"/usr/local/bin/pico -z hello.c\" -sS -oN e.log example.com/24\n\n");
}

char *seqreport1(struct seq_info *seq) {
  static char report[512];

  snprintf(report, sizeof(report), "TCP Sequence Prediction: Difficulty=%d (%s)\n", seq->index, seqidx2difficultystr1(seq->index));
  return report;
}

/* Convert a TCP sequence prediction difficulty index like 1264386
   into a difficulty string like "Worthy Challenge */
const char *seqidx2difficultystr1(unsigned long idx) {
  return  (idx < 10)? "Trivial joke" : (idx < 80)? "Easy" : (idx < 3000)? "Medium" : (idx < 5000)? "Formidable" : (idx < 100000)? "Worthy challenge" : "Good luck!";
}

char *seqreport(struct seq_info *seq) {
  static char report[512];

  snprintf(report, sizeof(report), "TCP Sequence Prediction: Difficulty=%d (%s)\n", seq->index, seqidx2difficultystr(seq->index));
  return report;
}

/* Convert a TCP sequence prediction difficulty index like 1264386
   into a difficulty string like "Worthy Challenge */
const char *seqidx2difficultystr(unsigned long idx) {
  return  (idx < 3)? "Trivial joke" : (idx < 6)? "Easy" : (idx < 11)? "Medium" : (idx < 12)? "Formidable" : (idx < 16)? "Worthy challenge" : "Good luck!";
}


char *seqclass2ascii(int seqclass) {
  switch(seqclass) {
  case SEQ_CONSTANT:
    return "constant sequence number (!)";
  case SEQ_64K:
    return "64K rule";
  case SEQ_TD:
    return "trivial time dependency";
  case SEQ_i800:
    return "increments by 800";
  case SEQ_RI:
    return "random positive increments";
  case SEQ_TR:
    return "truly random";
  case SEQ_UNKNOWN:
    return "unknown class";
  default:
    return "ERROR, WTF?";
  }
}

char *ipidclass2ascii(int seqclass) {
  switch(seqclass) {
  case IPID_SEQ_CONSTANT:
    return "Duplicated ipid (!)";
  case IPID_SEQ_INCR:
    return "Incremental";
  case IPID_SEQ_BROKEN_INCR:
    return "Broken little-endian incremental";
  case IPID_SEQ_RD:
    return "Randomized";
  case IPID_SEQ_RPI:
    return "Random positive increments";
  case IPID_SEQ_ZERO:
    return "All zeros";
  case IPID_SEQ_UNKNOWN:
    return "Busy server or unknown class";
  default:
    return "ERROR, WTF?";
  }
}

char *tsseqclass2ascii(int seqclass) {
  switch(seqclass) {
  case TS_SEQ_ZERO:
    return "zero timestamp";
  case TS_SEQ_2HZ:
    return "2HZ";
  case TS_SEQ_100HZ:
    return "100HZ";
  case TS_SEQ_1000HZ:
    return "1000HZ";
  case TS_SEQ_UNSUPPORTED:
    return "none returned (unsupported)";
  case TS_SEQ_UNKNOWN:
    return "unknown class";
  default:
    return "ERROR, WTF?";
  }
}




/* Just a routine for obtaining a string for printing based on the scantype */
char *scantype2str(stype scantype) {

  switch(scantype) {
  case STYPE_UNKNOWN: return "Unknown Scan Type"; break;
  case HOST_DISCOVERY: return "Host Discovery"; break;
  case ACK_SCAN: return "ACK Scan"; break;
  case SYN_SCAN: return "SYN Stealth Scan"; break;
  case FIN_SCAN: return "FIN Scan"; break;
  case XMAS_SCAN: return "XMAS Scan"; break;
  case UDP_SCAN: return "UDP Scan"; break;
  case CONNECT_SCAN: return "Connect() Scan"; break;
  case NULL_SCAN: return "NULL Scan"; break;
  case WINDOW_SCAN: return "Window Scan"; break;
  case RPC_SCAN: return "RPCGrind Scan"; break;
  case MAIMON_SCAN: return "Maimon Scan"; break;
  case IPPROT_SCAN: return "IPProto Scan"; break;
  case PING_SCAN: return "Ping Scan"; break;
  case PING_SCAN_ARP: return "ARP Ping Scan"; break;
  case IDLE_SCAN: return "Idle Scan"; break;
  case BOUNCE_SCAN: return "Bounce Scan"; break;
  case SERVICE_SCAN: return "Service Scan"; break;
  case OS_SCAN: return "OS Scan"; break;
  default: assert(0); break;
  }

  return NULL; /* Unreached */

}

char *statenum2str(int state) {
  switch(state) {
  case PORT_OPEN: return "open"; break;
  case PORT_FILTERED: return "filtered"; break;
  case PORT_UNFILTERED: return "UNfiltered"; break;
  case PORT_CLOSED: return "closed"; break;
  case PORT_OPENFILTERED: return "open|filtered"; break;
  case PORT_CLOSEDFILTERED: return "closed|filtered"; break;
  default: return "unknown"; break;
  }
  return "unknown";
}

int ftp_anon_connect(struct ftpinfo *ftp) {
  int sd;
  struct sockaddr_in sock;
  int res;
  char recvbuf[2048];
  char command[512];

  if (o.verbose || o.debugging) 
    log_write(LOG_STDOUT, "Attempting connection to ftp://%s:%s@%s:%i\n", ftp->user, ftp->pass,
	      ftp->server_name, ftp->port);

  if ((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Couldn't create ftp_anon_connect socket");
    return 0;
  }

  sock.sin_family = AF_INET;
  sock.sin_addr.s_addr = ftp->server.s_addr;
  sock.sin_port = htons(ftp->port); 
  res = connect(sd, (struct sockaddr *) &sock, sizeof(struct sockaddr_in));
  if (res < 0 ) {
    fprintf(stderr, "Your ftp bounce proxy server won't talk to us!\n");
    exit(1);
  }
  if (o.verbose || o.debugging) log_write(LOG_STDOUT, "Connected:");
  while ((res = recvtime(sd, recvbuf, sizeof(recvbuf) - 1,7, NULL)) > 0) 
    if (o.debugging || o.verbose) {
      recvbuf[res] = '\0';
      log_write(LOG_STDOUT, "%s", recvbuf);
    }
  if (res < 0) {
    perror("recv problem from ftp bounce server");
    exit(1);
  }

  snprintf(command, 511, "USER %s\r\n", ftp->user);

  send(sd, command, strlen(command), 0);
  res = recvtime(sd, recvbuf, sizeof(recvbuf) - 1,12, NULL);
  if (res <= 0) {
    perror("recv problem from ftp bounce server");
    exit(1);
  }
  recvbuf[res] = '\0';
  if (o.debugging) log_write(LOG_STDOUT, "sent username, received: %s", recvbuf);
  if (recvbuf[0] == '5') {
    fprintf(stderr, "Your ftp bounce server doesn't like the username \"%s\"\n", 
	    ftp->user);
    exit(1);
  }

  snprintf(command, 511, "PASS %s\r\n", ftp->pass);

  send(sd, command, strlen(command), 0);
  res = recvtime(sd, recvbuf, sizeof(recvbuf) - 1,12, NULL);
  if (res < 0) {
    perror("recv problem from ftp bounce server\n");
    exit(1);
  }
  if (!res) fprintf(stderr, "Timeout from bounce server ...");
  else {
    recvbuf[res] = '\0';
    if (o.debugging) log_write(LOG_STDOUT, "sent password, received: %s", recvbuf);
    if (recvbuf[0] == '5') {
      fprintf(stderr, "Your ftp bounce server refused login combo (%s/%s)\n",
	      ftp->user, ftp->pass);
      exit(1);
    }
  }
  while ((res = recvtime(sd, recvbuf, sizeof(recvbuf) - 1,2, NULL)) > 0) 
    if (o.debugging) {
      recvbuf[res] = '\0';
      log_write(LOG_STDOUT, "%s", recvbuf);
    }
  if (res < 0) {
    perror("recv problem from ftp bounce server");
    exit(1);
  }
  if (o.verbose) log_write(LOG_STDOUT, "Login credentials accepted by ftp server!\n");

  ftp->sd = sd;
  return sd;
}

#ifndef WIN32

void reaper(int signo) {
  int status;
  pid_t pid;

  if ((pid = wait(&status)) == -1) {
    gh_perror("waiting to reap child");
  } else {
    fprintf(stderr, "\n[%d finished status=%d (%s)]\nnmap> ", (int) pid, status, (status == 0)? "success"  : "failure");
  }
}
#endif

void sigdie(int signo) {
  int abt = 0;

  fflush(stdout);

  switch(signo) {
  case SIGINT:
    fprintf(stderr, "caught SIGINT signal, cleaning up\n");
    break;

#ifdef SIGTERM
  case SIGTERM:
    fprintf(stderr, "caught SIGTERM signal, cleaning up\n");
    break;
#endif

#ifdef SIGHUP
  case SIGHUP:
    fprintf(stderr, "caught SIGHUP signal, cleaning up\n");
    break;
#endif

#ifdef SIGSEGV
  case SIGSEGV:
    fprintf(stderr, "caught SIGSEGV signal, cleaning up\n");
    abt = 1;
    break;
#endif

#ifdef SIGBUS
  case SIGBUS:
    fprintf(stderr, "caught SIGBUS signal, cleaning up\n");
    abt = 1;
    break;
#endif

  default:
    fprintf(stderr, "caught signal %d, cleaning up\n", signo);
    abt = 1;
    break;
  }

  fflush(stderr);
  log_close(LOG_MACHINE|LOG_NORMAL|LOG_SKID);
  if (abt) abort();
  exit(1);
}

#ifdef WIN32
#define STAT_READABLE(st) st.st_mode & S_IREAD
#else
#define STAT_READABLE(st) st.st_mode & S_IRUSR
#endif

/* Returns true (nonzero) if the file pathname given exists, is not
 * a directory and is readable by the executing process.  Returns
 * zero if it is not
 */
static int fileexistsandisreadable(char *pathname) {
  struct stat st;

  if (stat(pathname, &st) == -1)
    return 0;

  if (!S_ISDIR(st.st_mode) && STAT_READABLE(st))
    return 1;

  return 0;
}

int nmap_fetchfile(char *filename_returned, int bufferlen, char *file) {
  char *dirptr;
  int res;
  int foundsomething = 0;
  struct passwd *pw;
  char dot_buffer[512];
  static int warningcount = 0;

  /* First we try [--datadir]/file, then $NMAPDIR/file
     next we try ~user/nmap/file
     then we try NMAPDATADIR/file <--NMAPDATADIR 
     finally we try ./file

	 -- or on Windows --

	 --datadir -> $NMAPDIR -> nmap.exe directory -> NMAPDATADIR -> .
  */

  if (o.datadir) {
    res = snprintf(filename_returned, bufferlen, "%s/%s", o.datadir, file);
    if (res > 0 && res < bufferlen) {
      if (fileexistsandisreadable(filename_returned))
	foundsomething = 1;
    }
  }

  if (!foundsomething && (dirptr = getenv("NMAPDIR"))) {
    res = snprintf(filename_returned, bufferlen, "%s/%s", dirptr, file);
    if (res > 0 && res < bufferlen) {
      if (fileexistsandisreadable(filename_returned))
	foundsomething = 1;
    }
  }
#ifndef WIN32
  if (!foundsomething) {
    pw = getpwuid(getuid());
    if (pw) {
      res = snprintf(filename_returned, bufferlen, "%s/.nmap/%s", pw->pw_dir, file);
      if (res > 0 && res < bufferlen) {
	if (fileexistsandisreadable(filename_returned))
	  foundsomething = 1;
      }
    }
    if (!foundsomething && getuid() != geteuid()) {
      pw = getpwuid(geteuid());
      if (pw) {
	res = snprintf(filename_returned, bufferlen, "%s/.nmap/%s", pw->pw_dir, file);
	if (res > 0 && res < bufferlen) {
	  if (fileexistsandisreadable(filename_returned))
	    foundsomething = 1;
	}
      }
    }
  }
#else
  if (!foundsomething) { /* Try the nMap directory */
	  char fnbuf[MAX_PATH];
	  int i;
	  res = GetModuleFileName(GetModuleHandle(0), fnbuf, 1024);
      if(!res) fatal("GetModuleFileName failed (!)\n");
	  /*	Strip it */
	  for(i = res - 1; i >= 0 && fnbuf[i] != '/' && fnbuf[i] != '\\'; i--);
	  if(i >= 0) /* we found it */
		  fnbuf[i] = 0;
	  res = snprintf(filename_returned, bufferlen, "%s/%s", fnbuf, file);
	  if(res > 0 && res < bufferlen) {
		  if (fileexistsandisreadable(filename_returned))
            foundsomething = 1;
      }
  }
#endif
  if (!foundsomething) {
    res = snprintf(filename_returned, bufferlen, "%s/%s", NMAPDATADIR, file);
    if (res > 0 && res < bufferlen) {
      if (fileexistsandisreadable(filename_returned))
	foundsomething = 1;
    }
  }
  if (foundsomething && (*filename_returned != '.')) {    
    res = snprintf(dot_buffer, sizeof(dot_buffer), "./%s", file);
    if (res > 0 && res < bufferlen) {
      if (fileexistsandisreadable(dot_buffer)) {
#ifdef WIN32
	if (warningcount++ < 1 && o.debugging)
#else
	if(warningcount++ < 1)
#endif
	  error("Warning: File %s exists, but Nmap is using %s for security and consistency reasons.  set NMAPDIR=. to give priority to files in your local directory (may affect the other data files too).", dot_buffer, filename_returned);
      }
    }
  }

  if (!foundsomething) {
    res = snprintf(filename_returned, bufferlen, "./%s", file);
    if (res > 0 && res < bufferlen) {
      if (fileexistsandisreadable(filename_returned))
	foundsomething = 1;
    }
  }

  if (!foundsomething) {
    filename_returned[0] = '\0';
    return -1;
  }

  if (o.debugging > 1)
    error("Fetchfile found %s\n", filename_returned);

  return 0;

}


