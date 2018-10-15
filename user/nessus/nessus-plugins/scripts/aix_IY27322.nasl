#
# (C) Tenable Network Security
#
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(22388);
 script_version ("$Revision: 1.1 $");
 name["english"] = "AIX 5.1 : IY27322";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing AIX Critical Security Patch number IY27322
(SECURITY: Environment variable name string too short.).

You should install this patch for your system to be up-to-date.

Solution : http://www-912.ibm.com/eserver/support/fixes/ 
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "Check for patch IY27322"; 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "AIX Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/AIX/lslpp");
 exit(0);
}



include("aix.inc");

 if( aix_check_patch(release:"5.1", patch:"IY27322", package:"bos.rte.security.5.1.0.17") < 0 ) 
   security_hole(port);
