
/***************************************************************************
 * nsock_ssl.c -- This contains functions that relate somewhat exclusively *
 * to SSL (over TCP) support in nsock.  Where SSL support is incidental,   *
 * it is often in other files where code can be more easily shared between *
 * the SSL and NonSSL paths.                                               *
 *                                                                         *
 ***********************IMPORTANT NSOCK LICENSE TERMS***********************
 *                                                                         *
 * The nsock parallel socket event library is (C) 1999-2006 Insecure.Com   *
 * LLC This library is free software; you may redistribute and/or          *
 * modify it under the terms of the GNU General Public License as          *
 * published by the Free Software Foundation; Version 2.  This guarantees  *
 * your right to use, modify, and redistribute this software under certain *
 * conditions.  If this license is unacceptable to you, Insecure.Com LLC   *
 * may be willing to sell alternative licenses (contact                    *
 * sales@insecure.com ).                                                   *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included Copying.OpenSSL file, and distribute linked      *
 * combinations including the two. You must obey the GNU GPL in all        *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         * 
 * If you received these files with a written license agreement stating    *
 * terms other than the (GPL) terms above, then that alternative license   *
 * agreement takes precedence over this comment.                          *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to fyodor@insecure.org for possible incorporation into the main         *
 * distribution.  By sending these changes to Fyodor or one the            *
 * insecure.org development mailing lists, it is assumed that you are      *
 * offering Fyodor and Insecure.Com LLC the unlimited, non-exclusive right *
 * to reuse, modify, and relicense the code.  Nmap will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).  We also occasionally relicense the    *
 * code to third parties as discussed above.  If you wish to specify       *
 * special license conditions of your contributions, just say so when you  *
 * send them.                                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details (                               *
 * http://www.gnu.org/copyleft/gpl.html ).                                 *
 *                                                                         *
 ***************************************************************************/

/* $Id: nsock_ssl.h 3870 2006-08-25 01:47:53Z fyodor $ */

#ifndef NSOCK_SSL_H
#define NSOCK_SSL_H

#ifdef HAVE_CONFIG_H
#include "nsock_config.h"
#endif

#if HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>


struct sslinfo {
  /* SSL_ERROR_NONE, SSL_ERROR_WANT_CONNECT, SSL_ERROR_WAINT_READ, or
     SSL_ERROR_WANT_WRITE */
  int ssl_desire; 
};

/* This is the SSL information that is global to nsock and not tied
   to any particular connection. */
struct NsockSSLInfo {
  SSL_CTX *ctx; /* The SSL Context (options and such) */
};

/* Initializes Nsock for low security (fast) SSL connections.
 Eventually it will probably have arguments for various attributes
 (such as whether you want the connection to be fast or secure).  It is
 OK to call it multiple times - only the first one will count.  */
void Nsock_SSL_Init();

/* This function returns the Nsock Global SSL information.  You should
   have called Nsock_SSL_Init once before, but this function will take
   care of it if you haven't. */
struct NsockSSLInfo *Nsock_SSLGetInfo();
#endif /* HAVE_OPENSSL */
#endif /* NSOCK_SSL_H */
