#include <includes.h>
#include <nessusraw.h>



#undef DEBUG 
#undef SHOW_RETRIES
#undef SHOW_RTT_REMOVAL

#define NUM_RETRIES 2
#define SCAN_FATAL_ERR (void*)(-1)

/*----------------------------------------------------------------------------*/
struct pseudohdr {
	struct in_addr  saddr;
	struct in_addr  daddr;
	u_char          zero;
	u_char          protocol;
	u_short         length;
	struct tcphdr   tcpheader;
};


static int 
in_cksum(p, n)
	u_short        *p;
	int             n;
{
	register u_short answer;
	register unsigned long sum = 0;
	u_short         odd_byte = 0;

	while (n > 1) {
		sum += *p++;
		n -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (n == 1) {
		*(u_char *) (&odd_byte) = *(u_char *) p;
		sum += odd_byte;
	}
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);	/* add carry */
	answer = (int) ~sum;	/* ones-complement, truncate */
	return (answer);
}



unsigned long 
maketime()
{
	struct timeval  tv;
	unsigned long   ret;

	gettimeofday(&tv, NULL);



	ret = ((tv.tv_sec & 0x0000000F) << 28) | (((tv.tv_usec) & 0xFFFFFFF0) >> 4);


	return htonl(ret);
}


struct timeval 
timeval(unsigned long val)
{
	struct timeval  ret;
	unsigned int h, l;

	val = ntohl(val);

	h = ( val & 0xF0000000 ) >> 28;
	l = ( val & 0x0FFFFFFF)  << 4;
     
	ret.tv_sec = h;
	ret.tv_usec = l;
	while ( ret.tv_usec >= 1000000 ) 
	 {
	  ret.tv_usec -= 1000000;
	  ret.tv_sec ++;
	 }

	if ( ret.tv_sec > 2 ) {
	 ret.tv_sec = 2;
	 ret.tv_usec = 0;
	}
	return ret;
}







unsigned long 
compute_rtt(unsigned long then)
{
	unsigned long   now = maketime();
	unsigned long   res;
	unsigned long   a, b;

	a = (unsigned long) ntohl(now);
	b = (unsigned long) ntohl(then);


	if (b > a) {
		return 0;
	}
	res = a - b;
	if ( res >= (1 << 28) ) 
		res = 1 << 28;

        return htonl(res);
}


int 
packetdead(unsigned long then, unsigned long rtt)
{
	unsigned long   now = maketime();

	then = ntohl(then);
	now = ntohl(now);
	rtt = ntohl(rtt);



	if ((now - then) >= 2 << 28 ) {
		return 1;
	} else {
		return 0;
	}
}

int 
rawsocket()
{
	int             soc = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	int             opt = 1;

	if (soc < 0) {
		perror("socket ");
		return -1;
	}
#ifdef IP_HDRINCL
	if (setsockopt(soc, IPPROTO_IP, IP_HDRINCL, (char *) &opt, sizeof(opt)) < 0) {
		perror("setsockopt ");
		close(soc);
		return -1;
	}
#endif

	return soc;
}


int 
openbpf(struct in_addr dst, struct in_addr * src, int magic)
{
	char           *iface;
	char            filter[255];
	int             bpf;

	iface = routethrough(&dst, src);
	snprintf(filter, sizeof(filter), "tcp and src host %s and dst port %d", inet_ntoa(dst), magic);
	bpf = bpf_open_live(iface, filter);
	return bpf;
}

/*----------------------------------------------------------------------------*/

struct list {
	unsigned short  dport;
	unsigned long   when;
	int             retries;
	struct list    *prev;
	struct list    *next;
};

struct list    *
get_packet(struct list * l, unsigned short dport)
{
	if ( l == SCAN_FATAL_ERR ) return NULL;
	while (l != NULL) {
		if (l->dport == dport)
			return l;
		else
			l = l->next;
	}
	return NULL;
}


