# This script was written by Michel Arboi <arboi@alussinan.org>
#
# GPL
#
# References:
# From: "Michal Zalewski" <lcamtuf@echelon.pl>
# To: bugtraq@securityfocus.com
# CC: sendmail-security@sendmail.org
# Subject: RAZOR advisory: multiple Sendmail vulnerabilities

if(description)
{
 script_id(11086);
 script_bugtraq_id(3377);
 script_cve_id("CVE-2001-0713");
 script_version ("$Revision: 1.8 $");
 
 name["english"] = "Sendmail custom configuration file";
 name["francais"] = "Fichier de configuration sp�cifique de sendmail";
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "
The remote sendmail server, according to its version number,
may be vulnerable to a 'Mail System Compromise' when a
user supplies a custom configuration file.
Although the mail server is suppose to run as a lambda user, 
a programming error allows the local attacker to regain the extra 
dropped privileges and run commands as root.

Solution : upgrade to the latest version of Sendmail
Risk factor : High
Note : This vulnerability is _local_ only"; 


 desc["francais"] = "
Le serveur sendmail distant, d'apr�s son num�ro de version,
est vuln�rable lorsqu'un utilisateur fournit un fichier de 
configuration sp�cifique.
Bien que le serveur soit cens� tourner sous une identit� lambda,
une erreur de programmation permet � l'attaquant local de regagner 
les privil�ges abandonn�s et d'ex�cuter des commandes en tant que root.

Solution : mettez � jour sendmail
Facteur de risque : Elev�
Note : cette vuln�rabiit� est locale uniquement";

 script_description(english:desc["english"],
 	 	    francais:desc["francais"]);
		    
 
 summary["english"] = "Checks the version number for 'custom config file'"; 
 summary["francais"] = "V�rification du num�ro de s�rie de sendmail pour l'attaque 'fichier de configuration sp�cifique'";
 script_summary(english:summary["english"],
 		 francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2002 Michel Arboi",
 		  francais:"Ce script est Copyright (C) 2002 Michel Arboi");
 
 family["english"] = "SMTP problems";
 family["francais"] = "Probl�mes SMTP";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes","smtpserver_detect.nasl");
 script_require_keys("SMTP/sendmail");
 script_require_ports("Services/smtp", 25);
 exit(0);
}

#

include("smtp_func.inc");

port = get_kb_item("Services/smtp");
if(!port) port = 25;

banner = get_smtp_banner(port: port);
if(! banner || "Switch-" >< banner ) exit(0);

if(egrep(pattern:".*Sendmail.*8\.12\.0.*", string:banner))
 	security_hole(port);
