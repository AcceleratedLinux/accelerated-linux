#
# This script was written by Renaud Deraison
#
#
# See the Nessus Scripts License for details
#

account = "gamez";
password = "lrkr0x";

if(description)
{
 script_id(11263);
 script_version ("$Revision: 1.11 $");
 script_cve_id("CVE-1999-0502");
 
 script_name(english:"Default password (lrkr0x) for gamez");

 desc["english"] = "
The account 'gamez' has the password 'lrkr0x'
An attacker may use it to gain further privileges on this system

Risk factor : High
Solution : Set a strong password for this account or disable it.
This may disable dependant applications so beware";

 script_description(english:desc["english"]);
 script_summary(english:"Logs into the remote host");

 script_category(ACT_GATHER_INFO);

 script_family(english:"Default Unix Accounts");
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison");
 
 
 script_dependencie("find_service.nes", "ssh_detect.nasl");
 script_require_ports("Services/telnet", 23, "Services/ssh", 22);
 script_require_keys("Settings/ThoroughTests");
 exit(0);
}

#
# The script code starts here : 
#
include("default_account.inc");
include("global_settings.inc");
if ( ! thorough_tests ) exit(0);

port = check_account(login:account, password:password);
if(port)security_hole(port);
