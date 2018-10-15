#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10350);
 script_bugtraq_id(2189);
 script_version ("$Revision: 1.13 $");
 script_cve_id("CVE-2000-0138");
 
 name["english"] = "Shaft Detect";
 name["francais"] = "Detection de Shaft";
 
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The remote host appears to be running
Shaft, which is a trojan that can be 
used to control your system or make it 
attack another network (this is 
actually called a distributed denial
of service attack tool)

It is very likely that this host
has been compromised

Solution : Restore your system from backups,
	   contact CERT and your local
	   authorities

Risk factor : Critical";



 desc["francais"] = "
Le systeme distant semble faire tourner
Shaft qui peut etre utilis� pour prendre 
le controle de celui-ci ou pour attaquer un 
autre r�seau (outil de d�ni de service 
distribu�)

Il est tr�s probable que ce systeme a �t�
compromis

Solution : reinstallez votre syst�me � partir
	   des sauvegardes, et contactez le CERT
	   et les autorit�s locales
	   
Facteur de risque : Critique";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Detects the presence of Shaft";
 summary["francais"] = "Detecte la pr�sence de Shaft";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Backdoors";
 family["francais"] = "Backdoors";
 script_family(english:family["english"], francais:family["francais"]);
 script_require_keys("Settings/ThoroughTests");

 
 exit(0);
}

#
# The script code starts here
#


include('global_settings.inc');

if ( islocalhost() ) exit(0);
if ( ! thorough_tests ) exit(0);

shaft_dstport = 18753;
shaft_rctport = 20433;
shaft_scmd = "alive";
shaft_spass = "tijgu";



command = string(shaft_scmd, " ", shaft_spass, " hi 5 1918");


ip  = forge_ip_packet(ip_hl:5, ip_v:4,   ip_off:0,
                     ip_id:9, ip_tos:0, ip_p : IPPROTO_UDP,
                     ip_len : 20, ip_src : this_host(),
                     ip_ttl : 255);
		   
length = 8 + strlen(command);		     
udpip = forge_udp_packet(ip : ip,
		         uh_sport : 1024,    
                         uh_dport : shaft_dstport,
			 uh_ulen : length,
			 data : command);
			 
filter = string("udp and src host ", get_host_ip(), " and dst host ", this_host(), " and dst port ", shaft_rctport);		 
rep = send_packet(udpip, pcap_filter:filter, pcap_active:TRUE);		
	 	
if(!isnull(rep))
{
 dstport = get_udp_element(udp:rep, element:"uh_dport");
 if(dstport == shaft_rctport)security_hole(port:shaft_dstport, protocol:"udp");
}
 



