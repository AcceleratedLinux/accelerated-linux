/* Definitions for tcpsrv class.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
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
#ifndef INCLUDED_TCPSRV_H
#define INCLUDED_TCPSRV_H

#include "obj.h"
#include "tcps_sess.h"

/* the tcpsrv object */
typedef struct tcpsrv_s {
	BEGINobjInstance;	/**< Data to implement generic object - MUST be the first data element! */
	int *pSocksLstn;	/**< listen socket array for server [0] holds count */
	int iSessMax;		/**< max number of sessions supported */
	char *TCPLstnNode;	/**< the node the listener shall listen on */
	char *TCPLstnPort;	/**< the port the listener shall listen on */
	tcps_sess_t **pSessions;/**< array of all of our sessions */
	void *pUsr;		/**< a user-settable pointer (provides extensibility for "derived classes")*/
	/* callbacks */
	int      (*pIsPermittedHost)(struct sockaddr *addr, char *fromHostFQDN, void*pUsrSrv, void*pUsrSess);
	int      (*pRcvData)(tcps_sess_t*, char*, size_t);
	int*     (*OpenLstnSocks)(struct tcpsrv_s*);
	rsRetVal (*pOnListenDeinit)(void*);
	rsRetVal (*OnDestruct)(void*);
	rsRetVal (*pOnRegularClose)(tcps_sess_t *pSess);
	rsRetVal (*pOnErrClose)(tcps_sess_t *pSess);
	/* session specific callbacks */
	rsRetVal (*pOnSessAccept)(struct tcpsrv_s *, tcps_sess_t*);
	rsRetVal (*OnSessConstructFinalize)(void*);
	rsRetVal (*pOnSessDestruct)(void*);
} tcpsrv_t;


/* interfaces */
BEGINinterface(tcpsrv) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(tcpsrv);
	rsRetVal (*Construct)(tcpsrv_t **ppThis);
	rsRetVal (*ConstructFinalize)(tcpsrv_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(tcpsrv_t **ppThis);
	void (*configureTCPListen)(tcpsrv_t*, char *cOptarg);
	int (*SessAccept)(tcpsrv_t *pThis, tcps_sess_t**ppSess, int fd);
	int* (*create_tcp_socket)(tcpsrv_t *pThis);
	rsRetVal (*Run)(tcpsrv_t *pThis);
	/* set methods */
	rsRetVal (*SetUsrP)(tcpsrv_t*, void*);
	rsRetVal (*SetCBIsPermittedHost)(tcpsrv_t*, int (*) (struct sockaddr *addr, char*, void*, void*));
	rsRetVal (*SetCBOpenLstnSocks)(tcpsrv_t *, int* (*)(tcpsrv_t*));
	rsRetVal (*SetCBRcvData)(tcpsrv_t *, int (*)(tcps_sess_t*, char*, size_t));
	rsRetVal (*SetCBOnListenDeinit)(tcpsrv_t*, rsRetVal (*)(void*));
	rsRetVal (*SetCBOnDestruct)(tcpsrv_t*, rsRetVal (*) (void*));
	rsRetVal (*SetCBOnRegularClose)(tcpsrv_t*, rsRetVal (*) (tcps_sess_t*));
	rsRetVal (*SetCBOnErrClose)(tcpsrv_t*, rsRetVal (*) (tcps_sess_t*));
	/* session specifics */
	rsRetVal (*SetCBOnSessAccept)(tcpsrv_t*, rsRetVal (*) (tcpsrv_t*, tcps_sess_t*));
	rsRetVal (*SetCBOnSessDestruct)(tcpsrv_t*, rsRetVal (*) (void*));
	rsRetVal (*SetCBOnSessConstructFinalize)(tcpsrv_t*, rsRetVal (*) (void*));
ENDinterface(tcpsrv)
#define tcpsrvCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(tcpsrv);

/* the name of our library binary */
#define LM_TCPSRV_FILENAME "lmtcpsrv"

#endif /* #ifndef INCLUDED_TCPSRV_H */
