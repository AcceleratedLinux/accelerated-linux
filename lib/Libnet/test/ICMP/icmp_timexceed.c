/*
 *  $Id: icmp_timexceed.c,v 1.1.1.1 2000/05/25 00:28:49 route Exp $
 *
 *  libnet
 *  icmp_timexceed.c - ICMP_TIMEXCEED Packet assembly tester
 *
 *  Copyright (c) 1998, 1999, 2000 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

int
main(int argc, char **argv)
{
    int sock, c;
    u_char *buf;
    u_long src_ip, dst_ip;

    printf("ICMP_TIMEXCEED packet building/writing test\n");

    src_ip = 0;
    dst_ip = 0;
    while((c = getopt(argc, argv, "d:s:")) != EOF)
    {
        switch (c)
        {
            case 'd':
                if (!(dst_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad destination IP address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 's':
                if (!(src_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad source IP address: %s\n", optarg);
                    exit(1);
                }
                break;
        }
    }
    if (!src_ip || !dst_ip)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     *  Get our block of memory for the packet.  In this case, we only need
     *  memory for the packet headers.
     */
    buf = malloc(IP_MAXPACKET);
    if (!buf)
    {
        perror("No memory for packet header");
        exit(EXIT_FAILURE);
    }

    /*
     *  Open our raw IP socket and set IP_HDRINCL.
     */
    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1)
    {
        perror("No socket");
        exit(EXIT_FAILURE);
    }
    
    /*
     *  Build the IP header (shown exploded for commenting).
     */
    libnet_build_ip(LIBNET_ICMP_TIMXCEED_H + LIBNET_IP_H, /* Size of the payload */
            IPTOS_LOWDELAY | IPTOS_THROUGHPUT,  /* IP tos */
            242,                                /* IP ID */
            0,                                  /* Frag stuff */
            48,                                 /* TTL */
            IPPROTO_ICMP,                       /* Transport protocol */
            src_ip,                             /* Source IP */
            dst_ip,                             /* Destination IP */
            NULL,                               /* Pointer to payload (none) */
            0,
            buf);                               /* Packet header memory */

    /*
     *  Build the ICMP header.
     */
    libnet_build_icmp_timeexceed(ICMP_TIMXCEED,     /* type */
                ICMP_TIMXCEED_INTRANS,              /* code */
                0,
                IPTOS_LOWDELAY | IPTOS_THROUGHPUT,  /* IP tos */
                424,                                /* IP ID */
                0,                                  /* Frag stuff */
                64,                                 /* TTL */
                IPPROTO_ICMP,                       /* Transport protocol */
                dst_ip,                             /* Source IP */
                src_ip,                             /* Destination IP */
                NULL,	                            /* pointer to payload */
                0,                                  /* size of payload */
                buf + LIBNET_IP_H);                 /* packet header memory */

    libnet_do_checksum(buf, IPPROTO_ICMP, LIBNET_ICMP_TIMXCEED_H + LIBNET_IP_H);

    /*
     *  Write the packet to the network.
     */
    c = libnet_write_ip(sock, buf, LIBNET_ICMP_TIMXCEED_H + 2 * LIBNET_IP_H);
    if (c < LIBNET_ICMP_TIMXCEED_H + 2 * LIBNET_IP_H)
    {
        fprintf(stderr, "libnet_write_ip\n");
    }
    printf("Completed, wrote %d bytes\n", c);
    free(buf);

    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


void
usage(u_char *name)
{
    fprintf(stderr, "usage: %s -s source_ip -d destination_ip\n ", name);
}

/* EOF */
