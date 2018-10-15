/*
 *  $Id: libnet_link_nit.c,v 1.1.1.1 2000/05/25 00:28:49 route Exp $
 *
 *  libnet
 *  libnet_nit.c - network interface tap routines
 *
 *  Copyright (c) 1998 - 2001 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"

#include "../include/gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif

struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    struct sockaddr_nit snit;
    register struct libnet_link_int *l;

    l = (struct libnet_link_int *)malloc(sizeof(*p));
    if (l == NULL)
    {
        strcpy(ebuf, ll_strerror(errno));
        return (NULL);
    }

    memset(l, 0, sizeof(*l));

    l->fd = socket(AF_NIT, SOCK_RAW, NITPROTO_RAW);
    if (l->fd < 0)
    {
        sprintf(ebuf, "socket: %s", ll_strerror(errno));
        goto bad;
    }
    snit.snit_family = AF_NIT;
    strncpy(snit.snit_ifname, device, NITIFSIZ);

    if (bind(l->fd, (struct sockaddr *)&snit, sizeof(snit)))
    {
        sprintf(ebuf, "bind: %s: %s", snit.snit_ifname, ll_strerror(errno));
        goto bad;
    }

    /*
     * NIT supports only ethernets.
     */
    l->linktype = DLT_EN10MB;

    return (l);

bad:
    if (l->fd >= 0)
    {
        close(l->fd);
    }
    free(l);
    return (NULL);
}


int
libnet_close_link_interface(struct libnet_link_int *l)
{
    if (close(l->fd) == 0)
    {
        return (1);
    }
    else
    {
        return (-1);
    }
}


int
write_link_layer(struct libnet_link_int *l, const char *device,
            u_char *buf, int len)
{
    int c;
    struct sockaddr sa;

    memset(&sa, 0, sizeof(sa));
    strncpy(sa.sa_data, device, sizeof(sa.sa_data));

    c = sendto(l->fd, buf, len, 0, &sa, sizeof(sa));
    if (c != len)
    {
#if (__DEBUG)
        libnet_error(LN_ERR_WARNING,
                "write_link_layer: %d bytes written (%s)\n", c,
                strerror(errno));
#endif
    }
    return (c);
}

