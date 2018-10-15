/*
 * matrix_ssl.c
 *
 * MatrixSSL  GoAhead integration layer
 *
 */

#ifdef MATRIX_SSL

/******************************** Description *********************************/



/********************************* Includes ***********************************/

#include "wsIntrn.h"
#include "webs.h"
#include "websSSL.h"
#include "sslSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/******************************* Definitions **********************************/

#define CERT_PATH    "/etc_ro"
#define SSL_CERTFILE "servercert.pem"
#define SSL_PRIVFILE "serverkey.pem"

#define SSL_PORT     443

static int       sslListenSock   = -1;            /* Listen socket */
static sslKeys_t *sslKeys = NULL;


/******************************* Prototypes  **********************************/

int         websSSLAccept(int sid, char *ipaddr, int port, int listenSid);
static void websSSLSocketEvent(int sid, int mask, int iwp);
static int  websSSLReadEvent(webs_t wp);


/******************************************************************************/
/*
 *    Start up the SSL Context for the application, and start a listen on the
 *    SSL port (usually 443, and defined by SSL_PORT)
 *    Return 0 on success, -1 on failure.
 */

//int websSSLOpen(char_t* certPath, int sslPort)
int websSSLOpen(void)
{
	char* certFile = NULL;
	char* privFile = NULL;

	if (0 < matrixSslOpen()) {
		return -1;
	}

	privFile = (char*) malloc(strlen(CERT_PATH) + strlen(SSL_PRIVFILE) + 1);
	sprintf(privFile, "%s/%s", CERT_PATH, SSL_PRIVFILE);
	certFile = (char*) malloc(strlen(CERT_PATH) + strlen(SSL_CERTFILE) + 1);
	sprintf(certFile, "%s/%s", CERT_PATH, SSL_CERTFILE);

	if (0 < matrixSslReadKeys(&sslKeys, 
				certFile, 
				privFile, 
				NULL /* privPass */,  
				NULL /* trustedCAFile */)) {
		fprintf(stderr, "failed to read certificates in websSSLOpen\n");
		free(certFile);
		free(privFile);
		return -1;
	}
	free(certFile);
	free(privFile);

	/* lumentis */
	sslListenSock = socketOpenConnection(NULL, SSL_PORT, websSSLAccept, SOCKET_BLOCK); 

	if (sslListenSock < 0) {
		/* trace(2, T("SSL: Unable to open SSL socket on port <%d>!\n"), sslPort); */
		fprintf(stderr, "SSL: Unable to open SSL socket on port <%d>!\n", SSL_PORT);
		return -1;
	}

	return 0;
}


/******************************************************************************/
/*
 *    Accept a connection
 */

int websSSLAccept(int sid, char *ipaddr, int port, int listenSid)
{
	webs_t    wp;
	int        wid;

	a_assert(ipaddr && *ipaddr);
	a_assert(sid >= 0);
	a_assert(port >= 0);

	/*
	 *    Allocate a new handle for this accepted connection. This will allocate
	 *    a webs_t structure in the webs[] list
	 */
	if ((wid = websAlloc(sid)) < 0) {
		return -1;
	}
	wp = webs[wid];
	a_assert(wp);
	wp->listenSid = listenSid;

	ascToUni(wp->ipaddr, ipaddr, min(sizeof(wp->ipaddr), strlen(ipaddr)+1));

	/*
	 *    Check if this is a request from a browser on this system. This is useful
	 *    to know for permitting administrative operations only for local access
	 */
	if (gstrcmp(wp->ipaddr, T("127.0.0.1")) == 0 ||
			gstrcmp(wp->ipaddr, websIpaddr) == 0 ||
			gstrcmp(wp->ipaddr, websHost) == 0) {
		wp->flags |= WEBS_LOCAL_REQUEST;
	}
	/*
	 *    Since the acceptance came in on this channel, it must be secure
	 */
	wp->flags |= WEBS_SECURE;

	/*
	 *    Arrange for websSocketEvent to be called when read data is available
	 */
	socketCreateHandler(sid, SOCKET_READABLE, websSSLSocketEvent, (int) wp);

	/*
	 *    Arrange for a timeout to kill hung requests
	 */
	wp->timeout = emfSchedCallback(WEBS_TIMEOUT, websTimeout, (void *) wp);
	trace(8, T("websSSLAccept(): webs: accept request\n"));
	return 0;
}


/******************************************************************************/
/*
 *    Perform a read of the SSL socket
 */

int websSSLRead(websSSL_t *wsp, char_t *buf, int len)
{
	int numBytesReceived;
	int status;

	a_assert(wsp);
	a_assert(buf);

	numBytesReceived = sslRead(wsp->sslConn, buf, len, &status);
	if (numBytesReceived == 0 && status == SSLSOCKET_EOF) {
		numBytesReceived = -1;
	}

	return numBytesReceived;
}


/******************************************************************************/
/*
 *    Perform a gets of the SSL socket, returning an balloc'ed string
 *
 *    Get a string from a socket. This returns data in *buf in a malloced string
 *    after trimming the '\n'. If there is zero bytes returned, *buf will be set
 *    to NULL. If doing non-blocking I/O, it returns -1 for error, EOF or when
 *    no complete line yet read. If doing blocking I/O, it will block until an
 *    entire line is read. If a partial line is read socketInputBuffered or
 *    socketEof can be used to distinguish between EOF and partial line still
 *    buffered. This routine eats and ignores carriage returns.
 */

