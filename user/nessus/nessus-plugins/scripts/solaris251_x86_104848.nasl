#
# (C) Tenable Network Security
#
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(12807);
 if(defined_func("script_xref"))script_xref(name:"IAVA", value:"2002-t-0008");
 script_version ("$Revision: 1.6 $");
 script_bugtraq_id(4631);
 script_cve_id("CVE-2002-0084");
 name["english"] = "Solaris 2.5.1 (i386) : 104848-09";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing Sun Security Patch number 104848-09
( /kernel/fs/cachefs patch).

You should install this patch for your system to be up-to-date.

Solution : http://sunsolve.sun.com/search/document.do?assetkey=1-21-104848-09-1
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "Check for patch 104848-09"; 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "Solaris Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Solaris/showrev");
 exit(0);
}



include("solaris.inc");

e =  solaris_check_patch(release:"5.5.1_x86", arch:"i386", patch:"104848-09", obsoleted_by:"", package:"SUNWcsr SUNWcsu SUNWhea");

if ( e < 0 ) security_hole(0);
else if ( e > 0 )
{
	set_kb_item(name:"CVE-2002-0084", value:TRUE);
	set_kb_item(name:"BID-4631", value:TRUE);
}
