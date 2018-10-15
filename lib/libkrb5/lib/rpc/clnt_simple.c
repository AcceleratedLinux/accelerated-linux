/* @(#)clnt_simple.c	2.2 88/08/01 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)clnt_simple.c 1.35 87/08/11 Copyr 1984 Sun Micro";
#endif

/* 
 * clnt_simple.c
 * Simplified front end to rpc.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdio.h>
/* for close() */
#include <unistd.h>
#include <gssrpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <port-sockets.h>
#include "autoconf.h"

static struct callrpc_private {
	CLIENT	*client;
        SOCKET  socket;
	rpcprog_t oldprognum;
	rpcvers_t oldversnum;
	int	valid;
	char	*oldhost;
} *callrpc_private;

int
callrpc(
	char *host,
	rpcprog_t prognum,
	rpcvers_t versnum,
	rpcproc_t procnum,
	xdrproc_t inproc,
	char *in,
	xdrproc_t outproc,
	char *out)
{
	register struct callrpc_private *crp = callrpc_private;
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	if (crp == 0) {
		crp = (struct callrpc_private *)calloc(1, sizeof (*crp));
		if (crp == 0)
			return (0);
		callrpc_private = crp;
	}
	if (crp->oldhost == NULL) {
		crp->oldhost = mem_alloc(256);
		if (crp->oldhost == 0)
		    return 0;
		crp->oldhost[0] = 0;
		crp->socket = RPC_ANYSOCK;
	}
	if (crp->valid && crp->oldprognum == prognum && crp->oldversnum == versnum
		&& strcmp(crp->oldhost, host) == 0) {
		/* reuse old client */		
	} else {
		crp->valid = 0;
                (void)closesocket(crp->socket);
		crp->socket = RPC_ANYSOCK;
		if (crp->client) {
			clnt_destroy(crp->client);
			crp->client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 5;
		memset(&server_addr, 0, sizeof(server_addr));
		memmove((char *)&server_addr.sin_addr, hp->h_addr, 
			sizeof(server_addr.sin_addr));
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
		server_addr.sin_len = sizeof(server_addr);
#endif
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((crp->client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &crp->socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		crp->valid = 1;
		crp->oldprognum = prognum;
		crp->oldversnum = versnum;
		(void) strncpy(crp->oldhost, host, 255);
		crp->oldhost[255] = '\0';
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(crp->client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		crp->valid = 0;
	return ((int) clnt_stat);
}
