#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16640);
 script_version ("$Revision: 1.4 $");

 name["english"] = "HP-UX Security patch : PHSS_29626";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHSS_29626 .
(SSRT3617 Potential security vulnerability in OpenView DCE (rev.2))

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/11.X/PHSS_29626
See also : HPUX security bulletin 274
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHSS_29626";
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

if ( ! hpux_check_ctx ( ctx:"700:11.00 800:11.11 700:11.11 800:11.00 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHSS_29626 PHSS_29643 PHSS_30204 PHSS_30548 PHSS_31006 PHSS_32099 PHSS_32893 PHSS_33692 PHSS_34381 PHSS_35072 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"OVOPC-CLT.OVOPC-LIN-CLT", version:"A.07.10") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OVOPC-CLT.OVOPC-LIN-CLT", version:"A.07.10") )
{
 security_hole(0);
 exit(0);
}
