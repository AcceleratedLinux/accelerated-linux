/* @(#)auth_none.c	2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)auth_none.c 1.19 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * auth_none.c
 * Creates a client authentication handle for passing "null" 
 * credentials and verifiers to remote systems. 
 * 
 * Copyright (C) 1984, Sun Microsystems, Inc. 
 */

#include <gssrpc/types.h>
#include <gssrpc/xdr.h>
#include <gssrpc/auth.h>
#include <stdlib.h>
#define MAX_MARSHEL_SIZE 20

/*
 * Authenticator operations routines
 */
static void	authnone_verf(AUTH *);
static void	authnone_destroy(AUTH *);
static bool_t	authnone_marshal(AUTH *, XDR *);
static bool_t	authnone_validate(AUTH *, struct opaque_auth *);
static bool_t	authnone_refresh(AUTH *, struct rpc_msg *);
static bool_t	authnone_wrap(AUTH *, XDR *, xdrproc_t, caddr_t);

static struct auth_ops ops = {
	authnone_verf,
	authnone_marshal,
	authnone_validate,
	authnone_refresh,
	authnone_destroy,
	authnone_wrap,
	authnone_wrap
};

static struct authnone_private {
	AUTH	no_client;
	char	marshalled_client[MAX_MARSHEL_SIZE];
	u_int	mcnt;
} *authnone_private;

AUTH *
authnone_create(void)
{
	register struct authnone_private *ap = authnone_private;
	XDR xdr_stream;
	register XDR *xdrs;

	if (ap == 0) {
		ap = (struct authnone_private *)calloc(1, sizeof (*ap));
		if (ap == 0)
			return (0);
		authnone_private = ap;
	}
	if (!ap->mcnt) {
		ap->no_client.ah_cred = ap->no_client.ah_verf = gssrpc__null_auth;
		ap->no_client.ah_ops = &ops;
		xdrs = &xdr_stream;
		xdrmem_create(xdrs, ap->marshalled_client, (u_int)MAX_MARSHEL_SIZE,
		    XDR_ENCODE);
		(void)xdr_opaque_auth(xdrs, &ap->no_client.ah_cred);
		(void)xdr_opaque_auth(xdrs, &ap->no_client.ah_verf);
		ap->mcnt = XDR_GETPOS(xdrs);
		XDR_DESTROY(xdrs);
	}
	return (&ap->no_client);
}

/*ARGSUSED*/
static bool_t
authnone_marshal(AUTH *client, XDR *xdrs)
{
	register struct authnone_private *ap = authnone_private;

	if (ap == 0)
		return (0);
	return ((*xdrs->x_ops->x_putbytes)(xdrs,
	    ap->marshalled_client, ap->mcnt));
}

/*ARGSUSED*/
static void 
authnone_verf(AUTH *auth)
{
}

/*ARGSUSED*/
static bool_t
authnone_validate(AUTH *auth, struct opaque_auth *verf)
{

	return (TRUE);
}

/*ARGSUSED*/
static bool_t
authnone_refresh(AUTH *auth, struct rpc_msg *msg)
{

	return (FALSE);
}

/*ARGSUSED*/
static void
authnone_destroy(AUTH *auth)
{
}

static bool_t
authnone_wrap(AUTH *auth, XDR *xdrs, xdrproc_t xfunc, caddr_t xwhere)
{
	return ((*xfunc)(xdrs, xwhere));
}