struct list    *
add_packet(struct list * l, unsigned short dport, unsigned long ack)
{
	struct list    *ret;

	if ( l == SCAN_FATAL_ERR ) return NULL;
	ret = get_packet(l, dport);
	if (ret != NULL) {
#ifdef SHOW_RETRIES
		printf("RETRIES FOR %d = %d\n", dport, ret->retries);
#endif
		ret->retries++;
		ret->when = ack;
		return l;
	}
	ret = emalloc(sizeof(struct list));


	ret->next = l;
	ret->prev = NULL;
	if (ret->next != NULL)
		ret->next->prev = ret;

	ret->dport = dport;
	ret->when = ack;
	ret->retries = 0;
	return ret;
}



struct list    *
rm_packet(struct list * l, unsigned short dport)
{
	struct list    *ret = l;
	struct list    *p = get_packet(l, dport);

	if ( l == SCAN_FATAL_ERR ) return NULL;
	if (p == NULL) {
#if DEBUG > 1
		fprintf(stderr, "Odd - no entry for %d - RTT too low ?!\n", dport);
#endif
		return l;
	}
	if (p->next != NULL)
		p->next->prev = p->prev;

	if (p->prev != NULL)
		p->prev->next = p->next;
	else
		ret = p->next;

	efree(&p);
	return ret;
}

struct list    *
rm_dead_packets(struct list * l, unsigned long rtt, int *retry)
{
	struct list    *ret = l;
	struct list    *p = l;


	if ( l == SCAN_FATAL_ERR ) return NULL;
	*retry = 0;
	while (p != NULL) {
		struct list    *next = p->next;
		if (packetdead(p->when, rtt)) {
			if (p->retries < NUM_RETRIES) {
#ifdef SHOW_RETRIES
				printf("Will retry port %d\n", p->dport);
#endif
				*retry = p->dport;
			} else {
#ifdef SHOW_RTT_REMOVAL
				printf("Removing port %d (RTT elapsed)\n", p->dport);
#endif
				if (p->next != NULL)
					p->next->prev = p->prev;

				if (p->prev != NULL)
					p->prev->next = p->next;
				else
					ret = p->next;
				efree(&p);
			}
		}
		p = next;
	}
	return ret;
}




/*-----------------------------------------------------------------------------*/


struct tcphdr * extracttcp(char * pkt, int len)
{
 struct ip * ip;
	struct tcphdr  *tcp;
	unsigned long   ret;
 
 ip = (struct ip*)pkt;
 if(ip->ip_hl * 4 + sizeof(struct tcphdr) > len)
  return NULL;
  
 tcp = (struct tcphdr*)(pkt + ip->ip_hl * 4);
 return tcp;
}

unsigned long 
extractack(char *pkt, int len)
{
 unsigned long   ret;
 struct tcphdr * tcp = extracttcp(pkt, len);
 
 if( tcp == NULL )
  return -1;
 
 ret = htonl(ntohl(tcp->th_ack) - 1);
	return ret;
}


unsigned short 
extractsport(char *pkt, int len)
{
	struct tcphdr  *tcp = extracttcp(pkt, len);
 unsigned long   ret;
 if(tcp == NULL)return 0;
 
	return ntohs(tcp->th_sport);
}

int 
issynack(char *pkt, int len)
{
	struct tcphdr  *tcp = extracttcp(pkt, len); 
	unsigned long   ret;
 
 if(tcp == NULL)return 0;

	return tcp->th_flags == (TH_SYN | TH_ACK);
}

