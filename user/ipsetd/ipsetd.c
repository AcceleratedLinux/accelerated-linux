/*
 * ipsetd.c -- Daemon for setting IP address with ARP+Ping in user-space
 *
 * Copyright (C) 2001  Axis Communications AB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: ipsetd.c,v 1.3 2008-07-21 10:15:31 davidm Exp $
 */

/*
 * Short description:
 * -c	       Change IP address immediately when IP-set ping arrives.
 *
 * -e CMDLINE  Execute CMDLINE when IP-set ping arrives, the new IP address
 *	       is available from the environment variable $NEW_IPADDR.
 *
 * -l BYTES    Specifies valid length of ICMP payload.
 *	       ICMP packets with a different payload will not affect the
 *	       the IP adress setting.
 *
 * -t SECONDS  Specifies how long it will take until the ipsetd exits.
 *
 * -n	       Do not become a daemon.
 */

#include <stdio.h>
#include <syslog.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/types.h>
#include <linux/filter.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#ifndef __UCLIBC__
#include <netpacket/packet.h>
#else
#include <linux/if_packet.h>
#endif

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

/*
 * The reported received ICMP packet data size includes the IP header
 * (normally 20 bytes) and ICMP header (8 bytes).
 */
#define IP_ICMP_HEADERS_SIZE	 (20 + 8)
#define MIN_ICMP_PAYLOAD_SIZE	 (ICMP_MINLEN + 4)
#define MAX_ICMP_PAYLOAD_SIZE	 (ETH_DATA_LEN - IP_ICMP_HEADERS_SIZE)
#define IGNORE_PING_PAYLOAD_SIZE -1

#define IFACE_DEFAULT	      "eth0"

void usage(FILE *where, char *pgm)
{
	fprintf(where,
		"Usage: %s [-c] [-e command] [-t secs] "
		"[-l bytes] [-i interface] [-n] [-h]\n",
		pgm);
}

void ipsetd_timeout(int arg)
{
	syslog(LOG_INFO, "Timed out.");
	exit(EXIT_SUCCESS);
}

