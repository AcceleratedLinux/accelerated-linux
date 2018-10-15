/* 
 *	websSSL.h -- SSL Patch header
 *
 * Copyright (c) GoAhead Software Inc., 1992-2000. All Rights Reserved.
 *
 *	See the file "license.txt" for information on usage and redistribution
 *
 *	$Id: websSSL.h,v 1.4 2007-10-24 05:01:44 winfred Exp $
 */

#ifndef _h_websSSL
#define _h_websSSL 1

/******************************** Description *********************************/

/* 
 *	Header file for the GoAhead Patch for SSL. This defines the interface to 
 *	integrate SSL into the GoAhead Webserver.
 */

/********************************* Includes ***********************************/


#ifdef WEBS_SSL_SUPPORT

#ifdef OPENSSL
#define SSLEAY		/* turn off a few special case MONOLITH macros */
#define USE_SOCKETS /* needed for the _O_BINARY defs in the MS world */
#include <openssl/ssl.h>
#include <sslc.h>
#endif /* OPENSSL */

#ifdef MATRIX_SSL
#include "sslSocket.h"
#endif /* MATRIX_SSL */

#ifdef RSA_SSL
#include <sslc.h>
#endif /* RSA_SSL */

#ifndef UEMF
	#include	"basic/basic.h"
	#include	"emf/emf.h"
#else
	#include	"uemf.h"
#endif

#include "webs.h"

/********************************** Defines ***********************************/

typedef struct {
#ifdef __ENABLE_MOCANA_SSL_SERVER__ 
  int mocanaConnectionInstance;
  webs_t      wp;
#endif /* __ENABLE_MOCANA_SSL_SERVER__  */

#ifdef MATRIX_SSL
  sslConn_t* sslConn;
  struct websRec* wp;
#endif /* MATRIX_SSL */

#if defined(OPENSSL) || defined(RSA_SSL)
  SSL	*ssl;
  BIO	*bio;
#endif /* defined(OPENSSL) || defined(RSA_SSL) */
} websSSL_t;


/******************************** Prototypes **********************************/

extern int	websSSLOpen();
extern int	websSSLIsOpen();
extern void websSSLClose();

extern int	websSSLWrite(websSSL_t *wsp, char_t *buf, int nChars);
extern int	websSSLGets(websSSL_t *wsp, char_t **buf);
extern int	websSSLRead(websSSL_t *wsp, char_t *buf, int nChars);
extern int	websSSLEof(websSSL_t *wsp);

extern int	websSSLFree(websSSL_t *wsp);
extern int	websSSLFlush(websSSL_t *wsp);

extern int	websSSLSetKeyFile(char_t *keyFile);
extern int	websSSLSetCertFile(char_t *certFile);

#endif /* WEBS_SSL_SUPPORT */

#endif /* _h_websSSL */

/*****************************************************************************/

