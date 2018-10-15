
/***************************************************************************
 * nmap_dns.cc -- Handles parallel reverse DNS resolution for target IPs   *
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

// mass_rdns - Parallel Asynchronous Reverse DNS Resolution
//
// One of Nmap's features is to perform reverse DNS queries
// on large number of IP addresses. Nmap supports 2 different
// methods of accomplishing this:
//
// System Resolver (specified using --system-dns):
// Performs sequential getnameinfo() calls on all the IPs.
// As reliable as your system resolver, almost guaranteed
// to be portable, but intolerably slow for scans of hundreds
// or more because the result from each query needs to be
// received before the next one can be sent.
//
// Mass/Async DNS (default):
// Attempts to resolve host names in parallel using a set
// of DNS servers. DNS servers are found here:
//
//    --dns-servers <serv1[,serv2],...>   (all platforms - overrides everything else)
//
//    /etc/resolv.conf   (only on unix)
//
//    These registry keys:   (only on windows)
//
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\NameServer
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\DhcpNameServer
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\*\NameServer
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\*\DhcpNameServer
//
//
// Also, most systems maintain a file "/etc/hosts" that contains
// IP to hostname mappings. We also try to consult these files. Here
// is where we look for the files:
//
// Unix: /etc/hosts
//
// Windows:
//   for 95/98/Me: WINDOWS_DIR\hosts
//   for NT/2000/XP Pro: WINDOWS_DIR\system32\drivers\etc\hosts
//   for XP Home: WINDOWS_DIR\system32\drivers\etc\hosts
//     --see http://accs-net.com/hosts/how_to_use_hosts.html
//
//
// Created by Doug Hoyte
// doug at hcsw.org
// http://www.hcsw.org


// TODO:
//
// * Tune performance parameters
//
// * Figure out best way to estimate completion time
//   and display it in a ScanProgressMeter 

#ifdef WIN32
#include "nmap_winconfig.h"
#endif

#include <stdlib.h>
#include <limits.h>
#include <list>
#include <vector>

#include "nmap.h"
#include "NmapOps.h"
#include "nmap_dns.h"
#include "nsock.h"
#include "utils.h"
#include "nmap_tty.h"

extern NmapOps o;



//------------------- Performance Parameters ---------------------

// Algorithm:
//
// A batch of num_targets hosts is passed to nmap_mass_rdns():
//   void nmap_mass_rdns(Target **targets, int num_targets)
//
// mass_dns sends out CAPACITY_MIN of these hosts to the DNS
// servers detected, alternating in sequence.

// When a request is fulfilled (either a resolved domain, NXDomain,
// or confirmed ServFail) CAPACITY_UP_STEP is added to the current
// capacity of the server the request was found by.

// When a request times out and retries on the same server,
// the server's capacity is scaled by CAPACITY_MINOR_DOWN_STEP.

// When a request times out and moves to the next server in
// sequence, the server's capacity is scaled by CAPACITY_MAJOR_DOWN_STEP.

// mass_dns tries to maintain the current number of "outstanding
// queries" on each server to that of its current capacity. The
// packet is dropped if it cycles through all specified DNS
// servers.


// Since multiple DNS servers can be specified, different sequences
// of timers are maintained. These are the various retransmission
// intervals for each server before we move on to the next DNS server:

// In milliseconds
// Each row MUST be terminated with -1
static int read_timeouts[][4] = {
  { 4000, 4000, 5000, -1 }, // 1 server
  { 2500, 4000,   -1, -1 }, // 2 servers
  { 2500, 3000,   -1, -1 }, // 3+ servers
};

#define CAPACITY_MIN 10
#define CAPACITY_MAX 200
#define CAPACITY_UP_STEP 2
#define CAPACITY_MINOR_DOWN_SCALE 0.9
#define CAPACITY_MAJOR_DOWN_SCALE 0.7

// Each request will try to resolve on at most this many servers:
#define SERVERS_TO_TRY 3


//------------------- Other Parameters ---------------------

// How often to display a short debugging summary if debugging is
// specified. Lower numbers means it's displayed more often.
#define SUMMARY_DELAY 50

// Minimum debugging level to display packet trace
#define TRACE_DEBUG_LEVEL 4

// The amount of time we wait for nsock_write() to complete before
// retransmission. This should almost never happen. (in milliseconds)
#define WRITE_TIMEOUT 100

// Size of hash table used to hold the hosts from /etc/hosts
#define HASH_TABLE_SIZE 256



//------------------- Internal Structures ---------------------

typedef struct dns_server_s dns_server;
typedef struct request_s request;
typedef struct host_elem_s host_elem;

struct dns_server_s {
  char *hostname;
  sockaddr_in addr;
  nsock_iod nsd;
  int connected;
  int reqs_on_wire;
  int capacity;
  int write_busy;
  std::list<request *> to_process;
  std::list<request *> in_process;
};

struct request_s {
  Target *targ;
  struct timeval timeout;
  int tries;
  int servers_tried;
  dns_server *first_server;
  dns_server *curr_server;
  u16 id;
};

struct host_elem_s {
  char *name;
  u32 addr;
};


//------------------- Globals ---------------------

static std::list<dns_server *> servs;
static std::list<request *> new_reqs;
static std::list<request *> cname_reqs;
static int total_reqs;
static nsock_pool dnspool=NULL;

static std::list<host_elem *> etchosts[HASH_TABLE_SIZE];

static int stat_actual, stat_ok, stat_nx, stat_sf, stat_trans, stat_dropped, stat_cname;
static struct timeval starttv;
static int read_timeout_index;
static u16 id_counter;

static int firstrun=1;
static ScanProgressMeter *SPM;


//------------------- Prototypes and macros ---------------------

static void put_dns_packet_on_wire(request *req);

#define ACTION_FINISHED 0
#define ACTION_CNAME_LIST 1
#define ACTION_TIMEOUT 2



//------------------- Misc code --------------------- 

static void output_summary() {
  int tp = stat_ok + stat_nx + stat_dropped;
  struct timeval now;

  memcpy(&now, nsock_gettimeofday(), sizeof(struct timeval));

  if (o.debugging && (tp%SUMMARY_DELAY == 0))
    log_write(LOG_STDOUT, "mass_rdns: %.2fs %d/%d [#: %lu, OK: %d, NX: %d, DR: %d, SF: %d, TR: %d]\n",
                    TIMEVAL_MSEC_SUBTRACT(now, starttv) / 1000.0,
                    tp, stat_actual,
                    (unsigned long) servs.size(), stat_ok, stat_nx, stat_dropped, stat_sf, stat_trans);
}

static void check_capacities(dns_server *tpserv) {
  if (tpserv->capacity < CAPACITY_MIN) tpserv->capacity = CAPACITY_MIN;
  if (tpserv->capacity > CAPACITY_MAX) tpserv->capacity = CAPACITY_MAX;
  if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "CAPACITY <%s> = %d\n", tpserv->hostname, tpserv->capacity);
}

// Closes all nsis created in connect_dns_servers()
static void close_dns_servers() {
  std::list<dns_server *>::iterator serverI;

  for(serverI = servs.begin(); serverI != servs.end(); serverI++) {
    if ((*serverI)->connected) {
      nsi_delete((*serverI)->nsd, NSOCK_PENDING_SILENT);
      (*serverI)->connected = 0;
      (*serverI)->to_process.clear();
      (*serverI)->in_process.clear();
    }
  }
}


// Inserts an integer (endian non-specifically) into a DNS packet.
// Returns number of bytes written
static int add_integer_to_dns_packet(char *packet, int c) {
  char tpnum[4];
  int tplen;

  sprintf(tpnum, "%d", c);
  tplen = strlen(tpnum);
  packet[0] = (char) tplen;
  memcpy(packet+1, tpnum, tplen);

  return tplen+1;
}

// Puts as many packets on the line as capacity will allow
static void do_possible_writes() {
  std::list<dns_server *>::iterator servI;
  dns_server *tpserv;
  request *tpreq;

  for(servI = servs.begin(); servI != servs.end(); servI++) {
    tpserv = *servI;

    if (tpserv->write_busy == 0 && tpserv->reqs_on_wire < tpserv->capacity) {
      tpreq = NULL;
      if (!tpserv->to_process.empty()) {
        tpreq = tpserv->to_process.front();
        tpserv->to_process.pop_front();
      } else if (!new_reqs.empty()) {
        tpreq = new_reqs.front();
        tpreq->first_server = tpreq->curr_server = tpserv;
        new_reqs.pop_front();
      }

      if (tpreq) {
        if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: TRANSMITTING for <%s> (server <%s>)\n", tpreq->targ->targetipstr() , tpserv->hostname);
        stat_trans++;
        put_dns_packet_on_wire(tpreq);
      }
    }
  }
}

// nsock write handler
static void write_evt_handler(nsock_pool nsp, nsock_event evt, void *req_v) {
  request *req = (request *) req_v;

  req->curr_server->write_busy = 0;
  req->curr_server->in_process.push_front(req);

  do_possible_writes();
}

// Takes a DNS request structure and actually puts it on the wire
// (calls nsock_write()). Does various other tasks like recording
// the time for the timeout.
static void put_dns_packet_on_wire(request *req) {
  char packet[512];
  int plen=0;
  u32 ip;
  struct timeval now, timeout;

  ip = (u32) ntohl(req->targ->v4host().s_addr);

  packet[0] = (req->id >> 8) & 0xFF;
  packet[1] = req->id & 0xFF;
  plen += 2;

  memcpy(packet+plen, "\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00", 10);
  plen += 10;

  plen += add_integer_to_dns_packet(packet+plen, ip & 0xFF);
  plen += add_integer_to_dns_packet(packet+plen, (ip>>8) & 0xFF);
  plen += add_integer_to_dns_packet(packet+plen, (ip>>16) & 0xFF);
  plen += add_integer_to_dns_packet(packet+plen, (ip>>24) & 0xFF);

  memcpy(packet+plen, "\x07in-addr\004arpa\x00\x00\x0c\x00\x01", 18);
  plen += 18;

  req->curr_server->write_busy = 1;
  req->curr_server->reqs_on_wire++;

  memcpy(&now, nsock_gettimeofday(), sizeof(struct timeval));
  TIMEVAL_MSEC_ADD(timeout, now, read_timeouts[read_timeout_index][req->tries]);
  memcpy(&req->timeout, &timeout, sizeof(struct timeval));

  req->tries++;

  nsock_write(dnspool, req->curr_server->nsd, write_evt_handler, WRITE_TIMEOUT, req, packet, plen);
}

// Processes DNS packets that have timed out
// Returns time until next read timeout
static int deal_with_timedout_reads() {
  std::list<dns_server *>::iterator servI;
  std::list<dns_server *>::iterator servItemp;
  std::list<request *>::iterator reqI;
  std::list<request *>::iterator nextI;
  dns_server *tpserv;
  request *tpreq;
  struct timeval now;
  int tp, min_timeout = INT_MAX;

  memcpy(&now, nsock_gettimeofday(), sizeof(struct timeval));

  if (keyWasPressed())
    SPM->printStats((double) (stat_ok + stat_nx + stat_dropped) / stat_actual, &now);

  for(servI = servs.begin(); servI != servs.end(); servI++) {
    tpserv = *servI;

    nextI = tpserv->in_process.begin();
    if (nextI == tpserv->in_process.end()) continue;

    do {
      reqI = nextI++;
      tpreq = *reqI;

      tp = TIMEVAL_MSEC_SUBTRACT(tpreq->timeout, now);
      if (tp > 0 && tp < min_timeout) min_timeout = tp;

      if (tp <= 0) {
        tpserv->capacity = (int) (tpserv->capacity * CAPACITY_MINOR_DOWN_SCALE);
        check_capacities(tpserv);
        tpserv->in_process.erase(reqI);
        tpserv->reqs_on_wire--;

        // If we've tried this server enough times, move to the next one
        if (read_timeouts[read_timeout_index][tpreq->tries] == -1) {
          tpserv->capacity = (int) (tpserv->capacity * CAPACITY_MAJOR_DOWN_SCALE);
          check_capacities(tpserv);

          servItemp = servI;
          servItemp++;

          if (servItemp == servs.end()) servItemp = servs.begin();

          tpreq->curr_server = *servItemp;
          tpreq->tries = 0;
          tpreq->servers_tried++;

          if (tpreq->curr_server == tpreq->first_server || tpreq->servers_tried == SERVERS_TO_TRY) {
            // Either give up on the IP
            // or, for maximum reliability, put the server back into processing
            // Note it's possible that this will never terminate.
            // FIXME: Find a good compromise

            // **** We've already tried all servers... give up
            if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: *DR*OPPING <%s>\n", tpreq->targ->targetipstr());

            output_summary();
            stat_dropped++;
            total_reqs--;
            delete tpreq;

            // **** OR We start at the back of this server's queue
            //(*servItemp)->to_process.push_back(tpreq);
          } else {
            (*servItemp)->to_process.push_back(tpreq);
          }
        } else {
          tpserv->to_process.push_back(tpreq);
        }

    }

    } while (nextI != tpserv->in_process.end());

  }

  if (min_timeout > 500) return 500;
  else return min_timeout;

}

// After processing a DNS response, we search through the IPs we're
// looking for and update their results as necessary.
// Returns non-zero if this matches a query we're looking for
static int process_result(u32 ia, char *result, int action, u16 id) {
  std::list<dns_server *>::iterator servI;
  std::list<request *>::iterator reqI;
  dns_server *tpserv;
  request *tpreq;

  for(servI = servs.begin(); servI != servs.end(); servI++) {
    tpserv = *servI;

    for(reqI = tpserv->in_process.begin(); reqI != tpserv->in_process.end(); reqI++) {
      tpreq = *reqI;

      if (id == tpreq->id) {

        if (ia != 0 && (u32) (tpreq->targ->v4host().s_addr) != ia) continue;

        if (action == ACTION_CNAME_LIST || action == ACTION_FINISHED) {
        tpserv->capacity += CAPACITY_UP_STEP;
        check_capacities(tpserv);

        if (result) tpreq->targ->setHostName(result);
        tpserv->in_process.remove(tpreq);
        tpserv->reqs_on_wire--;

        total_reqs--;

          if (action == ACTION_CNAME_LIST) cname_reqs.push_back(tpreq);
          if (action == ACTION_FINISHED) delete tpreq;
        } else {
          memcpy(&tpreq->timeout, nsock_gettimeofday(), sizeof(struct timeval));
          deal_with_timedout_reads();
        }

        do_possible_writes();

        // Close DNS servers if we're all done so that we kill
        // all events and return from nsock_loop immediatley
        if (total_reqs == 0)
          close_dns_servers();
        return 1;
      }
    }
  }

  return 0;
}


// Gets an IP address from a X.X.X.X.in-addr.arpa DNS
// encoded string inside a packet.
// maxlen is the very maximum length (in total bytes)
// that should be processed
static u32 parse_inaddr_arpa(unsigned char *buf, int maxlen) {
  u32 ip=0;
  int i, j;

  if (maxlen <= 0) return 0;

  for (i=0; i<=3; i++) {
    if (buf[0] < 1 || buf[0] > 3) return 0;

    maxlen -= buf[0] + 1;
    if (maxlen <= 0) return 0;

    for (j=1; j<=buf[0]; j++) if (!isdigit(buf[j])) return 0;

    ip |= atoi((char *) buf+1) << (8*i);
    buf += buf[0] + 1;
  }

  if (maxlen < 14) return 0; // length of the following string
  if (strcasecmp((char *) buf, "\x07in-addr\004arpa\0")) return 0;

  return ntohl(ip);
}


// Turns a DNS packet encoded name (see the RFC) and turns it into
// a normal decimal separated hostname.
// ASSUMES NAME LENGTH/VALIDITY HAS ALREADY BEEN VERIFIED
static int encoded_name_to_normal(unsigned char *buf, char *output, int outputsize){
  while (buf[0]) {
    if (buf[0] >= outputsize-1) return -1;
    memcpy(output, buf+1, buf[0]);
    outputsize -= buf[0];
    output += buf[0];
    buf += buf[0]+1;

    if (buf[0]) {
      *output++ = '.';
      outputsize--;
    } else {
      *output = '\0';
    }
  }

  return 0;
}


// Takes a pointer to the start of a DNS name inside a packet. It makes
// sure that there is enough space in the name, deals with compression, etc.
static int advance_past_dns_name(u8 *buf, int buflen, int curbuf, 
			  int *nameloc) {
  int compression=0;

  if (curbuf <= 0 || curbuf >= buflen) return -1;

  if ((buf[curbuf] & 0xc0)) {
    // Need 2 bytes for compression info
    if (curbuf + 1 >= buflen) return -1;

    // Compression is OK
    compression = curbuf+2;
    curbuf = (buf[curbuf+1] + (buf[curbuf] << 8)) & 0x3FFF;
    if (curbuf < 0 || curbuf >= buflen) return -1;
  }

  if (nameloc != NULL) *nameloc = curbuf;

  while(buf[curbuf]) {
    if (curbuf + buf[curbuf] >= buflen || buf[curbuf] <= 0) return -1;
    curbuf += buf[curbuf] + 1;
  }

  if (compression) return compression;
  else return curbuf+1;
}

// Nsock read handler. One nsock read for each DNS server exists at each
// time. This function uses various helper functions as defined above.
static void read_evt_handler(nsock_pool nsp, nsock_event evt, void *nothing) {
  u8 *buf;
  int buflen, curbuf=0;
  int i, nameloc, rdlen, atype, aclass;
  int errcode=0;
  int queries, answers;
  u16 packet_id;

  if (total_reqs >= 1)
    nsock_read(nsp, nse_iod(evt), read_evt_handler, -1, NULL);

  if (nse_type(evt) != NSE_TYPE_READ || nse_status(evt) != NSE_STATUS_SUCCESS) {
    if (o.debugging)
      log_write(LOG_STDOUT, "mass_dns: warning: got a %s:%s in read_evt_handler()\n",
        nse_type2str(nse_type(evt)),
        nse_status2str(nse_status(evt)));
    return;
  }

  buf = (unsigned char *) nse_readbuf(evt, &buflen);

  // Size of header is 12, and we must have additional data as well
  if (buflen <= 12) return;

  packet_id = buf[1] + (buf[0] << 8);

  // Check that this is a response, standard query, and that no truncation was performed
  // 0xFA == 11111010 (we're not concerned with AA or RD bits)
  if ((buf[2] & 0xFA) != 0x80) return;

  // Check that the zero field is all zeros and there is no error condition.
  // We don't care if recursion is available or not since we might be querying
  // an authoritative DNS server.
  if (buf[3] != 0x80 && buf[3] != 0) {
    if ((buf[3] & 0xF) == 2) errcode = 2;
    else if ((buf[3] & 0xF) == 3) errcode = 3;
    else return;
  }

  queries = buf[5] + (buf[4] << 8);
  answers = buf[7] + (buf[6] << 8);

  // With a normal resolution, we should have 1+ queries and 1+ answers.
  // If the domain doesn't resolve (NXDOMAIN or SERVFAIL) we should have
  // 1+ queries and 0 answers:
  if (errcode) {
    int found;

    // NXDomain means we're finished (doesn't exist for sure)
    // but SERVFAIL might just mean a server timeout
    found = process_result(0, NULL, errcode == 3 ? ACTION_FINISHED : ACTION_TIMEOUT, packet_id);

    if (errcode == 2 && found) {
      if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: SERVFAIL <id = %d>\n", packet_id);
      stat_sf++;
    } else if (errcode == 3 && found) {
      if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: NXDOMAIN <id = %d>\n", packet_id);
      output_summary();
      stat_nx++;
  }

    return;
  }

  if (queries <= 0 || answers <= 0) return;

  curbuf = 12;

  // Need to safely skip past QUERY section

  for (i=0; i<queries; i++) {
    curbuf = advance_past_dns_name(buf, buflen, curbuf, &nameloc);
    if (curbuf == -1) return;

    // Make sure we have the QTYPE and QCLASS fields
    if (curbuf + 4 >= buflen) return;
    curbuf += 4;
      }

  // We're now at the ANSWER section

  for (i=0; i<answers; i++) {
    curbuf = advance_past_dns_name(buf, buflen, curbuf, &nameloc);
    if (curbuf == -1) return;

    // Make sure we have the TYPE (2), CLASS (2), TTL (4), and
    // RDLENGTH (2) fields
    if (curbuf + 10 >= buflen) return;

    atype = buf[curbuf+1] + (buf[curbuf+0] << 8);
    aclass = buf[curbuf+3] + (buf[curbuf+2] << 8);
    rdlen = buf[curbuf+9] + (buf[curbuf+8] << 8);
    curbuf += 10;

    if (atype == 12 && aclass == 1) {
      // TYPE 12 is PTR
      struct in_addr ia;
      char outbuf[512];

      ia.s_addr = parse_inaddr_arpa(buf+nameloc, buflen-nameloc);
      if (ia.s_addr == 0) return;

      curbuf = advance_past_dns_name(buf, buflen, curbuf, &nameloc);
      if (curbuf == -1 || curbuf > buflen) return;

      if (encoded_name_to_normal(buf+nameloc, outbuf, sizeof(outbuf)) == -1) return;

      if (process_result(ia.s_addr, outbuf, ACTION_FINISHED, packet_id)) {
        if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: OK MATCHED <%s> to <%s>\n", inet_ntoa(ia), outbuf);
        output_summary();
        stat_ok++;
      }
    } else if (atype == 5 && aclass == 1) {
      // TYPE 5 is CNAME
      struct in_addr ia;

      ia.s_addr = parse_inaddr_arpa(buf+nameloc, buflen-nameloc);
      if (ia.s_addr == 0) return;

      if (o.debugging >= TRACE_DEBUG_LEVEL) log_write(LOG_STDOUT, "mass_rdns: CNAME found for <%s>\n", inet_ntoa(ia));
      process_result(ia.s_addr, NULL, ACTION_CNAME_LIST, packet_id);
    } else {
      if (rdlen < 0 || rdlen + curbuf >= buflen) return;
      curbuf += rdlen;
    }

    if (curbuf >= buflen) return;
  }

}


// nsock connect handler - Empty because it doesn't really need to do anything...
static void connect_evt_handler(nsock_pool nsp, nsock_event evt, void *servers) {
}


// Adds DNS servers to the dns_server list. They can be separated by
// commas or spaces - NOTE this doesn't actually do any connecting!
static void add_dns_server(char *ipaddrs) {
  std::list<dns_server *>::iterator servI;
  dns_server *tpserv;
  char *hostname;
  struct sockaddr_in addr;
  size_t addr_len = sizeof(addr);

  for (hostname = strtok(ipaddrs, " ,"); hostname != NULL; hostname = strtok(NULL, " ,")) {

    if (!resolve(hostname, (struct sockaddr_storage *) &addr, &addr_len, PF_INET)) continue;

    for(servI = servs.begin(); servI != servs.end(); servI++) {
      tpserv = *servI;

      // Already added!
      if (memcmp(&addr, &tpserv->addr, sizeof(addr)) == 0) break;
    }

    // If it hasn't already been added, add it!
    if (servI == servs.end()) {
      tpserv = new dns_server;

      tpserv->hostname = strdup(hostname);
      memcpy(&tpserv->addr, &addr, sizeof(addr));

      servs.push_front(tpserv);

      if (o.debugging) log_write(LOG_STDOUT, "mass_rdns: Using DNS server %s\n", hostname);
    }

  }

}

void free_dns_servers() {
  std::list<dns_server *>::iterator servI;
  dns_server *tpserv;

  for(servI = servs.begin(); servI != servs.end();servI++){
    tpserv = *servI;
    if(tpserv){
      if(tpserv->hostname)
        free(tpserv->hostname);
      delete tpserv;
    }
  }
  servs.clear();
}


// Creates a new nsi for each DNS server
static void connect_dns_servers() {
  std::list<dns_server *>::iterator serverI;
  dns_server *s;

  for(serverI = servs.begin(); serverI != servs.end(); serverI++) {
    s = *serverI;

    s->nsd = nsi_new(dnspool, NULL);
    s->reqs_on_wire = 0;
    s->capacity = CAPACITY_MIN;
    s->write_busy = 0;

    nsock_connect_udp(dnspool, s->nsd, connect_evt_handler, NULL, (struct sockaddr *) &s->addr, sizeof(struct sockaddr), 53);
    nsock_read(dnspool, s->nsd, read_evt_handler, -1, NULL);
    s->connected = 1;
  }

}


#ifdef WIN32
void win32_read_registry(char *controlset) {
  HKEY hKey;
  HKEY hKey2;
  char keybasebuf[2048];
  char buf[2048], keyname[2048], *p;
  DWORD sz, i;

  snprintf(keybasebuf, sizeof(keybasebuf), "SYSTEM\\%s\\Services\\Tcpip\\Parameters", controlset);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keybasebuf,
                    0, KEY_READ, &hKey) != ERROR_SUCCESS) {
    if (firstrun) error("mass_dns: warning: Error opening registry to read DNS servers. Try using --system-dns or specify valid servers with --dns-servers");
    return;
  }

  sz = sizeof(buf);
  if (RegQueryValueEx(hKey, "NameServer", NULL, NULL, (LPBYTE) buf, (LPDWORD) &sz) == ERROR_SUCCESS)
    add_dns_server(buf);

  sz = sizeof(buf);
  if (RegQueryValueEx(hKey, "DhcpNameServer", NULL, NULL, (LPBYTE) buf, (LPDWORD) &sz) == ERROR_SUCCESS)
    add_dns_server(buf);

  RegCloseKey(hKey);

  snprintf(keybasebuf, sizeof(keybasebuf), "SYSTEM\\%s\\Services\\Tcpip\\Parameters\\Interfaces", controlset);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keybasebuf,
                    0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS) {

    sz = sizeof(buf);
    for (i=0; RegEnumKeyEx(hKey, i, buf, &sz, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS; i++) {

      snprintf(keyname, sizeof(keyname), "SYSTEM\\%s\\Services\\Tcpip\\Parameters\\Interfaces\\%s", controlset, buf);

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname,
                        0, KEY_READ, &hKey2) == ERROR_SUCCESS) {

        sz = sizeof(buf);
        if (RegQueryValueEx(hKey2, "DhcpNameServer", NULL, NULL, (LPBYTE) buf, (LPDWORD) &sz) == ERROR_SUCCESS)
          add_dns_server(buf);

        sz = sizeof(buf);
        if (RegQueryValueEx(hKey2, "NameServer", NULL, NULL, (LPBYTE) buf, (LPDWORD) &sz) == ERROR_SUCCESS)
          add_dns_server(buf);

        RegCloseKey(hKey2);
      }

      sz = sizeof(buf);
    }

    RegCloseKey(hKey);

  }

}
#endif



// Parses /etc/resolv.conf (unix) or the registry (win32) and adds
// all the nameservers found via the add_dns_server() function.
static void parse_resolvdotconf() {

#ifndef WIN32

  FILE *fp;
  char buf[2048], *tp;
  char ipaddr[16];

  fp = fopen("/etc/resolv.conf", "r");
  if (fp == NULL) {
    if (firstrun) error("mass_dns: warning: Unable to open /etc/resolv.conf. Try using --system-dns or specify valid servers with --dns-servers");
    return;
  }

  while (fgets(buf, sizeof(buf), fp)) {
    tp = buf;

    // Clip off comments #, \r, \n
    while (*tp != '\r' && *tp != '\n' && *tp != '#' && *tp) tp++;
    *tp = '\0';

    tp = buf;
    // Skip any leading whitespace
    while (*tp == ' ' || *tp == '\t') tp++;

    if (sscanf(tp, "nameserver %15s", ipaddr) == 1) add_dns_server(ipaddr);
  }

  fclose(fp);

#else
  win32_read_registry("CurrentControlSet");
#endif
}


static void parse_etchosts(char *fname) {
  FILE *fp;
  char buf[2048], hname[256], ipaddrstr[16], *tp;
  struct in_addr ia;
  host_elem *he;

  fp = fopen(fname, "r");
  if (fp == NULL) return; // silently is OK

  while (fgets(buf, sizeof(buf), fp)) {
    tp = buf;

    // Clip off comments #, \r, \n
    while (*tp != '\r' && *tp != '\n' && *tp != '#' && *tp) tp++;
    *tp = '\0';

    tp = buf;
    // Skip any leading whitespace
    while (*tp == ' ' || *tp == '\t') tp++;

    if (sscanf(tp, "%15s %255s", ipaddrstr, hname) == 2) {
      if (inet_pton(AF_INET, ipaddrstr, &ia)) {
        he = new host_elem;
        he->name = strdup(hname);
        he->addr = (u32) ia.s_addr;
        etchosts[he->addr % HASH_TABLE_SIZE].push_front(he);
      }
    }
  }

  fclose(fp);
}

void free_etchosts() {
  host_elem *he;
  std::list<host_elem *>::iterator hi;
  int i;

  for(i=0; i < HASH_TABLE_SIZE; i++){
    for(hi = etchosts[i].begin(); hi != etchosts[i].end(); hi++) {
      he = *hi;
      if(he) {
        free(he->name);
        delete he;
      }
    }
    etchosts[i].clear();
  }
}


static char *lookup_etchosts(u32 ip) {
  std::list<host_elem *>::iterator hostI;
  host_elem *tpelem;

  for(hostI = etchosts[ip % HASH_TABLE_SIZE].begin(); hostI != etchosts[ip % HASH_TABLE_SIZE].end(); hostI++) {
    tpelem = *hostI;
    if (tpelem->addr == ip) return tpelem->name;
  }

  return NULL;
}


static void etchosts_init(void) {
  static int initialized = 0;
  if (initialized) return;
  initialized = 1;

#ifdef WIN32
  char windows_dir[1024];
  char tpbuf[2048];
  int has_backslash;

  if (!GetWindowsDirectory(windows_dir, sizeof(windows_dir)))
    fatal("Failed to determine your windows directory");

  // If it has a backslash it's C:\, otherwise something like C:\WINNT
  has_backslash = (windows_dir[strlen(windows_dir)-1] == '\\');

  // Windows 95/98/Me:
  snprintf(tpbuf, sizeof(tpbuf), "%s%shosts", windows_dir, has_backslash ? "" : "\\");
  parse_etchosts(tpbuf);

  // Windows NT/2000/XP/2K3:
  snprintf(tpbuf, sizeof(tpbuf), "%s%ssystem32\\drivers\\etc\\hosts", windows_dir, has_backslash ? "" : "\\");
  parse_etchosts(tpbuf);

#else
  parse_etchosts("/etc/hosts");
#endif
}


//------------------- Main loops ---------------------


// Actual main loop
static void nmap_mass_rdns_core(Target **targets, int num_targets) {

  Target **hostI;
  std::list<request *>::iterator reqI;
  request *tpreq;
  int timeout;
  char *tpname;
  int i;
  bool lasttrace = false;
  char spmobuf[1024];

  if (o.mass_dns == false) {
    Target *currenths;
    struct sockaddr_storage ss;
    size_t sslen;
    char hostname[MAXHOSTNAMELEN + 1] = "";

    for(hostI = targets; hostI < targets+num_targets; hostI++) {
      currenths = *hostI;

      if (((currenths->flags & HOST_UP) || o.resolve_all) && !o.noresolve) stat_actual++;
    }

    snprintf(spmobuf, sizeof(spmobuf), "System DNS resolution of %d host%s.", num_targets, num_targets-1 ? "s" : "");
    SPM = new ScanProgressMeter(spmobuf);

    for(i=0, hostI = targets; hostI < targets+num_targets; hostI++, i++) {
      currenths = *hostI;
	
      if (keyWasPressed())
        SPM->printStats((double) i / stat_actual, NULL);

      if (((currenths->flags & HOST_UP) || o.resolve_all) && !o.noresolve) {
        if (currenths->TargetSockAddr(&ss, &sslen) != 0)
          fatal("Failed to get target socket address.");
        if (getnameinfo((struct sockaddr *)&ss, sslen, hostname,
                        sizeof(hostname), NULL, 0, NI_NAMEREQD) == 0) {
          stat_ok++;
          currenths->setHostName(hostname);
        }
      }
    }

    SPM->endTask(NULL, NULL);
    delete SPM;

    return;
  }

  // If necessary, set up the dns server list from resolv.conf
  if (servs.size() == 0) {
    if (o.dns_servers) add_dns_server(o.dns_servers);
    else parse_resolvdotconf();

    if (servs.size() == 0 && firstrun) error("mass_dns: warning: Unable to determine any DNS servers. Reverse DNS is disabled. Try using --system-dns or specify valid servers with --dns_servers");
  }


  // If necessary, set up the /etc/hosts hashtable
  etchosts_init();


  total_reqs = 0;
  id_counter = get_random_u16();

  // Set up the request structure
  for(hostI = targets; hostI < targets+num_targets; hostI++) {
    if (!((*hostI)->flags & HOST_UP) && !o.resolve_all) continue;

    // See if it's in /etc/hosts
    tpname = lookup_etchosts((u32) (*hostI)->v4hostip()->s_addr);
    if (tpname) {
      (*hostI)->setHostName(tpname);
      continue;
    }

    tpreq = new request;
    tpreq->targ = *hostI;
    tpreq->tries = 0;
    tpreq->servers_tried = 0;
    tpreq->id = id_counter++;

    new_reqs.push_back(tpreq);

    stat_actual++;
    total_reqs++;
  }

  if (total_reqs == 0 || servs.size() == 0) return;

  // And finally, do it!

  if ((dnspool = nsp_new(NULL)) == NULL)
    fatal("Unable to create nsock pool in nmap_mass_rdns_core()");

  if ((lasttrace = o.packetTrace()))
    nsp_settrace(dnspool, 5, o.getStartTime());
  
  connect_dns_servers();

  cname_reqs.clear();

  read_timeout_index = MIN(sizeof(read_timeouts)/sizeof(read_timeouts[0]), servs.size()) - 1;

  snprintf(spmobuf, sizeof(spmobuf), "Parallel DNS resolution of %d host%s.", num_targets, num_targets-1 ? "s" : "");
  SPM = new ScanProgressMeter(spmobuf);

  while (total_reqs > 0) {
    timeout = deal_with_timedout_reads();

    do_possible_writes();

    if (total_reqs <= 0) break;

    /* Because this can change with runtime interaction */
    if (o.packetTrace() != lasttrace) {
      lasttrace = !lasttrace;
      if (lasttrace)
	nsp_settrace(dnspool, 5, o.getStartTime());
      else nsp_settrace(dnspool, 0, o.getStartTime());
    }
    nsock_loop(dnspool, timeout);
  }

  SPM->endTask(NULL, NULL);
  delete SPM;

  close_dns_servers();

  nsp_delete(dnspool);

  if (cname_reqs.size() && o.debugging)
    log_write(LOG_STDOUT, "Performing system-dns for %d domain names that use CNAMEs\n", (int) cname_reqs.size());

  if (cname_reqs.size()) {
    snprintf(spmobuf, sizeof(spmobuf), "System CNAME DNS resolution of %u host%s.", (unsigned) cname_reqs.size(), cname_reqs.size()-1 ? "s" : "");
    SPM = new ScanProgressMeter(spmobuf);

    for(i=0, reqI = cname_reqs.begin(); reqI != cname_reqs.end(); reqI++, i++) {
      struct sockaddr_storage ss;
      size_t sslen;
      char hostname[MAXHOSTNAMELEN + 1] = "";

      if (keyWasPressed())
        SPM->printStats((double) i / cname_reqs.size(), NULL);

      tpreq = *reqI;

      if (tpreq->targ->TargetSockAddr(&ss, &sslen) != 0)
        fatal("Failed to get target socket address.");

      if (getnameinfo((struct sockaddr *)&ss, sslen, hostname,
                      sizeof(hostname), NULL, 0, NI_NAMEREQD) == 0) {
        stat_ok++;
        stat_cname++;
        tpreq->targ->setHostName(hostname);
      }

      delete tpreq;

    }

    SPM->endTask(NULL, NULL);
    delete SPM;
  }

  cname_reqs.clear();

}