void set_ip(int sockfd, unsigned int ipaddr, char *ifr_name_str)
{
	int r;

	struct sockaddr_in sa;
	struct ifreq ifr;

	sa.sin_family = AF_INET;
	sa.sin_port = 0;
	sa.sin_addr.s_addr = ipaddr;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifr_name_str, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	memcpy((char *) &ifr.ifr_addr,
	       (char *) &sa, sizeof(struct sockaddr));

	ifr.ifr_addr.sa_family = AF_INET;

	r = ioctl(sockfd, SIOCSIFADDR, &ifr);
	if (r < 0) {
		perror("set_ip: ioctl");
		syslog(LOG_ERR, "ioctl: %s.", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int is_local_ip(int sockfd, in_addr_t new_addr)
{
	struct ifconf ifc;
	struct ifreq ifreqs[10];
	int i;

	memset(ifreqs, 0, sizeof ifreqs);
	ifc.ifc_len = sizeof ifreqs;
	ifc.ifc_req = ifreqs;
	if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
		perror("is_local_ip: ioctl");
		syslog(LOG_ERR, "ioctl: %m");
		return 0;
	}

	for (i = 0; i < sizeof ifreqs / sizeof ifreqs[0]; i++) {
		if (ifreqs[i].ifr_name[0] == '\0') {
			return 0;
		}
		if (((struct sockaddr_in *)&ifreqs[i].ifr_addr)->sin_addr.s_addr
		    == new_addr) {
			return 1;
		}
	}

	syslog(LOG_WARNING, "ifreqs too small to determine if address is local,"
			    " assuming it's not local!");
	return 0;
}

int main(int argc, char **argv)
{
	int sockfd;
	int r;
	int c;
	int ping_timeout = -1; /* never timeout */
	int ping_length = IGNORE_PING_PAYLOAD_SIZE;
	int change_ip_on_ping = 0;
	char newip_exec[256] = "";
	char iface[IFNAMSIZ] = IFACE_DEFAULT;

	struct ifreq ifr;
	struct sockaddr_ll sl;
	int bind_to_interface = 0;
	int no_daemon = 0;

	struct sock_fprog filter_prog;
	struct sock_filter filter_instr[] = {
		/* Is ICMP ? */
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, 23),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, IPPROTO_ICMP, 0, 3),
		/* Is Echo Request ? */
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 34),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ICMP_ECHO << 8, 0, 1),
		/* Return packet on hit */
		BPF_STMT(BPF_RET+BPF_K, (unsigned int)-1),
		/* Return no packet on miss */
		BPF_STMT(BPF_RET+BPF_K, 0),
	};

	openlog("ipsetd", LOG_PID, LOG_USER);

	while (1) {
		c = getopt(argc, argv, "t:e:l:ci:nh");

		if (c == -1) {
			/* End of option list */
			break;
		}

		switch (c) {
		case 't':
			/* timeout */
			if (optarg) {
				ping_timeout = atoi(optarg);
				if (ping_timeout < 1)
				{
					fprintf(stderr,
						"Invalid timeout argument.\n");
					exit(EXIT_FAILURE);
				}
			}
			break;

		case 'e':
			/* exec */
			if (optarg) {
				strncpy(newip_exec, optarg,
					min(strlen(optarg), sizeof(newip_exec)));
				newip_exec[sizeof(newip_exec) - 1] = '\0';
			}
			break;

		case 'l':
			/* length */
			if (optarg) {
				ping_length = atoi(optarg);
				if ((ping_length < MIN_ICMP_PAYLOAD_SIZE) ||
				    (ping_length > MAX_ICMP_PAYLOAD_SIZE)) {
					fprintf(stderr,
					       "Invalid echo request payload "
						"size %d (min %d, max %d).\n",
						ping_length,
						MIN_ICMP_PAYLOAD_SIZE,
						MAX_ICMP_PAYLOAD_SIZE);
					exit(EXIT_FAILURE);
				}
			}
			break;

		case 'c':
			/* Change the IP on ping */
			change_ip_on_ping = 1;
			break;

		case 'i':
			bind_to_interface = 1;
			if (optarg) {
				strncpy(iface, optarg, IFNAMSIZ);
				iface[IFNAMSIZ - 1] = '\0';
			}
			break;

		case 'n':
			no_daemon = 1;
			break;

		case 'h':
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);

		default:
			usage(stderr, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (!no_daemon && (daemon(0, 0) < 0)) {
		perror("daemon");
		exit(EXIT_FAILURE);
	}

	filter_prog.len = sizeof(filter_instr) / sizeof(struct sock_filter);
	filter_prog.filter = filter_instr;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

	if (sockfd < 0) {
		perror("socket");
		fprintf(stderr, "socket: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (bind_to_interface) {
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name));
		ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
		if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
			perror("SIOCGIFINDEX");
			fprintf(stderr, "SIOCGIFINDEX: %s.\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}

		memset(&sl, 0, sizeof(sl));
		sl.sll_family = AF_PACKET;
		sl.sll_protocol = htonl(ETH_P_IP);
		sl.sll_ifindex = ifr.ifr_ifindex;
		if (bind(sockfd, (struct sockaddr *)&sl, sizeof(sl))) {
			perror("bind");
			fprintf(stderr, "bind: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}


	r = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER,
		       &filter_prog, sizeof(struct sock_fprog));
	if (r < 0) {
		perror("setsockopt");
		fprintf(stderr, "setsockopt: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Timeout in %d s.", ping_timeout);
	syslog(LOG_INFO, "Expecting echo request payload %d bytes.",
	       ping_length);
	syslog(LOG_INFO, "On echo request arrival, execute %s.", newip_exec);

	if (ping_timeout > 0) {
		signal(SIGALRM, ipsetd_timeout);
		alarm(ping_timeout);
	}

	while (1) {
		struct iphdr iph;
		struct in_addr new_ip;
		struct sockaddr_ll from;
		socklen_t fromlen;
		int ping_packet_len;
		unsigned char buf[2000];

		fromlen = sizeof(from);
		r = recvfrom(sockfd, buf, sizeof(buf), 0,
			     (struct sockaddr *)&from, &fromlen);
		if (r < 0 || from.sll_pkttype != PACKET_HOST) {
			continue;
		}

		memcpy(&iph, buf + sizeof(struct ethhdr), sizeof(struct iphdr));
		new_ip.s_addr = iph.daddr;
		ping_packet_len = ntohs(iph.tot_len) - IP_ICMP_HEADERS_SIZE;

		if (((ping_length == IGNORE_PING_PAYLOAD_SIZE) ||
		     (ping_length == ping_packet_len)) &&
		    !is_local_ip(sockfd, new_ip.s_addr) &&
		    !is_local_ip(sockfd, iph.saddr)) {

			if (change_ip_on_ping) {
				syslog(LOG_INFO, "Changing IP address "
				       "on interface %s to %s.", iface,
				       inet_ntoa(new_ip));
				set_ip(sockfd, new_ip.s_addr, iface);
			}

			if (newip_exec[0]) {
				char buffer[10];

				/* Cancel timeout */
				alarm(0);
				setenv("NEW_IPADDR", inet_ntoa(new_ip), 1);
				snprintf(buffer, sizeof(buffer), "%lu",
					 (unsigned long)(ntohl(new_ip.s_addr) >> 24));
				setenv("NEW_IP_ANET", buffer, 1);
				setenv("IFNAME", iface, 1);

				r = system(newip_exec);
				if (r == -1) {
					/* Fork failed */
					syslog(LOG_ERR, "%s execution failed.",
					       newip_exec);
					exit(EXIT_FAILURE);
				}
				if (WIFEXITED(r) != 0) {
					/* Child exited normally */
					syslog(LOG_INFO, "%s exit with %d.",
					       newip_exec, WEXITSTATUS(r));
					exit(EXIT_SUCCESS);
				}
				if (WIFSIGNALED(r)) {
					/* Child exit with signal */
					syslog(LOG_ERR,
					       "%s died with signal %d.",
					       newip_exec, WTERMSIG(r));
					exit(EXIT_FAILURE);
				}
			}

			return 0;
		}
	}

	return 0;
}
