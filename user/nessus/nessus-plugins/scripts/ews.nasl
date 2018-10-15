#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10064);
 script_bugtraq_id(2248);
 script_version ("$Revision: 1.18 $");
 script_cve_id("CVE-1999-0279");
 name["english"] = "Excite for WebServers";
 name["francais"] = "Excite for WebServers";
 name["deutsch"] = "Excite for WebServers";
 script_name(english:name["english"], francais:name["francais"], deutsch:name["deutsch"]);
 
 desc["english"] = "The Excite for Webservers is installed. This CGI has
a well known security flaw that lets anyone execute arbitrary
commands with the privileges of the http daemon (root or nobody).

Versions newer than 1.1. are patched.


Solution : if you are running version 1.1 or older, then
upgrade it.

Risk factor : High";


 desc["francais"] = "Excite for Webservers est install�. Celui-ci poss�de
un probl�me de s�curit� bien connu qui permet � n'importe qui de faire
executer des commandes arbitraires au daemon http, avec les privil�ges
de celui-ci (root ou nobody). 

Les versions sup�rieures � 1.1 sont patch�s

Solution : Mettez � jour votre serveur

Facteur de risque : S�rieux";



script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /cgi-bin/ews";
 summary["francais"] = "V�rifie la pr�sence de /cgi-bin/ews";
 summary["deutsch"] = "�berpr�ft die Existenz von /cgi-bin/ews";
 
 script_summary(english:summary["english"], francais:summary["francais"], deutsch:summary["deutsch"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison",
		deutsch:"Dieses Skript ist Copyright gesch�tzt. (C) 1999 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 family["deutsch"] = "CGI Sicherheitsl�cke";
 script_family(english:family["english"], francais:family["francais"], deutsch:family["deutsch"]);
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");
include("http_keepalive.inc");
include("global_settings.inc");

if ( report_paranoia < 2 ) exit(0);

port = get_http_port(default:80);

res = is_cgi_installed_ka(item:"ews/ews/architext_query.pl", port:port);
if(res)security_hole(port);

