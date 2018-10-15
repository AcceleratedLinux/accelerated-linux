/* Copyright (C) 2000 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "vio_priv.h"

#ifdef HAVE_OPENSSL

static bool     ssl_algorithms_added    = FALSE;
static bool     ssl_error_strings_loaded= FALSE;
static int      verify_depth = 0;
static int      verify_error = X509_V_OK;

static unsigned char dh512_p[]=
{
  0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
  0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
  0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
  0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
  0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
  0x47,0x74,0xE8,0x33,
};

static unsigned char dh512_g[]={
  0x02,
};

static DH *get_dh512(void)
{
  DH *dh;
  if ((dh=DH_new()))
  {
    dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
    dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);
    if (! dh->p || ! dh->g)
    {
      DH_free(dh);
      dh=0;
    }
  }
  return(dh);
}


static void
report_errors()
{
  unsigned long	l;
  const char*	file;
  const char*	data;
  int		line,flags;

  DBUG_ENTER("report_errors");

  while ((l=ERR_get_error_line_data(&file,&line,&data,&flags)) != 0)
  {
#ifndef DBUG_OFF				/* Avoid warning */
    char buf[200];
    DBUG_PRINT("error", ("OpenSSL: %s:%s:%d:%s\n", ERR_error_string(l,buf),
			 file,line,(flags & ERR_TXT_STRING) ? data : "")) ;
#endif
  }
  DBUG_VOID_RETURN;
}


static int
vio_set_cert_stuff(SSL_CTX *ctx, const char *cert_file, const char *key_file)
{
  DBUG_ENTER("vio_set_cert_stuff");
  DBUG_PRINT("enter", ("ctx=%p, cert_file=%s, key_file=%s",
		       ctx, cert_file, key_file));
  if (cert_file != NULL)
  {
    if (SSL_CTX_use_certificate_file(ctx,cert_file,SSL_FILETYPE_PEM) <= 0)
    {
      DBUG_PRINT("error",("unable to get certificate from '%s'\n",cert_file));
      /* FIX stderr */
      fprintf(stderr,"Error when connection to server using SSL:");
      ERR_print_errors_fp(stderr);
      fprintf(stderr,"Unable to get certificate from '%s'\n", cert_file);
      fflush(stderr);
      DBUG_RETURN(0);
    }
    if (key_file == NULL)
      key_file = cert_file;
    if (SSL_CTX_use_PrivateKey_file(ctx,key_file,
				    SSL_FILETYPE_PEM) <= 0)
    {
      DBUG_PRINT("error", ("unable to get private key from '%s'\n",key_file));
      /* FIX stderr */
      fprintf(stderr,"Error when connection to server using SSL:");
      ERR_print_errors_fp(stderr);
      fprintf(stderr,"Unable to get private key from '%s'\n", cert_file);
      fflush(stderr);      
      DBUG_RETURN(0);
    }

    /*
      If we are using DSA, we can copy the parameters from the private key
      Now we know that a key and cert have been set against the SSL context
    */
    if (!SSL_CTX_check_private_key(ctx))
    {
      DBUG_PRINT("error",
		 ("Private key does not match the certificate public key\n"));
      DBUG_RETURN(0);
    }
  }
  DBUG_RETURN(1);
}


