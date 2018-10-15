#
# (C) Tenable Network Security
#

if(description)
{
 script_id(21210);
 script_version("$Revision: 1.6 $");
 script_bugtraq_id(17131, 17181, 17196, 17468, 17460, 17457, 17455, 17454, 17453, 17450);
 script_cve_id("CVE-2006-1359", "CVE-2006-1245", "CVE-2006-1388", "CVE-2006-1185", "CVE-2006-1186", "CVE-2006-1188", "CVE-2006-1189", "CVE-2006-1190", "CVE-2006-1191", "CVE-2006-1192");

 name["english"] = "Cumulative Security Update for Internet Explorer (912812)";

 script_name(english:name["english"]);
 
 desc["english"] = "
Synopsis :

Arbitrary code can be executed on the remote host through the web client.

Description :

The remote host is missing the IE cumulative security update 912812.

The remote version of IE is vulnerable to several flaws which may allow an 
attacker to execute arbitrary code on the remote host.


Solution : 

Microsoft has released a set of patches for Windows 2000, XP and 2003 :

http://www.microsoft.com/technet/security/bulletin/ms06-013.mspx

Risk factor : 

High / CVSS Base Score : 9.9
(AV:R/AC:L/Au:NR/C:C/I:C/A:C/B:N)";


 script_description(english:desc["english"]);
 
 summary["english"] = "Determines the presence of update 912812";

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
 if ( hotfix_is_vulnerable (os:"5.2", sp:0, file:"Mshtml.dll", version:"6.0.3790.497", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.2", sp:1, file:"Mshtml.dll", version:"6.0.3790.2666", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:1, file:"Mshtml.dll", version:"6.0.2800.1543", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.1", sp:2, file:"Mshtml.dll", version:"6.0.2900.2873", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.0", file:"Mshtml.dll", version:"6.0.2800.1543", min_version:"6.0.0.0", dir:"\system32") ||
      hotfix_is_vulnerable (os:"5.0", file:"Mshtml.dll", version:"5.0.3839.2200", dir:"\system32") )
   security_hole (get_kb_item("SMB/transport"));
 
 hotfix_check_fversion_end(); 
 exit (0);
}
else
{
if ( ( hotfix_missing(name:"912812") > 0 ) &&
     ( hotfix_missing(name:"916281") > 0 ) &&
     ( hotfix_missing(name:"918899") > 0 ) )
	{
	 minorversion = get_kb_item("SMB/Registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Internet Settings/MinorVersion");
	if ( ( "912812" >!< minorversion ) &&
             ( "916281" >!< minorversion ) &&
             ( "918899" >!< minorversion ) )
          security_hole(get_kb_item("SMB/transport"));
	}

}
