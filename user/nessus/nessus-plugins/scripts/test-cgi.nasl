#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10282);
 script_bugtraq_id(2003);
 script_version ("$Revision: 1.23 $");
 script_cve_id("CVE-1999-0070");
 
 name["english"] = "test-cgi";
 name["francais"] = "test-cgi";
 name["portugues"] = "test-cgi";
 script_name(english:name["english"], francais:name["francais"], portugues:name["portugues"]);
 
 desc["english"] = "The 'test-cgi' cgi is installed. This CGI has
a well known security flaw that lets an attacker read arbitrary
files with the privileges of the http daemon (usually root or nobody).

Solution : remove it from /cgi-bin.

Risk factor : High";


 desc["francais"] = "Le cgi 'test-cgi' est install�. Celui-ci poss�de
un probl�me de s�curit� bien connu qui permet � n'importe qui de faire
lire des fichiers arbitraires au daemon http, avec les privil�ges
de celui-ci (root ou nobody). 

Solution : retirez-le de /cgi-bin.

Facteur de risque : S�rieux";


 desc["portugues"] = "O cgi 'test-cgi' est� instalado. Este CGI tem
uma falha de seguran�a bem conhecida que permite a qualquer um ler
arquivos arbitr�rios com o privil�gio do daemon http (root ou nobody).

Solu��o : Remov�-lo do /cgi-bin.

Fator de risco : S�rio";


 script_description(english:desc["english"], francais:desc["francais"],
 		portugues:desc["portugues"]);
 
 summary["english"] = "Checks for the presence of /cgi-bin/test-cgi";
 summary["francais"] = "V�rifie la pr�sence de /cgi-bin/test-cgi";
 summary["portugues"] = "Verifica a presen�a de /cgi-bin/test-cgi";
 
 script_summary(english:summary["english"], francais:summary["francais"],
 		portugues:summary["portugues"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison",
		portugues:"Este script � Copyright (C) 1999 Renaud Deraison");

 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 family["portugues"] = "Abusos de CGI";
 script_family(english:family["english"], francais:family["francais"],
 		portugues:family["portugues"]);

 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 script_exclude_keys("Settings/disable_cgi_scanning");
 exit(0);
}

include("http_func.inc");
include("http_keepalive.inc");

#
# The script code starts here
#

function check(url)
{ 
 url = string(url, "/test-cgi?/*");
 req = http_get(item:url, port:port);
 buf = http_keepalive_send_recv(port:port, data:req);
 if( buf == NULL ) exit(0);
 
 if (("/root" >< buf) && ("/usr" >< buf) && ("/sbin" >< buf) && ("/tmp" >< buf))
   {
    security_hole(port);
    exit(0);
   }
}


port = get_http_port(default:80);


if(!get_port_state(port))exit(0);

foreach dir (cgi_dirs())
{
check(url:dir);
}
