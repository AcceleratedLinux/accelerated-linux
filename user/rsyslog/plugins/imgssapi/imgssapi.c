/* imgssapi.c
 * This is the implementation of the GSSAPI input module.
 *
 * Note: the root gssapi code was contributed by varmojfekoj and is most often
 * maintened by him. I am just doing the plumbing around it (I event don't have a
 * test lab for gssapi yet... ). I am very grateful for this useful code
 * contribution -- rgerhards, 2008-03-05
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * Copyright 2007 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <gssapi/gssapi.h>
#include "rsyslog.h"
#include "syslogd.h"
#include "cfsysline.h"
#include "module-template.h"
#include "net.h"
#include "srUtils.h"
#include "gss-misc.h"
#include "tcpsrv.h"
#include "tcps_sess.h"
#include "errmsg.h"


MODULE_TYPE_INPUT

/* defines */
#define ALLOWEDMETHOD_GSS 2
#define ALLOWEDMETHOD_TCP 1


/* some forward definitions - they may go away when we no longer include imtcp.c */
static rsRetVal addGSSListener(void __attribute__((unused)) *pVal, uchar *pNewVal);
static int TCPSessGSSInit(void);
static void TCPSessGSSClose(tcps_sess_t* pSess);
static int TCPSessGSSRecv(tcps_sess_t *pSess, void *buf, size_t buf_len);
static rsRetVal onSessAccept(tcpsrv_t *pThis, tcps_sess_t *ppSess);
static rsRetVal OnSessAcceptGSS(tcpsrv_t *pThis, tcps_sess_t *ppSess);

/* static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(tcpsrv)
DEFobjCurrIf(tcps_sess)
DEFobjCurrIf(gssutil)
DEFobjCurrIf(errmsg)
DEFobjCurrIf(net)

static tcpsrv_t *pOurTcpsrv = NULL;  /* our TCP server(listener) TODO: change for multiple instances */
static gss_cred_id_t gss_server_creds = GSS_C_NO_CREDENTIAL;

/* our usr structure for the tcpsrv object */
typedef struct gsssrv_s {
	char allowedMethods;
} gsssrv_t;

/* our usr structure for the session object */
typedef struct gss_sess_s {
	OM_uint32 gss_flags;
	gss_ctx_id_t gss_context;
	char allowedMethods;
} gss_sess_t;


/* config variables */
static int iTCPSessMax = 200; /* max number of sessions */
static char *gss_listen_service_name = NULL;
static int bPermitPlainTcp = 0; /* plain tcp syslog allowed on GSSAPI port? */


