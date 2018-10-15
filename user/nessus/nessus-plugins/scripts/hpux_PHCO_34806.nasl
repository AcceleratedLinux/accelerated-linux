#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(22261);
 script_version ("$Revision: 1.1 $");

 name["english"] = "HP-UX Security patch : PHCO_34806";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHCO_34806 .
(SSRT51153 rev.1 - HP-UX in Trusted mode, Local Denial of Service (DoS))

Solution : ftp://ftp.itrc.hp.com/hp-ux_patches/s700_800/11.X/PHCO_34806
See also : HPUX security bulletin 2141
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHCO_34806";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "HP-UX Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/HP-UX/swlist");
 exit(0);
}

include("hpux.inc");

if ( ! hpux_check_ctx ( ctx:"700:11.00 800:11.00 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHCO_34806 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"OS-Core.CORE-SHLIBS", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
