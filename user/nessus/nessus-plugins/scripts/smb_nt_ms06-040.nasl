#
# (C) Tenable Network Security
#

if(description)
{
 script_id(22182);
 script_bugtraq_id(19409);
 script_version("$Revision: 1.1 $");
 script_cve_id("CVE-2006-3439");

 name["english"] = "Vulnerability in Server Service Could Allow Remote Code Execution (921883)";

 script_name(english:name["english"]);
 
 desc["english"] = "
Synopsis :

Arbitrary code can be executed on the remote host due to a flaw in the 
'server' service.

Description :

The remote host is vulnerable to a buffer overrun in the 'Server' service
which may allow an attacker to execute arbitrary code on the remote host
with the 'System' privileges.

Solution : 

Microsoft has released a set of patches for Windows 2000, XP and 2003 :

http://www.microsoft.com/technet/security/bulletin/ms06-040.mspx

Risk factor : 

Critical / CVSS Base Score : 10
(AV:R/AC:L/Au:NR/C:C/A:C/I:C/B:N)";


 script_description(english:desc["english"]);
 
 summary["english"] = "Determines the presence of update 921883";

 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "Windows : Microsoft Bulletins";
 script_family(english:family["english"]);
 
 script_dependencies("smb_hotfixes.nasl");
 script_require_keys("SMB/Registry/Enumerated");
 script_require_ports(139, 445);
 exit(0);
}


include("smb_hotfixes_fcheck.inc");
include("smb_hotfixes.inc");
include("smb_func.inc");


if ( hotfix_check_sp(xp:3, win2003:2, win2k:6) <= 0 ) exit(0);

if (is_accessible_share())
{
 if ( hotfix_is_vulnerable (os:"5.2", sp:0, file:"Netapi32.dll", version:"5.2.3790.559", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.2", sp:1, file:"Netapi32.dll", version:"5.2.3790.2747", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:1, file:"Netapi32.dll", version:"5.1.2600.1874", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:2, file:"Netapi32.dll", version:"5.1.2600.2952", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.0", file:"Netapi32.dll", version:"5.0.2195.7105", dir:"\system32") )
   security_hole (get_kb_item("SMB/transport"));
 
 hotfix_check_fversion_end(); 
 exit (0);
}
else if ( hotfix_missing(name:"921883") > 0 )
	 security_hole(get_kb_item("SMB/transport"));


