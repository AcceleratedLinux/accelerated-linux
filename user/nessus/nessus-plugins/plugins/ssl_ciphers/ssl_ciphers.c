/*
 * Check SSL ciphers & certificates
 *
 * This plugin connects to a SSL server, checks its certificate and the
 * available ciphers
 *
 * This plugin was written by Michel Arboi <arboi@alussinan.org>
 */
 
#include "includes.h"

#ifndef ssl_get_cipher_by_char
#define ssl_get_cipher_by_char(ssl,ptr) \
	((ssl)->method->get_cipher_by_char((unsigned char*)ptr))
#endif

#define EN_NAME "SSL Certificate"

/*
#define EN_DESC "\
This plugin connects to a SSL server, and\n\
checks its certificate and the available ciphers.\n\
Weak (export version) ciphers are reported, as well as \n\
self signed certificates or certificates signed by unknown\n\
CAs."
*/
#define EN_DESC "\
This plugin connects to a SSL server, and displays its certificate.\n"


#define COPYRIGHT "(C) 2002 Michel Arboi"
#define SUMMARY "Displays the server certificate"


PlugExport int plugin_init(desc)
     struct arglist * desc;
{
#ifndef HAVE_SSL
  return -1;
#else
  plug_set_id(desc, 10863);
  plug_set_version(desc, "$Revision: 1.15 $");
 
  plug_set_name(desc, EN_NAME, NULL);
  
  plug_set_description(desc, EN_DESC, NULL);
  plug_set_summary(desc, SUMMARY, NULL);
  plug_set_copyright(desc, COPYRIGHT, NULL);
  plug_set_category(desc, ACT_GATHER_INFO);
  plug_set_family(desc, "General", NULL);
  plug_set_dep(desc, "find_service.nes");
  return 0;
#endif
}


static int report_cat(char ** report, int * report_sz, char * msg)
{
 if ( *report == NULL )
 {
   *report_sz = 1024;
   *report = emalloc ( *report_sz );
 }

 if ( strlen(*report) + strlen(msg) + 1 >= *report_sz )
 {
  int new_sz = *report_sz;
  while ( strlen(*report) + strlen(msg) + 1 > new_sz )
  {
	new_sz *= 2;
  }
  *report_sz = new_sz;
  *report = erealloc(*report, *report_sz);
 }

 strcat(*report, msg);

 if ( msg[strlen(msg) - 1] != '\n' )
	strcat(*report, "\n");
}



PlugExport 
int plugin_run(env)
     struct arglist * env;
{
#ifndef HAVE_SSL
  return -1;
#else
  char		*p, *q, *trp_name, *trp0_name;
  int		port, trp, trp0, cnx = -1, bits;
  char		buf[2048], *pbuf, rep[512], *prep;
  SSL_CIPHER		*c = NULL;
  SSL			*ssl = NULL;
  int			weak = 0, medium = 0, strong = 0, null =0, nCiphers = 0;
  int			cert_printed = 0;
  char			*msg;
  X509			*cert;
  BIO			*b;
  BUF_MEM		*bptr;
  int			rejected[3];
  char 			* report = NULL;
  int			report_sz = 0;
  int 			warning = 0;
#ifdef NEW_KB_MGMT
  int type;

  p = plug_get_key(env, "Transport/SSL", &type);
  if ( p == NULL ) 
	return 0;

  if ( type == KB_TYPE_STR )
	port = atoi(p);
  else
	port = (int)p;
#else  
  p  = plug_get_key(env, "Transport/SSL");
  if (p == NULL)
    return 0;
  port = atoi(p);
#endif

  trp0 = plug_get_port_transport(env, port);
  trp0_name = (char*)get_encaps_name(trp0);

  for (trp = NESSUS_ENCAPS_SSLv2; trp <=  NESSUS_ENCAPS_TLSv1; trp ++)
    {
      if (cnx >= 0)
	{
	  close_stream_connection(cnx);
	  cnx = -1;
	}

      if ((cnx = open_stream_connection(env, port, trp, -2)) < 0)
	{
	  rejected[trp - NESSUS_ENCAPS_SSLv2]  = 1;
	  continue;
	}
      rejected[trp - NESSUS_ENCAPS_SSLv2]  = 0;

      if ((ssl = (SSL*)stream_get_ssl(cnx)) == NULL)
	continue;

      trp_name = (char*)get_encaps_name(trp);

      if (! cert_printed)
	{
	  cert = SSL_get_peer_certificate(ssl);
	  if(cert != NULL)
	    {
	      b = BIO_new(BIO_s_mem());
	      if(X509_print(b, cert) > 0)
		{
		  BIO_get_mem_ptr(b, &bptr);
		  msg = emalloc(bptr->length + 1 + 80);
		  snprintf(msg, bptr->length + 1 + 80, "Here is the %s server certificate:\n",
			  trp_name);
		  for (p = msg; *p != '\0'; p ++) /*NOP*/ ;
		  strncpy(p, bptr->data, bptr->length);
		  report_cat(&report, &report_sz, msg);
		  efree(&msg);
		}
	      BIO_free(b);
	      cert_printed ++;
	    }
	}
  }


  
  prep = rep;
  *prep = '\0';

  if (! rejected[trp0 - NESSUS_ENCAPS_SSLv2])
    {
      for (trp = NESSUS_ENCAPS_SSLv2; trp <= NESSUS_ENCAPS_TLSv1; trp ++)
	if (rejected[trp - NESSUS_ENCAPS_SSLv2])
	  {
	    sprintf(prep, "This %s server does not accept %s connections.\n",
		    trp0_name, get_encaps_name(trp));
	    while (*prep != '\0')
	      prep++;
	  }
    }

  for (trp = NESSUS_ENCAPS_SSLv2; trp <= NESSUS_ENCAPS_TLSv1; trp ++)
    if (trp != trp0 && ! rejected[trp - NESSUS_ENCAPS_SSLv2])
      {
	sprintf(prep, "This %s server also accepts %s connections.\n",
		trp0_name, get_encaps_name(trp));
	    while (*prep != '\0')
	      prep++;
      }
  
  if (*rep != '\0')
    report_cat(&report, &report_sz, rep);

  if (cnx >= 0)
    close_stream_connection(cnx);

  if ( report_sz != 0 && report != NULL && report[0] != '\0' )
	{
	 if ( warning != 0 ) post_hole(env,port, report);
	 else post_note(env, port, report);
	}

  return 0;
#endif
}
