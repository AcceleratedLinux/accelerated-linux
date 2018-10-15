# This script was written by Michel Arboi <arboi@alussinan.org>
#
# GPL
#

if(description)
{
 script_id(11096);
 script_bugtraq_id(3901);
 script_version ("$Revision: 1.10 $");
 script_cve_id("CVE-2002-0134");
 name["english"] = "Avirt gateway insecure telnet proxy";
 script_name(english:name["english"]);
 
 desc["english"] = "
It was possible to connect to the remote telnet server without
password and to get a command prompt with the 'DOS' command.

An attacker may use this flaw to get access on your system.

Solution : Contact your vendor for a patch or disable this service
Risk factor : High";

 desc["francais"] = "
Il s'est av�r� possible de se connecter au serveur telnet distant
sans mot de passe et d'obtenir un interpr�teur de commande avec 
l'ordre 'DOS'.

Un pirate peut utiliser cette faille pour prendre pied sur votre
syst�me.

Solution : contactez votre vendeur pour un patch ou 
	d�sactivez ce service
	   
Facteur de risque : Elev�";
	 
 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Remote system compromise through insecure telnet proxy";
 summary["francais"] = "prise de contr�le � distance � travers le relais telnet d�faillant";
 script_summary(english:summary["english"],
 		francais:summary["francais"]);
 
 script_category(ACT_ATTACK);
 
 script_copyright(english:"This script is Copyright (C) 2002 Michel Arboi");
 family["english"] = "Gain root remotely";
 family["francais"] = "Passer root � distance";
 script_family(english:family["english"], francais:family["francais"]);
 script_require_ports("Services/telnet", 23);
 script_dependencies("find_service.nes");
 exit(0);
}

#
# The script code starts here
#
include('telnet_func.inc');
port = get_kb_item("Services/telnet");
if(!port)port = 23;
if (!get_port_state(port))  exit(0);

soc = open_sock_tcp(port);
if (!soc) exit(0);

banner = telnet_negotiate(socket:soc);
cmd = string("dos\r\n");
send(socket:soc, data:cmd);
res = recv(socket: soc, length: 512);

close(soc);
flag = egrep(pattern:"^[A-Z]:\\.*>", string: res);
if (flag) security_hole(port);
