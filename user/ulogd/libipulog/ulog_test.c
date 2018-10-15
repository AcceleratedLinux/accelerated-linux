/* ulog_test, $Revision: 740 $
 *
 * small testing program for libipulog, part of the netfilter ULOG target
 * for the linux 2.4 netfilter subsystem.
 *
 * (C) 2000-2005 by Harald Welte <laforge@gnumonks.org>
 *
 * this code is released under the terms of GNU GPL
 *
 * $Id: ulog_test.c 740 2005-03-11 11:47:53Z laforge $
 */

#include <stdio.h>
#include <stdlib.h>
#include <libipulog/libipulog.h>

#define MYBUFSIZ 2048

/* prints some logging about a single packet */
void handle_packet(ulog_packet_msg_t *pkt)
{
	unsigned char *p;
	int i;
	
	printf("Hook=%u Mark=%lu len=%d ",
	       pkt->hook, pkt->mark, pkt->data_len);
	if (strlen(pkt->prefix))
		printf("Prefix=%s ", pkt->prefix);
	
	if (pkt->mac_len)
	{
		printf("mac=");
		p = pkt->mac;
		for (i = 0; i < pkt->mac_len; i++, p++)
			printf("%02x%c", *p, i==pkt->mac_len-1 ? ' ':':');
	}
	printf("\n");

}

int main(int argc, char *argv[])
{
	struct ipulog_handle *h;
	unsigned char* buf;
	int len;
	ulog_packet_msg_t *upkt;
	int i;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s count group timeout\n", argv[0]);
		exit(1);
	}

	/* allocate a receive buffer */
	buf = (unsigned char *) malloc(MYBUFSIZ);
	
	/* create ipulog handle */
	h = ipulog_create_handle(ipulog_group2gmask(atoi(argv[2])),150000);
	if (!h)
	{
		/* if some error occurrs, print it to stderr */
		ipulog_perror(NULL);
		exit(1);
	}

	alarm(atoi(argv[3]));

	/* loop receiving packets and handling them over to handle_packet */
	for (i = 0; i < atoi(argv[1]); i++) {
		len = ipulog_read(h, buf, MYBUFSIZ, 1);
		if (len <= 0) {
			ipulog_perror("ulog_test: short read");
			exit(1);
		}
		printf("%d bytes received\n", len);
		while (upkt = ipulog_get_packet(h, buf, len)) {
			handle_packet(upkt);
		}
	}
	
	/* just to give it a cleaner look */
	ipulog_destroy_handle(h);
	return 0;
}