static int
vio_verify_callback(int ok, X509_STORE_CTX *ctx)
{
  char	buf[256];
  X509*	err_cert;
  int	err,depth;

  DBUG_ENTER("vio_verify_callback");
  DBUG_PRINT("enter", ("ok=%d, ctx=%p", ok, ctx));
  err_cert=X509_STORE_CTX_get_current_cert(ctx);
  err=	   X509_STORE_CTX_get_error(ctx);
  depth=   X509_STORE_CTX_get_error_depth(ctx);

  X509_NAME_oneline(X509_get_subject_name(err_cert),buf,sizeof(buf));
  if (!ok)
  {
    DBUG_PRINT("error",("verify error: num: %d : '%s'\n",err,
			X509_verify_cert_error_string(err)));
    if (verify_depth >= depth)
    {
      ok=1;
      verify_error=X509_V_OK;
    }
    else
    {
      verify_error=X509_V_ERR_CERT_CHAIN_TOO_LONG;
    }
  }
  switch (ctx->error) {
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,256);
    DBUG_PRINT("info",("issuer= %s\n",buf));
    break;
  case X509_V_ERR_CERT_NOT_YET_VALID:
  case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    DBUG_PRINT("error", ("notBefore"));
    /*ASN1_TIME_print_fp(stderr,X509_get_notBefore(ctx->current_cert));*/
    break;
  case X509_V_ERR_CERT_HAS_EXPIRED:
  case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    DBUG_PRINT("error", ("notAfter error"));
    /*ASN1_TIME_print_fp(stderr,X509_get_notAfter(ctx->current_cert));*/
    break;
  }
  DBUG_PRINT("exit", ("%d", ok));
  DBUG_RETURN(ok);
}


/************************ VioSSLConnectorFd **********************************/
/*
  TODO:
       Add option --verify to mysql to be able to change verification mode
*/

struct st_VioSSLConnectorFd *
new_VioSSLConnectorFd(const char* key_file,
		      const char* cert_file,
		      const char* ca_file,
		      const char* ca_path,
		      const char* cipher)
{
  int	verify = SSL_VERIFY_NONE;
  struct st_VioSSLConnectorFd* ptr;
  int result;
  DH *dh=NULL; 
  DBUG_ENTER("new_VioSSLConnectorFd");
  DBUG_PRINT("enter",
	     ("key_file=%s, cert_file=%s, ca_path=%s, ca_file=%s, cipher=%s",
	      key_file, cert_file, ca_path, ca_file, cipher));

  if (!(ptr=((struct st_VioSSLConnectorFd*)
	     my_malloc(sizeof(struct st_VioSSLConnectorFd),MYF(0)))))
    DBUG_RETURN(0);

  ptr->ssl_context= 0;
  ptr->ssl_method=  0;
  /* FIXME: constants! */

  if (!ssl_algorithms_added)
  {
    DBUG_PRINT("info", ("todo: OpenSSL_add_all_algorithms()"));
    ssl_algorithms_added = TRUE;
    OpenSSL_add_all_algorithms();
  }
  if (!ssl_error_strings_loaded)
  {
    DBUG_PRINT("info", ("todo:SSL_load_error_strings()"));
    ssl_error_strings_loaded = TRUE;
    SSL_load_error_strings();
  }
  ptr->ssl_method = TLSv1_client_method();
  ptr->ssl_context = SSL_CTX_new(ptr->ssl_method);
  DBUG_PRINT("info", ("ssl_context: %p",ptr->ssl_context));
  if (ptr->ssl_context == 0)
  {
    DBUG_PRINT("error", ("SSL_CTX_new failed"));
    report_errors();
    goto ctor_failure;
  }
  /*
    SSL_CTX_set_options
    SSL_CTX_set_info_callback
   */
  if (cipher)
  {
    result=SSL_CTX_set_cipher_list(ptr->ssl_context, cipher);
    DBUG_PRINT("info",("SSL_set_cipher_list() returned %d",result));
  }
  SSL_CTX_set_verify(ptr->ssl_context, verify, vio_verify_callback);
  if (vio_set_cert_stuff(ptr->ssl_context, cert_file, key_file) == -1)
  {
    DBUG_PRINT("error", ("vio_set_cert_stuff failed"));
    report_errors();
    goto ctor_failure;
  }
  if (SSL_CTX_load_verify_locations( ptr->ssl_context, ca_file,ca_path) == 0)
  {
    DBUG_PRINT("warning", ("SSL_CTX_load_verify_locations failed"));
    if (SSL_CTX_set_default_verify_paths(ptr->ssl_context) == 0)
    {
      DBUG_PRINT("error", ("SSL_CTX_set_default_verify_paths failed"));
      report_errors();
      goto ctor_failure;
    }
  } 

  /* DH stuff */
  dh=get_dh512();
  SSL_CTX_set_tmp_dh(ptr->ssl_context,dh);
  DH_free(dh);

  DBUG_RETURN(ptr);
ctor_failure:
  DBUG_PRINT("exit", ("there was an error"));
  my_free((gptr)ptr,MYF(0));
  DBUG_RETURN(0);
}


