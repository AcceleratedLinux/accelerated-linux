#
# (C) Tenable Network Security
#

if(description)
{
 script_id(21687);
 script_version("$Revision: 1.2 $");

 #script_cve_id("CVE-2006-1313");
 script_bugtraq_id(18359);

 name["english"] = "Vulnerability in Microsoft JScript Could Allow Remote Code Execution (917344)";

 script_name(english:name["english"]);
 
 desc["english"] = "
Synopsis :

Arbitrary code can be executed on the remote host through the web or
email client. 

Description :

The remote host is running a version of Windows which contains a flaw
in JScript. 

An attacker may be able to execute arbitrary code on the remote host
by constructing a malicious JScript and enticing a victim to visit a
web site or view a specially-crafted email message. 

Solution : 

Microsoft has released a set of patches for Windows 2000, XP and 2003 :

http://www.microsoft.com/technet/security/Bulletin/MS06-023.mspx

Risk factor : 

High / CVSS Base Score : 8 
(AV:R/AC:H/Au:NR/C:C/A:C/I:C/B:N)";


 script_description(english:desc["english"]);
 
 summary["english"] = "Determines the presence of update 917344";

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


rootfile = hotfix_get_systemroot();
if(!rootfile) exit(0);

if ( hotfix_check_sp(xp:3, win2003:2, win2k:5) <= 0 ) exit(0);

if (is_accessible_share())
{
 if ( hotfix_is_vulnerable (os:"5.2", sp:0, file:"Jscript.dll", version:"5.6.0.8831", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.2", sp:1, file:"Jscript.dll", version:"5.6.0.8831", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:1, file:"Jscript.dll", version:"5.6.0.8831", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:2, file:"Jscript.dll", version:"5.6.0.8831", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.0", file:"Jscript.dll", version:"5.6.0.8831", min_version:"5.6.0.0", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.0", file:"Jscript.dll", version:"5.1.0.12512", dir:"\system32") )
   security_hole (get_kb_item("SMB/transport"));
 
 hotfix_check_fversion_end(); 
 exit (0);
}
else
{
  if ( hotfix_missing(name:"917344") > 0  )
    security_hole(port);
}
