#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16632);
 script_version ("$Revision: 1.2 $");

 name["english"] = "HP-UX Security patch : PHCO_24723";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHCO_24723 .
(SSRT3496 Potential Security Vulnerability in rexec)

Solution : ftp://ftp.itrc.hp.com/patches_with_warnings/hp-ux_patches/s700_800/11.X/PHCO_24723
See also : HPUX security bulletin 257
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHCO_24723";
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

if ( hpux_patch_installed (patches:"PHCO_24723 PHCO_25707 PHCO_25976 PHCO_27608 PHCO_27774 PHCO_27731 PHCO_28425 PHCO_29284 PHCO_29633 PHCO_29956 PHCO_32448 PHCO_33609 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"OS-Core.C-MIN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OS-Core.C-MIN-64ALIB", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OS-Core.CORE-64SLIB", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OS-Core.CORE-SHLIBS", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"ProgSupport.PROG-AUX", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"ProgSupport.PROG-AX-64ALIB", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"ProgSupport.PROG-MIN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