/* methods */
/* callbacks */
static rsRetVal OnSessConstructFinalize(void *ppUsr)
{
	DEFiRet;
	gss_sess_t **ppGSess = (gss_sess_t**) ppUsr;
	gss_sess_t *pGSess;

	assert(ppGSess != NULL);

	if((pGSess = calloc(1, sizeof(gss_sess_t))) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	pGSess->gss_flags = 0;
	pGSess->gss_context = GSS_C_NO_CONTEXT;
	pGSess->allowedMethods = 0;

	*ppGSess = pGSess;

finalize_it:
	RETiRet;
}


/* Destruct the user session pointer for a GSSAPI session. Please note
 * that it *is* valid to receive a NULL user pointer. In this case, the
 * sessions is to be torn down before it was fully initialized. This 
 * happens in error cases, e.g. when the host ACL did not match.
 * rgerhards, 2008-03-03
 */
static rsRetVal
OnSessDestruct(void *ppUsr)
{
	DEFiRet;
	gss_sess_t **ppGSess = (gss_sess_t**) ppUsr;

	assert(ppGSess != NULL);
	if(*ppGSess == NULL)
		FINALIZE;

	if((*ppGSess)->allowedMethods & ALLOWEDMETHOD_GSS) {
		OM_uint32 maj_stat, min_stat;
		maj_stat = gss_delete_sec_context(&min_stat, &(*ppGSess)->gss_context, GSS_C_NO_BUFFER);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status("deleting context", maj_stat, min_stat);
	}

	free(*ppGSess);
	*ppGSess = NULL;

finalize_it:
	RETiRet;
}


/* Check if the host is permitted to send us messages.
 * Note: the pUsrSess may be zero if the server is running in tcp-only mode!
 */
static int
isPermittedHost(struct sockaddr *addr, char *fromHostFQDN, void *pUsrSrv, void*pUsrSess)
{
	gsssrv_t *pGSrv;
	gss_sess_t *pGSess;
	char allowedMethods = 0;

	BEGINfunc
	assert(pUsrSrv != NULL);
	pGSrv = (gsssrv_t*) pUsrSrv;
	pGSess = (gss_sess_t*) pUsrSess;

	if((pGSrv->allowedMethods & ALLOWEDMETHOD_TCP) &&
	   net.isAllowedSender(net.pAllowedSenders_TCP, addr, (char*)fromHostFQDN))
		allowedMethods |= ALLOWEDMETHOD_TCP;
	if((pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) &&
	   net.isAllowedSender(net.pAllowedSenders_GSS, addr, (char*)fromHostFQDN))
		allowedMethods |= ALLOWEDMETHOD_GSS;
	if(allowedMethods && pGSess != NULL)
		pGSess->allowedMethods = allowedMethods;
	ENDfunc
	return allowedMethods;
}

static rsRetVal
onSessAccept(tcpsrv_t *pThis, tcps_sess_t *pSess)
{
	DEFiRet;
	gsssrv_t *pGSrv;
	
	pGSrv = (gsssrv_t*) pThis->pUsr;

	if(pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) {
		iRet = OnSessAcceptGSS(pThis, pSess);
	} 

	RETiRet;
}


static rsRetVal
onRegularClose(tcps_sess_t *pSess)
{
	DEFiRet;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	if(pGSess->allowedMethods & ALLOWEDMETHOD_GSS)
		TCPSessGSSClose(pSess);
	else {
		/* process any incomplete frames left over */
		tcps_sess.PrepareClose(pSess);
		/* Session closed */
		tcps_sess.Close(pSess);
	}
	RETiRet;
}


static rsRetVal
onErrClose(tcps_sess_t *pSess)
{
	DEFiRet;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	if(pGSess->allowedMethods & ALLOWEDMETHOD_GSS)
		TCPSessGSSClose(pSess);
	else
		tcps_sess.Close(pSess);

	RETiRet;
}


/* open the listen sockets */
static int*
doOpenLstnSocks(tcpsrv_t *pSrv)
{
	int *pRet = NULL;
	gsssrv_t *pGSrv;

	ISOBJ_TYPE_assert(pSrv, tcpsrv);
	pGSrv = pSrv->pUsr;
	assert(pGSrv != NULL);

	/* first apply some config settings */
	if(pGSrv->allowedMethods) {
		if(pGSrv->allowedMethods & ALLOWEDMETHOD_GSS) {
			if(TCPSessGSSInit()) {
				errmsg.LogError(NO_ERRCODE, "GSS-API initialization failed\n");
				pGSrv->allowedMethods &= ~(ALLOWEDMETHOD_GSS);
			}
		}
		if(pGSrv->allowedMethods) {
			/* fallback to plain TCP */
			if((pRet =  tcpsrv.create_tcp_socket(pSrv)) != NULL) {
				dbgprintf("Opened %d syslog TCP port(s).\n", *pRet);
			}
		}
	}

	return pRet;
}


static int
doRcvData(tcps_sess_t *pSess, char *buf, size_t lenBuf)
{
	int state;
	int allowedMethods;
	gss_sess_t *pGSess;

	assert(pSess != NULL);
	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	allowedMethods = pGSess->allowedMethods;
	if(allowedMethods & ALLOWEDMETHOD_GSS)
		state = TCPSessGSSRecv(pSess, buf, lenBuf);
	else
		state = recv(pSess->sock, buf, lenBuf, 0);
	return state;
}


/* end callbacks */

static rsRetVal
addGSSListener(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	DEFiRet;
	gsssrv_t *pGSrv;

	if(pOurTcpsrv == NULL) {
		/* first create/init the gsssrv "object" */
		if((pGSrv = calloc(1, sizeof(gsssrv_t))) == NULL)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

		pGSrv->allowedMethods = ALLOWEDMETHOD_GSS;
		if(bPermitPlainTcp)
			pGSrv->allowedMethods |= ALLOWEDMETHOD_TCP;
		/* gsssrv initialized */

		CHKiRet(tcpsrv.Construct(&pOurTcpsrv));
		CHKiRet(tcpsrv.SetUsrP(pOurTcpsrv, pGSrv));
		CHKiRet(tcpsrv.SetCBOnSessConstructFinalize(pOurTcpsrv, OnSessConstructFinalize));
		CHKiRet(tcpsrv.SetCBOnSessDestruct(pOurTcpsrv, OnSessDestruct));
		CHKiRet(tcpsrv.SetCBIsPermittedHost(pOurTcpsrv, isPermittedHost));
		CHKiRet(tcpsrv.SetCBRcvData(pOurTcpsrv, doRcvData));
		CHKiRet(tcpsrv.SetCBOpenLstnSocks(pOurTcpsrv, doOpenLstnSocks));
		CHKiRet(tcpsrv.SetCBOnSessAccept(pOurTcpsrv, onSessAccept));
		CHKiRet(tcpsrv.SetCBOnRegularClose(pOurTcpsrv, onRegularClose));
		CHKiRet(tcpsrv.SetCBOnErrClose(pOurTcpsrv, onErrClose));
		tcpsrv.configureTCPListen(pOurTcpsrv, (char *) pNewVal);
		CHKiRet(tcpsrv.ConstructFinalize(pOurTcpsrv));
	}

finalize_it:
	RETiRet;
}


/* returns 0 if all went OK, -1 if it failed */
static int TCPSessGSSInit(void)
{
	gss_buffer_desc name_buf;
	gss_name_t server_name;
	OM_uint32 maj_stat, min_stat;

	if (gss_server_creds != GSS_C_NO_CREDENTIAL)
		return 0;

	name_buf.value = (gss_listen_service_name == NULL) ? "host" : gss_listen_service_name;
	name_buf.length = strlen(name_buf.value) + 1;
	maj_stat = gss_import_name(&min_stat, &name_buf, GSS_C_NT_HOSTBASED_SERVICE, &server_name);
	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status("importing name", maj_stat, min_stat);
		return -1;
	}

	maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
				    GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
				    &gss_server_creds, NULL, NULL);
	if (maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status("acquiring credentials", maj_stat, min_stat);
		return -1;
	}

	gss_release_name(&min_stat, &server_name);
	dbgprintf("GSS-API initialized\n");
	return 0;
}