int    websSSLGets(websSSL_t *wsp, char_t **buf)
{
	int status; 
	socket_t    *sp;
	ringq_t        *lq;
	char        c;
	int            len;
	webs_t      wp;
	int         sid;
	int         numBytesReceived;

	a_assert(wsp);
	a_assert(buf);

	*buf = NULL;

	wp  = wsp->wp;
	sid = wp->sid;

	if ((sp = socketPtr(sid)) == NULL) {
		return -1;
	}
	lq = &sp->lineBuf;

	while (1) {

		/* read one byte at a time */
		numBytesReceived = sslRead(wsp->sslConn, &c, 1, &status);
		if (numBytesReceived == 0 && status == SSLSOCKET_EOF) {
			numBytesReceived = -1;
		}

		if (0 > numBytesReceived) {
			return -1;
		}

		if (numBytesReceived == 0) {
			/*
			 * If there is a partial line and we are at EOF, pretend we saw a '\n'
			 */
			if (ringqLen(lq) > 0 && (sp->flags & SOCKET_EOF)) {
				c = '\n';
			} else {
				continue;
			}
		}
		/*
		 * If a newline is seen, return the data excluding the new line to the
		 * caller. If carriage return is seen, just eat it.
		 */
		if (c == '\n') {
			len = ringqLen(lq);
			if (len > 0) {
				*buf = ballocAscToUni((char *)lq->servp, len);
			} else {
				*buf = NULL;
			}
			ringqFlush(lq);
			return len;

		} else if (c == '\r') {
			continue;
		}
		ringqPutcA(lq, c);
	}
	return 0;
}



/******************************************************************************/
/*
 *    The webs socket handler.  Called in response to I/O. We just pass control
 *    to the relevant read or write handler. A pointer to the webs structure
 *    is passed as an (int) in iwp.
 */

static void websSSLSocketEvent(int sid, int mask, int iwp)
{
	webs_t    wp;

	wp = (webs_t) iwp;
	a_assert(wp);

	if (! websValid(wp)) {
		return;
	}

	if (mask & SOCKET_READABLE) {
		websSSLReadEvent(wp);
	}
	if (mask & SOCKET_WRITABLE) {
		if (wp->writeSocket) {
			(*wp->writeSocket)(wp);
		}
	}
}


/******************************************************************************/
/*
 *    Handler for SSL Read Events
 */

static int websSSLReadEvent (webs_t wp)
{
	int ret = 07, sock;
	socket_t *sptr;
	sslConn_t* sslConn;

	a_assert (wp);
	a_assert(websValid(wp));

	sptr = socketPtr(wp->sid);
	a_assert(sptr);

	sock = sptr->sock;

	if (0 > sslAccept(&sslConn, sock, sslKeys, NULL, 0)) {
		/* tbd debug */
		websTimeoutCancel(wp);
		socketCloseConnection(wp->sid);
		websFree(wp);
		return -1;
	}

	/*
	 *    Create the SSL data structure in the wp.
	 */
	wp->wsp = balloc(B_L, sizeof(websSSL_t));
	a_assert (wp->wsp);

	(wp->wsp)->sslConn = sslConn;
	(wp->wsp)->wp = wp;

	/*
	 *    Call the default Read Event
	 */
	websReadEvent(wp);

	return ret;
}


/******************************************************************************/
/*
 *    Return TRUE if websSSL has been opened
 */

int websSSLIsOpen()
{
	return (sslListenSock != -1);
}


/******************************************************************************/
/*
 *    Perform a write to the SSL socket
 */

int websSSLWrite(websSSL_t *wsp, char_t *buf, int len)
{
	int status;
	int sslBytesSent = 0;

	a_assert(wsp);
	a_assert(buf);

	if (wsp == NULL) {
		return -1;
	}

	/* send on socket */
	while ((sslBytesSent = sslWrite(wsp->sslConn, buf, len, &status)) == 0)  
		;

	if (0 > sslBytesSent) 
		sslBytesSent = -1;

	return sslBytesSent;
}


/******************************************************************************/
/*
 *    Return Eof for the underlying socket
 */

int websSSLEof(websSSL_t *wsp)
{
	webs_t      wp;
	int         sid;

	a_assert(wsp);

	wp  = wsp->wp;
	sid = wp->sid;

	return socketEof(sid);
}


/******************************************************************************/
/*
 *   Flush stub for compatibility
 */
int websSSLFlush(websSSL_t *wsp)
{
	a_assert(wsp);

	/* Autoflush - do nothing */
	return 0;
}


/******************************************************************************/
/*
 *    Free SSL resources
 */

int websSSLFree(websSSL_t *wsp)
{
	int status = 0;
	if (NULL != wsp)
	{
		/* tbd alert the other side that the connection is being closed? */
		/* void sslWriteClosureAlert(sslConn_t *cp) */

		/* close the session */
		sslFreeConnection(&wsp->sslConn);

		/* Free memory here.... */
		bfree(B_L, wsp);
	}

	return status;
}


/******************************************************************************/
/*
 *    Stops the SSL system
 */

void websSSLClose()
{
	matrixSslFreeKeys(sslKeys);
	matrixSslClose();
}


/******************************************************************************/

#endif /* MATRIX_SSL */