char           *
mktcp(struct in_addr src, int sport, struct in_addr dst, int dport, unsigned long th_ack, unsigned char flag)
{
	static char     pkt[sizeof(struct ip) + sizeof(struct tcphdr)];
	struct ip      *ip;
	struct tcphdr  *tcp;
	struct pseudohdr pseudohdr;
	char            tcpsumdata[sizeof(pseudohdr)];

	ip = (struct ip *) (&pkt);
	ip->ip_hl = 5;
	ip->ip_v = 4;
	ip->ip_tos = 0;
	ip->ip_len = FIX(sizeof(struct ip) + sizeof(struct tcphdr));
	ip->ip_id = rand();
	ip->ip_off = 0;
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_TCP;
	ip->ip_sum = 0;
	ip->ip_src.s_addr = src.s_addr;
	ip->ip_dst.s_addr = dst.s_addr;
	ip->ip_sum = in_cksum((u_short *) pkt, sizeof(struct ip));

	tcp = (struct tcphdr *) (&(pkt[sizeof(struct ip)]));
	tcp->th_sport = htons(sport);
	tcp->th_dport = htons(dport);
	tcp->th_seq = th_ack;
	tcp->th_ack = 0;
	tcp->th_x2 = 0;
	tcp->th_off = 5;
	tcp->th_flags = flag;
	tcp->th_win = 4096;
	tcp->th_sum = 0;
	tcp->th_urp = 0;

	bzero(&pseudohdr, 12);
	pseudohdr.saddr.s_addr = src.s_addr;
	pseudohdr.daddr.s_addr = dst.s_addr;
	pseudohdr.protocol = IPPROTO_TCP;
	pseudohdr.length = htons(sizeof(struct tcphdr));
	bcopy((char *) tcp, (char *) &pseudohdr.tcpheader, sizeof(struct tcphdr));
	bcopy(&pseudohdr, tcpsumdata, sizeof(struct pseudohdr));
	tcp->th_sum = in_cksum((unsigned short *) tcpsumdata, 12 + sizeof(struct tcphdr));

	return pkt;
}
/*--------------------------------------------------------------------*/

int 
find_rtt(struct in_addr dst, unsigned long *rtt)
{
	int             soc;
	unsigned short  ports[] = {21, 22, 34, 25, 53, 79, 80, 110, 113, 135, 139, 143, 264, 389, 443, 993, 1454, 1723, 3389, 8080, 0};
	unsigned short  use[3];
	int             num = 0;
	int             n;
	int             i;
	int             bpf;
	int             magic = 4441 + (rand() % 1200);
	struct sockaddr_in soca;
	int             len;
	int             skip;
	struct in_addr  src;
	int             j;
	unsigned long   max, max_max;
	int             err = 0;
	int             noresend = 0;


	soc = rawsocket();
	if (soc < 0)
		return -1;

	bpf = openbpf(dst, &src, magic);

	if (bpf < 0) {
		close(soc);
		return -1;
	}
	skip = get_datalink_size(bpf_datalink(bpf));
	bzero(&soca, sizeof(soca));
	soca.sin_family = AF_INET;
	soca.sin_addr = dst;

	for (i = 0; ports[i] != 0; i++) {
		char           *res;
		unsigned long   ack = maketime();
		char           *pkt = mktcp(src, magic, dst, ports[i], ack, TH_SYN);
		int             e;
		struct timeval  tv = {1, 0};
		int             err = 0;
		unsigned short  p = ports[i];

#if DEBUG > 1
		printf("send to port %d\n", ports[i]);
#endif
		errno = 0;
		e = sendto(soc, pkt, sizeof(struct ip) + sizeof(struct tcphdr), 0, (struct sockaddr *) & soca, sizeof(soca));
		if (e < 0) {
			perror("synscan->find_rtt->sendto ");
			close(soc);
			bpf_close(bpf);
			return -1;
		}
		res = (char *) bpf_next_tv(bpf, &len, &tv);
		if (res != NULL) {
#if DEBUG > 1
			printf("Found port %d\n", p);
#endif
			use[num++] = p;
			if (num >= 3)
				break;
		}
	}

	if (num == 0) {
#if DEBUG > 1
		printf("Found nothing\n");
#endif
		bpf_close(bpf);
		close(soc);
		*rtt = htonl(1 << 28);	/* One second */
		return 0;
	}
	max = max_max = 0;

	for (j = 0, n = 0; j < 10; j++, n++) {
		char           *res;
		unsigned long   ack = maketime();
		char           *pkt;
		int             e;
		struct timeval  tv = {1, 0};


		pkt = mktcp(src, magic, dst, use[n % num], ack, TH_SYN);
		e = sendto(soc, pkt, sizeof(struct ip) + sizeof(struct tcphdr), 0, (struct sockaddr *) & soca, sizeof(soca));
		if (e < 0)
			{
			 perror("synscan->find_rtt(2)->sendto ");
			 close(soc);
			 bpf_close(bpf);
			 return -1;
			}
		res = (char *) bpf_next_tv(bpf, &len, &tv);


		if (res != NULL) {
			unsigned long   val = compute_rtt(extractack(res + skip, len));
			noresend = 0;
			if (val && val > max_max) {
				if (max != 0) {
					if (val < max * 2) {
						max = max_max;
						max_max = val;
					}
				} else {
					max = max_max;
					max_max = val;
				}
			}
		} else {
#if DEBUG > 1
			printf("No reply ?!\n");
#endif
			j--;
			err++;
			noresend++;
			if (noresend > 4)
				noresend = 0;
			if (err > 10) {
				*rtt = htonl(1 << 28);
				return 0;
			}
		}
	}

	close(soc);
	bpf_close(bpf);
        if(max == 0)max = htonl(1 << 28);
	*rtt = max;
	return 1;
}


