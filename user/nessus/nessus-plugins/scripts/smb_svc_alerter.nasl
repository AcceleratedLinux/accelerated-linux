#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10457);
 script_version ("$Revision: 1.10 $");
 script_cve_id("CVE-1999-0630"); 

 name["english"] = "The alerter service is running";
 name["francais"] = "Le service 'avertissement' tourne";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The alerter service is running. This service allows
NT users to send pop-ups messages to each others.

This service can be abused by an attacker who can
trick valid users into doing some actions that may
harm their accounts or your network (social
engineering attack)

Solution : Disable this service.

Risk factor : Low

How to disable this service under NT 4 : 
    - open the 'Services' control panel
    - select the 'Alerter' service, and click 'Stop'
    - click on 'Startup...' and change to radio button of the
      field 'Startup Type' from 'Automatic' to 'Disabled'
    
Under Windows 2000 :
    - open the 'Administration tools' control panel
    - open the 'Services' item in it
    - double click on the 'Alerter' service
    - click on 'stop'
    - change the drop-down menu value from the field 'Startup Type'
      from 'Automatic' to 'Disabled'
";
    
    

 desc["francais"] = "
Le service 'Avertissement' tourne. Ce service permet 
aux utilisateurs de s'envoyer des messages entre eux,
d'une machine � l'autre, par le biais de pop-up.

Un pirate peut abuser de ce service de telle sorte qu'il
dupe un utilisateur normal, en se faisant passer pour
quelqu'un d'autre, afin de lui faire faire certaines actions
dangereuses pour son compte ou votre r�seau (attaque
par social engineering)

Solution : d�sactivez-le
Facteur de risque : Faible

Pour d�sactiver ce service sous NT 4 :
   - ouvrez le panneau de controle 'Service'
   - s�lectionnez l'�l�ment 'Avertissement'
   - clickez sur 'Arreter'
   - clickez sur 'D�marage', puis changez la valeur du
     bouton radio, dans le champ 'Type de d�marrage',
     en la valeur 'D�sactiv�'

Sous Windows 2000 :
    - ouvrez le panneau de controle 'Outils d'administration'
    - s�lectionnez l'�l�ment 'Avertissement'
    - clickez sur 'Arreter'
    - changez la valeur du menu drop-down dans le champ
      'Type de d�marrage' en 'D�sactiv�'
      
";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of the alerter service";
 summary["francais"] = "V�rifie la pr�sence du service avertissement";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Windows";
 family["francais"] = "Windows";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("smb_enum_services.nasl");
 script_require_keys("SMB/svcs");
 exit(0);
}

#
# The script code starts here
#

port = get_kb_item("SMB/transport");
if(!port)port = 139;

services = get_kb_item("SMB/svcs");

if(services)
{
 if("[Alerter]" >< services)security_warning(port);
}