/************************ VioSSLAcceptorFd **********************************/
/*
  TODO:
       Add option --verify to mysqld to be able to change verification mode
*/
struct st_VioSSLAcceptorFd*
new_VioSSLAcceptorFd(const char *key_file,
		     const char *cert_file,
		     const char *ca_file,
		     const char *ca_path,
		     const char *cipher)
{
  int verify = (SSL_VERIFY_PEER			|
		SSL_VERIFY_CLIENT_ONCE);
  struct st_VioSSLAcceptorFd* ptr;
  int result;
  DH *dh=NULL; 
  DBUG_ENTER("new_VioSSLAcceptorFd");
  DBUG_PRINT("enter",
	     ("key_file=%s, cert_file=%s, ca_path=%s, ca_file=%s, cipher=%s",
	      key_file, cert_file, ca_path, ca_file, cipher));

  ptr= ((struct st_VioSSLAcceptorFd*)
	my_malloc(sizeof(struct st_VioSSLAcceptorFd),MYF(0)));
  ptr->ssl_context=0;
  ptr->ssl_method=0;
  /* FIXME: constants! */
  ptr->session_id_context= ptr;

  if (!ssl_algorithms_added)
  {
    DBUG_PRINT("info", ("todo: OpenSSL_add_all_algorithms()"));
    ssl_algorithms_added = TRUE;
    OpenSSL_add_all_algorithms();

  }
  if (!ssl_error_strings_loaded)
  {
    DBUG_PRINT("info", ("todo: SSL_load_error_strings()"));
    ssl_error_strings_loaded = TRUE;
    SSL_load_error_strings();
  }
  ptr->ssl_method=  TLSv1_server_method();
  ptr->ssl_context= SSL_CTX_new(ptr->ssl_method);
  if (ptr->ssl_context == 0)
  {
    DBUG_PRINT("error", ("SSL_CTX_new failed"));
    report_errors();
    goto ctor_failure;
  }
  if (cipher)
  {
    result=SSL_CTX_set_cipher_list(ptr->ssl_context, cipher);
    DBUG_PRINT("info",("SSL_set_cipher_list() returned %d",result));
  }
  /* SSL_CTX_set_quiet_shutdown(ctx,1); */
  SSL_CTX_sess_set_cache_size(ptr->ssl_context,128);

  /* DH? */
  SSL_CTX_set_verify(ptr->ssl_context, verify, vio_verify_callback);
  SSL_CTX_set_session_id_context(ptr->ssl_context,
				 (const uchar*) &(ptr->session_id_context),
				 sizeof(ptr->session_id_context));

  /*
    SSL_CTX_set_client_CA_list(ctx,SSL_load_client_CA_file(CAfile));
  */
  if (vio_set_cert_stuff(ptr->ssl_context, cert_file, key_file) == -1)
  {
    DBUG_PRINT("error", ("vio_set_cert_stuff failed"));
    report_errors();
    goto ctor_failure;
  }
  if (SSL_CTX_load_verify_locations( ptr->ssl_context, ca_file, ca_path) == 0)
  {
    DBUG_PRINT("warning", ("SSL_CTX_load_verify_locations failed"));
    if (SSL_CTX_set_default_verify_paths(ptr->ssl_context)==0)
    {
      DBUG_PRINT("error", ("SSL_CTX_set_default_verify_paths failed"));
      report_errors();
      goto ctor_failure;
    }
  }
  /* DH stuff */
  dh=get_dh512();
  SSL_CTX_set_tmp_dh(ptr->ssl_context,dh);
  DH_free(dh);
  DBUG_RETURN(ptr);

ctor_failure:
  DBUG_PRINT("exit", ("there was an error"));
  my_free((gptr) ptr,MYF(0));
  DBUG_RETURN(0);
}
#endif /* HAVE_OPENSSL */
