#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Added link to the Bugtraq message archive
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10057);
 script_version ("$Revision: 1.30 $");
 
 name["english"] = "Lotus Domino ?open Vulnerability";
 name["francais"] = "Vuln�rabilit� ?open dans Lotus Domino";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "It is possible to browse the
remote web server directories by appending ?open
at the end of the URL. Like :

	http://www.example.com/?open
	

 Data that can be accessed by unauthorized users 
may include: usernames, server names and IP addresses, 
dial-up server phone numbers, administration logs, files 
names, and data files (including credit card information, 
proprietary corporate data, and other information stored in
eCommerce related databases.)  In some instances, it may 
be possible for an unauthorized user to modify these files 
or perform server administration functions via the web 
administration interface.

Reference : http://online.securityfocus.com/archive/1/10820

Solution :
	Disable the database browsing. To do this :
	1. From the Domino Administrator, click the 
	  Configuration tab, and open the Server 
	  document,
	2. Click the Internet Protocols - HTTP tab,
	3. In the 'Allow HTTP clients to browse databases'
	   field, choose No,
	4. Save the document.
	
Risk factor : High";	
 desc["francais"] = "Il est possible de lister
les r�pertoires du site distant en ajoutant ?open
� la fin de l'url demand�e, comme par exemple :

	http://www.example.com/?open
	
 Les donn�es pouvant ainsi etre lues par des
utilisateurs non-autoris�s peuvent contenir :
des noms d'utilisateurs, des noms de serveurs
et des adresses IP, des num�ros de t�l�phone
des serveurs dial-up, des logs administratifs,
des noms de fichiers et des fichiers
(comportant �ventuellement des num�ros de
cartes de cr�dit, des donn�es d'entreprise
confidentielles, etc...). Dans certains cas,
le pirate pourra m�me modifier ces fichiers
ou accomplir des taches d'administration au
travers de l'interface d'administration web.

Voir aussi :
	
	http://www.l0pht.com/advisories/domino3.txt

Solution : 
	D�sactivez le listage de base de donn�es. 
	Pour ce faire :

	1. A partir de l'administrateur Domino, cliquez
           sur le tableau de configuration, et ouvrez
           le document du server, 
	2. Cliquez sur les protocoles Internet - tableau HTTP,
	3. Dans la partie 'Accepte le listage des donn�es 
           par les clients HTTP', choisissez non,
	4. Sauvegardez le document.	

Facteur de risque : S�rieux.";	
	
 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the domino ?open feature";
 summary["francais"] = "V�rifie la fonctionalit� ?open de domino";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "Remote file access";
 family["francais"] = "Acc�s aux fichiers distants";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "http_version.nasl", "www_fingerprinting_hmap.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");
include("http_keepalive.inc");
port = get_http_port(default:80);


if(!get_port_state(port))exit(0);
sig = get_kb_item("www/hmap/" + port + "/description");
if ( sig && "Lotus Domino" >!< sig ) exit(0);


banner = get_http_banner(port:port);
	
if(egrep(pattern:"Server:.*otus.*", string:banner))
{
 cgi = "/?open";
 ok = is_cgi_installed_ka(item:cgi, port:port);
 if(ok)security_hole(port);
}