/* returns 0 if all went OK, -1 if it failed 
 * tries to guess if the connection uses gssapi.
 */
static rsRetVal
OnSessAcceptGSS(tcpsrv_t *pThis, tcps_sess_t *pSess)
{
	DEFiRet;
	gss_buffer_desc send_tok, recv_tok;
	gss_name_t client;
	OM_uint32 maj_stat, min_stat, acc_sec_min_stat;
	gss_ctx_id_t *context;
	OM_uint32 *sess_flags;
	int fdSess;
	char allowedMethods;
	gsssrv_t *pGSrv;
	gss_sess_t *pGSess;

	assert(pSess != NULL);

	pGSrv = (gsssrv_t*) pThis->pUsr;
	pGSess = (gss_sess_t*) pSess->pUsr;
	allowedMethods = pGSrv->allowedMethods;
	if(allowedMethods & ALLOWEDMETHOD_GSS) {
		/* Buffer to store raw message in case that
		 * gss authentication fails halfway through.
		 */
		char buf[MAXLINE];
		int ret = 0;

		dbgprintf("GSS-API Trying to accept TCP session %p\n", pSess);

		fdSess = pSess->sock; // TODO: method access!
		if (allowedMethods & ALLOWEDMETHOD_TCP) {
			int len;
			fd_set  fds;
			struct timeval tv;
		
			do {
				FD_ZERO(&fds);
				FD_SET(fdSess, &fds);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				ret = select(fdSess + 1, &fds, NULL, NULL, &tv);
			} while (ret < 0 && errno == EINTR);
			if (ret < 0) {
				errmsg.LogError(NO_ERRCODE, "TCP session %p will be closed, error ignored\n", pSess);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			} else if (ret == 0) {
				dbgprintf("GSS-API Reverting to plain TCP\n");
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			}

			do {
				ret = recv(fdSess, buf, sizeof (buf), MSG_PEEK);
			} while (ret < 0 && errno == EINTR);
			if (ret <= 0) {
				if (ret == 0)
					dbgprintf("GSS-API Connection closed by peer\n");
				else
					errmsg.LogError(NO_ERRCODE, "TCP(GSS) session %p will be closed, error ignored\n", pSess);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}

			if (ret < 4) {
				dbgprintf("GSS-API Reverting to plain TCP\n");
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			} else if (ret == 4) {
				/* The client might has been interupted after sending
				 * the data length (4B), give him another chance.
				 */
				srSleep(1, 0);
				do {
					ret = recv(fdSess, buf, sizeof (buf), MSG_PEEK);
				} while (ret < 0 && errno == EINTR);
				if (ret <= 0) {
					if (ret == 0)
						dbgprintf("GSS-API Connection closed by peer\n");
					else
						errmsg.LogError(NO_ERRCODE, "TCP session %p will be closed, error ignored\n", pSess);
					ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
				}
			}

			/* TODO: how does this work together with IPv6? Does it? */
			len = ntohl((buf[0] << 24)
				    | (buf[1] << 16)
				    | (buf[2] << 8)
				    | buf[3]);
			if ((ret - 4) < len || len == 0) {
				dbgprintf("GSS-API Reverting to plain TCP\n");
				pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
				ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
			}
		}

		context = &pGSess->gss_context;
		*context = GSS_C_NO_CONTEXT;
		sess_flags = &pGSess->gss_flags;
		do {
			if (gssutil.recv_token(fdSess, &recv_tok) <= 0) {
				errmsg.LogError(NO_ERRCODE, "TCP session %p will be closed, error ignored\n", pSess);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}
			maj_stat = gss_accept_sec_context(&acc_sec_min_stat, context, gss_server_creds,
							  &recv_tok, GSS_C_NO_CHANNEL_BINDINGS, &client,
							  NULL, &send_tok, sess_flags, NULL, NULL);
			if (recv_tok.value) {
				free(recv_tok.value);
				recv_tok.value = NULL;
			}
			if (maj_stat != GSS_S_COMPLETE && maj_stat != GSS_S_CONTINUE_NEEDED) {
				gss_release_buffer(&min_stat, &send_tok);
				if (*context != GSS_C_NO_CONTEXT)
					gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
				if ((allowedMethods & ALLOWEDMETHOD_TCP) && 
				    (GSS_ROUTINE_ERROR(maj_stat) == GSS_S_DEFECTIVE_TOKEN)) {
					dbgprintf("GSS-API Reverting to plain TCP\n");
					dbgprintf("tcp session socket with new data: #%d\n", fdSess);
					if(tcps_sess.DataRcvd(pSess, buf, ret) != RS_RET_OK) {
						errmsg.LogError(NO_ERRCODE, "Tearing down TCP Session %p - see "
							    "previous messages for reason(s)\n", pSess);
						ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
					}
					pGSess->allowedMethods = ALLOWEDMETHOD_TCP;
					ABORT_FINALIZE(RS_RET_OK); // TODO: define good error codes
				}
				gssutil.display_status("accepting context", maj_stat, acc_sec_min_stat);
				ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
			}
			if (send_tok.length != 0) {
				if(gssutil.send_token(fdSess, &send_tok) < 0) {
					gss_release_buffer(&min_stat, &send_tok);
					errmsg.LogError(NO_ERRCODE, "TCP session %p will be closed, error ignored\n", pSess);
					if (*context != GSS_C_NO_CONTEXT)
						gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
					ABORT_FINALIZE(RS_RET_ERR); // TODO: define good error codes
				}
				gss_release_buffer(&min_stat, &send_tok);
			}
		} while (maj_stat == GSS_S_CONTINUE_NEEDED);

		maj_stat = gss_display_name(&min_stat, client, &recv_tok, NULL);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status("displaying name", maj_stat, min_stat);
		else
			dbgprintf("GSS-API Accepted connection from: %s\n", (char*) recv_tok.value);
		gss_release_name(&min_stat, &client);
		gss_release_buffer(&min_stat, &recv_tok);

		dbgprintf("GSS-API Provided context flags:\n");
		gssutil.display_ctx_flags(*sess_flags);
		pGSess->allowedMethods = ALLOWEDMETHOD_GSS;
	}
	
finalize_it:
	RETiRet;
}


