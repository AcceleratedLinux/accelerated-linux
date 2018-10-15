/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2004 Tenable Network Security
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <includes.h>

#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"

#include "nasl_debug.h"
#include "nasl_socket.h"

#include "nasl_http.h"

#include "strutils.h"

/*-----------------[ http_* functions ]-------------------------------*/
 


  
 

tree_cell * http_open_socket(lex_ctxt * lexic)
{
  return nasl_open_sock_tcp_bufsz(lexic, 65536);
}

tree_cell * http_close_socket(lex_ctxt * lexic)
{ 
 return nasl_close_socket(lexic);
}


static tree_cell * _http_req(lex_ctxt* lexic, char* keyword)
{
 tree_cell * retc;
 char * str;
 char * item = get_str_local_var_by_name(lexic, "item");
 char * data = get_str_local_var_by_name(lexic, "data");
 int    port = get_int_local_var_by_name(lexic, "port", -1);
 char *url = NULL;
 struct arglist * script_infos = lexic->script_infos;
 char *auth, tmp[32];
 int    ver;
 int	cl;
 int    al;
 char	content_l_str[32];
 struct kb_item ** kb;


 if( item == NULL || port < 0)
 {
  nasl_perror(lexic, "Error : http_* functions have the following syntax :\n");
  nasl_perror(lexic, "http_*(port:<port>, item:<item> [, data:<data>]\n" );
  return NULL;
 } 
 
 if (port <= 0 || port > 65535)
 {
   nasl_perror(lexic, "http_req: invalid value %d for port parameter\n", port);
   return NULL;
 }

 kb = plug_get_kb(script_infos);
 snprintf(tmp, sizeof(tmp), "/tmp/http/auth/%d", port);
 auth = kb_item_get_str(kb, tmp);

 if (auth == NULL)
   auth = kb_item_get_str(kb, "http/auth");

 snprintf(tmp, sizeof(tmp), "http/%d", port);
 ver = kb_item_get_int(kb, tmp);

 if (data == NULL)
   {
     cl = 0; *content_l_str = '\0';
   }
 else
   {
     cl = strlen(data);
     snprintf(content_l_str, sizeof(content_l_str), "Content-Length: %d\r\n", cl);
   }

 if( auth != NULL )
    al = strlen(auth);
 else
    al = 0;
    
 if( (ver <= 0) || (ver == 11) )
 {
  char	*hostname, *ua;
 
  hostname = (char*)plug_get_host_fqdn(script_infos);
  if( hostname == NULL ) return NULL;
  ua = kb_item_get_str(kb, "http/user-agent");
 #define NESSUS_USER_AGENT	"Mozilla/4.75 [en] (X11, U; Nessus)"
  if (ua == NULL)
    ua = NESSUS_USER_AGENT;
  else
    {
      while (isspace(*ua))
	ua ++;
      if (*ua == '\0')
	ua = NESSUS_USER_AGENT;
    }

  url = build_encode_URL(script_infos, keyword, NULL, item, "HTTP/1.1");
  str = emalloc(strlen(url) + strlen(hostname) + al + cl + strlen(ua) + 1024);
  /* NIDS evasion */
  sprintf(str, "%s\r\n\
Connection: Close\r\n\
Host: %s\r\n\
Pragma: no-cache\r\n\
User-Agent: %s\r\n\
Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n\
Accept-Language: en\r\n\
Accept-Charset: iso-8859-1,*,utf-8\r\n",
		url, hostname, ua);
 }
 else
 {
   /* NIDS evasion */
   url = build_encode_URL(script_infos, keyword, NULL, item, "HTTP/1.0\r\n");

 str = emalloc(strlen(url) + al + cl + 120);
 strcpy(str, url);
 }
 efree(&url);

 if (auth != NULL)
   {
     strcat(str, auth);
     strcat(str, "\r\n");
   }

 if (data != NULL)
   strcat(str, content_l_str);
 
 strcat(str, "\r\n");

 if (data != NULL)
   {
     strcat(str, data);
   }

 retc = alloc_tree_cell(0, NULL);
 retc->type = CONST_DATA;
 retc->size = strlen(str);
 retc->x.str_val = str;
 return retc;
}
/*
 * Syntax :
 *
 * http_get(port:<port>, item:<item>);
 *
 */
tree_cell * http_get(lex_ctxt * lexic)
{
 return _http_req(lexic, "GET");
}

/*
 * Syntax :
 *
 * http_head(port:<port>, item:<item>);
 *
 */
tree_cell * http_head(lex_ctxt * lexic)
{
 return _http_req(lexic, "HEAD");
}


/*
 * Syntax :
 * http_post(port:<port>, item:<item>)
 */
tree_cell * http_post(lex_ctxt * lexic)
{
 return _http_req(lexic, "POST");
}

/*
 * http_delete(port:<port>, item:<item>)
 */
tree_cell * http_delete(lex_ctxt * lexic)
{
  return _http_req(lexic, "DELETE");
}

/*
 * http_put(port:<port>, item:<item>, data:<data>)
 */
tree_cell * http_put(lex_ctxt * lexic)
{
  return _http_req(lexic, "PUT");
}


/*-------------------[ cgibin() ]--------------------------------*/

 
tree_cell *cgibin(lex_ctxt * lexic)
{
 struct arglist * script_infos = lexic->script_infos;
 struct arglist * prefs = arg_get_value(script_infos, "preferences");
 char * path = prefs == NULL ? NULL : arg_get_value(prefs, "cgi_path");
 tree_cell * retc;

 if(path == NULL) path = "/cgi-bin:/scripts";
 retc = alloc_tree_cell(0, NULL);
 retc->type = CONST_DATA;
 retc->x.str_val = estrdup(path);
 retc->size = strlen(path);
    
 return retc;
}



/*-------------------------------------------------------------------------*/

tree_cell * nasl_is_cgi_installed(lex_ctxt * lexic)
{
 nasl_perror(lexic, "is_cgi_installed() is now deprecated. Use is_cgi_installed_ka() instead\n");
 return NULL;
}

/*--------------------------------------------------------------------------*/