struct list    *
sendpacket(int soc, int bpf, int skip, struct in_addr dst, struct in_addr src, int dport, int magic, struct list * packets, unsigned long * rtt, int sniff, struct arglist * env)
{
	unsigned long   ack = maketime();
	char           *pkt = mktcp(src, magic, dst, dport, ack, TH_SYN);
	int             len;
	char           *res;
	struct sockaddr_in soca;
  	struct timeval rtt_tv = timeval(*rtt);
	static int retry = 0;

	bzero(&soca, sizeof(soca));
	soca.sin_family = AF_INET;
	soca.sin_addr = dst;
	rtt_tv.tv_sec *= 1000;
	rtt_tv.tv_sec /= 8;
	
	rtt_tv.tv_usec += (rtt_tv.tv_sec % 1000) * 1000;
	rtt_tv.tv_sec  /= 1000;
        if ( rtt_tv.tv_sec >= 1 )
		{
		rtt_tv.tv_sec  = 1;
		rtt_tv.tv_usec = 0;
		}

	if (dport != 0) {
		int             e;
		packets = add_packet(packets, dport, ack);

send:
		errno = 0;
		e = sendto(soc, pkt, sizeof(struct ip) + sizeof(struct tcphdr), 0, (struct sockaddr *) & soca, sizeof(soca));
		if (e < 0) {
			if ( errno == ENOBUFS )
			{
			 retry ++;
			 if ( retry < 60 )
				{
				  sleep(1);
				  goto send;
				}
			}
			perror("synscan->sendpacket->sendto ");
			return SCAN_FATAL_ERR;
		}
	}
	if (sniff != 0) {
		struct timeval  t, n;
again:
		res = (char *) bpf_next_tv(bpf, &len, &rtt_tv);
		if (res != NULL) {
			unsigned short  sport = extractsport(res + skip, len);
			int             synack = issynack(res + skip, len);
			unsigned int rack = extractack(res + skip, len);
			if (synack) {
			  char * rst;
#ifdef DEBUG
				printf("=> Port %d is open\n", sport);
#endif
		  	   scanner_add_port(env, sport, "tcp");
			  /* Send a RST to make sure the connection is closed on the remote side */
			  rst = mktcp(src, magic, dst, sport, ack + 1, TH_RST);
			  sendto(soc, rst, sizeof(struct ip) + sizeof(struct tcphdr), 0, (struct sockaddr *) & soca, sizeof(soca));
			
			  /* Adjust the rtt */
			  *rtt = compute_rtt(rack);
			  if ( ntohl(*rtt) >= ( 1 << 28 ) ) *rtt = 1 << 28;

			}
			packets = rm_packet(packets, sport);
			rtt_tv.tv_sec = 0;
			rtt_tv.tv_usec = 0;
			goto again;
		}
	}
	return packets;
}