/* returns: number of bytes read or -1 on error
 * Replaces recv() for gssapi connections.
 */
int TCPSessGSSRecv(tcps_sess_t *pSess, void *buf, size_t buf_len)
{
	gss_buffer_desc xmit_buf, msg_buf;
	gss_ctx_id_t *context;
	OM_uint32 maj_stat, min_stat;
	int fdSess;
	int     conf_state;
	int state, len;
	gss_sess_t *pGSess;

	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	fdSess = pSess->sock;
	if ((state = gssutil.recv_token(fdSess, &xmit_buf)) <= 0)
		return state;

	context = &pGSess->gss_context;
	maj_stat = gss_unwrap(&min_stat, *context, &xmit_buf, &msg_buf,
			      &conf_state, (gss_qop_t *) NULL);
	if(maj_stat != GSS_S_COMPLETE) {
		gssutil.display_status("unsealing message", maj_stat, min_stat);
		if(xmit_buf.value) {
			free(xmit_buf.value);
			xmit_buf.value = 0;
		}
		return (-1);
	}
	if (xmit_buf.value) {
		free(xmit_buf.value);
		xmit_buf.value = 0;
	}

	len = msg_buf.length < buf_len ? msg_buf.length : buf_len;
	memcpy(buf, msg_buf.value, len);
	gss_release_buffer(&min_stat, &msg_buf);

	return len;
}


