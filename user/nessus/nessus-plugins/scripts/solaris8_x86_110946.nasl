#
# (C) Tenable Network Security
#
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(13464);
 script_version ("$Revision: 1.4 $");
 script_bugtraq_id(7820);
 name["english"] = "Solaris 8 (i386) : 110946-08";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing Sun Security Patch number 110946-08
( /usr/sbin/syslogd patch).

You should install this patch for your system to be up-to-date.

Solution : http://sunsolve.sun.com/search/document.do?assetkey=1-21-110946-08-1
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "Check for patch 110946-08"; 
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

e =  solaris_check_patch(release:"5.8_x86", arch:"i386", patch:"110946-08", obsoleted_by:"", package:"SUNWcsu");

if ( e < 0 ) security_hole(0);
else if ( e > 0 )
{
	set_kb_item(name:"BID-7820", value:TRUE);
}