int 
scan(struct arglist * env, struct in_addr dst, unsigned long rtt)
{
	int             num;
	int             soc = rawsocket();
	int             bpf;
	struct in_addr  src;
	int             magic = 4441 + (rand() % 1200);
	int             skip;
	int             i;
	struct sockaddr_in soca;
	struct list    *packets = NULL;
	struct timeval  rtt_tv = timeval(htonl(ntohl(rtt) / 2));
	struct arglist *globals = arg_get_value(env, "globals");
	struct arglist *hostinfos = arg_get_value(env, "HOSTNAME");
	char           *hname = arg_get_value(hostinfos, "NAME");
	int             retry;
	char           *range = get_preference(env, "port_range");
	unsigned short *ports;

#ifdef DEBUG
	printf("===> port range = %s\n", range);
#endif

	ports = (unsigned short *) getpts(range, &num);

	if (soc < 0)
		return -1;

	bpf = openbpf(dst, &src, magic);
	skip = get_datalink_size(bpf_datalink(bpf));
	bzero(&soca, sizeof(soca));
	soca.sin_family = AF_INET;
	soca.sin_addr = dst;


	for (i = 0; i < num ; i += 2) {
		int  retry = 0;

   	  	if (i % 100 == 0)
                        comm_send_status(globals, hname, "portscan", i, num);

		packets = sendpacket(soc, bpf, skip, dst, src, ports[i], magic, packets, &rtt, 0, env);
		if ( packets == SCAN_FATAL_ERR ) break;
	 	if ( i + 1 < num ) {
			packets = sendpacket(soc, bpf, skip, dst, src, ports[i + 1], magic, packets, &rtt, 1, env);
			if ( packets == SCAN_FATAL_ERR ) break;
			}
	}

#ifdef DEBUG
	printf("Done with the sending\n");
#endif

	while (packets != NULL && packets != SCAN_FATAL_ERR ) {
		i = 0;
		retry = 0;
		packets = rm_dead_packets(packets, rtt, &retry);
		while (retry != 0 && i < 2) {
			packets = sendpacket(soc, bpf, skip, dst, src, retry, magic, packets, &rtt, 0, env);
			if ( packets == SCAN_FATAL_ERR ) break;
			packets = rm_dead_packets(packets, rtt, &retry);
			i++;
		}
		packets = sendpacket(soc, bpf, skip, dst, src, retry, magic, packets, &rtt, 1, env);
	}

	comm_send_status(globals, hname, "portscan", num, num);
#if 0
        plug_set_key(env, "Host/num_ports_scanned", ARG_INT, (void*)num);
#endif
	close(soc);
	bpf_close(bpf);
	if(ports != NULL)efree(&ports);
	if (num >= 65535)
	  plug_set_key(env, "Host/full_scan", ARG_INT, (void*) 1);
	return 0;
}




#define EN_NAME "SYN Scan"
#define EN_DESC "\n\
This plugins performs a supposedly fast SYN port scan\n\
It does so by computing the RTT (round trip time) of the packets\n\
coming back and forth between the nessusd host and the target,\n\
then it uses that to quicky send SYN packets to the remote host\n"


#define COPYRIGHT "Copyright (C) Renaud Deraison <deraison@cvs.nessus.org>"

#define EN_SUMMARY "Performs a TCP SYN scan"
#define EN_FAMILY "Port scanners"

PlugExport int 
plugin_init(struct arglist * desc)
{
	plug_set_id(desc, 11219);
	plug_set_version(desc, "$Revision: 1.22 $");



	plug_set_name(desc, EN_NAME, NULL);
	plug_set_summary(desc, EN_SUMMARY, NULL);
	plug_set_description(desc, EN_DESC, NULL);

	plug_set_copyright(desc, COPYRIGHT, NULL);
	plug_set_category(desc, ACT_SCANNER);
	plug_set_family(desc, EN_FAMILY, NULL);

	plug_set_dep(desc, "ping_host.nasl");
	return (0);
}



PlugExport int 
plugin_run(struct arglist * env)
{
	int             ret = maketime();
	unsigned long   rtt;
	struct in_addr *dst = plug_get_host_ip(env);
	struct timeval  tv;


      if ( islocalhost(dst) ) return 0;

#if 0
	if (find_rtt(*dst, &rtt) < 0) {
		fprintf(stderr, "Something went wrong, bailing out\n");
		return (1);
	}
#endif
	rtt = htonl(1 << 28);

#ifdef DEBUG
	printf("RTT = 0x%.8x\n", ntohl(rtt));
#endif

	tv = timeval(rtt);

#ifdef DEBUG
	printf("That's %d seconds and %d usecs\n", tv.tv_sec, tv.tv_usec);
#endif


	scan(env, *dst, rtt);
	plug_set_key(env, "Host/scanned", ARG_INT, (void *) 1);
	plug_set_key(env, "Host/scanners/synscan", ARG_INT, (void*)1);
	return 0;
}