/* Takes care of cleaning up gssapi stuff and then calls
 * TCPSessClose().
 */
void TCPSessGSSClose(tcps_sess_t* pSess)
{
	OM_uint32 maj_stat, min_stat;
	gss_ctx_id_t *context;
	gss_sess_t *pGSess;

	assert(pSess->pUsr != NULL);
	pGSess = (gss_sess_t*) pSess->pUsr;

	context = &pGSess->gss_context;
	maj_stat = gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
	if (maj_stat != GSS_S_COMPLETE)
		gssutil.display_status("deleting context", maj_stat, min_stat);
	*context = GSS_C_NO_CONTEXT;
	pGSess->gss_flags = 0;
	pGSess->allowedMethods = 0;

	tcps_sess.Close(pSess);
}


/* Counterpart of TCPSessGSSInit(). This is called to exit the GSS system
 * at all. It is a server-based session exit. 
 */
static rsRetVal
TCPSessGSSDeinit(void)
{
	DEFiRet;
	OM_uint32 maj_stat, min_stat;

	if (gss_server_creds != GSS_C_NO_CREDENTIAL) {
		maj_stat = gss_release_cred(&min_stat, &gss_server_creds);
		if (maj_stat != GSS_S_COMPLETE)
			gssutil.display_status("releasing credentials", maj_stat, min_stat);
	}
	RETiRet;
}

/* This function is called to gather input.
 */
BEGINrunInput
CODESTARTrunInput
	iRet = tcpsrv.Run(pOurTcpsrv);
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
	if(pOurTcpsrv == NULL)
		ABORT_FINALIZE(RS_RET_NO_RUN);

	net.PrintAllowedSenders(2); /* TCP */
	net.PrintAllowedSenders(3); /* GSS */
finalize_it:
ENDwillRun



BEGINmodExit
CODESTARTmodExit
	if(pOurTcpsrv != NULL)
		iRet = tcpsrv.Destruct(&pOurTcpsrv);
	TCPSessGSSDeinit();

	/* release objects we used */
	objRelease(tcps_sess, LM_TCPSRV_FILENAME);
	objRelease(tcpsrv, LM_TCPSRV_FILENAME);
	objRelease(gssutil, LM_GSSUTIL_FILENAME);
	objRelease(errmsg, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
ENDmodExit


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
	if (net.pAllowedSenders_TCP != NULL) {
		net.clearAllowedSenders (net.pAllowedSenders_TCP);
		net.pAllowedSenders_TCP = NULL;
	}
	if (net.pAllowedSenders_GSS != NULL) {
		net.clearAllowedSenders (net.pAllowedSenders_GSS);
		net.pAllowedSenders_GSS = NULL;
	}
ENDafterRun


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
ENDqueryEtryPt


static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	if (gss_listen_service_name != NULL) {
		free(gss_listen_service_name);
		gss_listen_service_name = NULL;
	}
	bPermitPlainTcp = 0;
	iTCPSessMax = 200;
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current definition */
CODEmodInit_QueryRegCFSLineHdlr
	pOurTcpsrv = NULL;
	/* request objects we use */
	CHKiRet(objUse(tcps_sess, LM_TCPSRV_FILENAME));
	CHKiRet(objUse(tcpsrv, LM_TCPSRV_FILENAME));
	CHKiRet(objUse(gssutil, LM_GSSUTIL_FILENAME));
	CHKiRet(objUse(errmsg, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverpermitplaintcp", 0, eCmdHdlrBinary,
				   NULL, &bPermitPlainTcp, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverrun", 0, eCmdHdlrGetWord,
				   addGSSListener, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssserverservicename", 0, eCmdHdlrGetWord,
				   NULL, &gss_listen_service_name, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputgssservermaxsessions", 0, eCmdHdlrInt,
				   NULL, &iTCPSessMax, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vim:set ai:
 */