// Publicly available function. Basically just a wrapper so we
// can record time information, restart statistics, etc.
void nmap_mass_rdns(Target **targets, int num_targets) {

  struct timeval now;

  gettimeofday(&starttv, NULL);

  stat_actual = stat_ok = stat_nx = stat_sf = stat_trans = stat_dropped = stat_cname = 0;

  nmap_mass_rdns_core(targets, num_targets);

  gettimeofday(&now, NULL);

  if (stat_actual > 0) {
    if (o.debugging || o.verbose >= 3) {
      if (o.mass_dns) {
	// #:  Number of DNS servers used
	// OK: Number of fully reverse resolved queries
	// NX: Number of confirmations of 'No such reverse domain eXists'
	// DR: Dropped IPs (no valid responses were received)
	// SF: Number of IPs that got 'Server Failure's
	// TR: Total number of transmissions necessary. The number of domains is ideal, higher is worse
	log_write(LOG_STDOUT, "DNS resolution of %d IPs took %.2fs. Mode: Async [#: %lu, OK: %d, NX: %d, DR: %d, SF: %d, TR: %d, CN: %d]\n",
		  stat_actual, TIMEVAL_MSEC_SUBTRACT(now, starttv) / 1000.0,
		  (unsigned long) servs.size(), stat_ok, stat_nx, stat_dropped, stat_sf, stat_trans, stat_cname);
      } else {
	log_write(LOG_STDOUT, "DNS resolution of %d IPs took %.2fs. Mode: System [OK: %d, ??: %d]\n",
		  stat_actual, TIMEVAL_MSEC_SUBTRACT(now, starttv) / 1000.0,
		  stat_ok, stat_actual - stat_ok);
      }
    }
  }

  firstrun=0;
}
