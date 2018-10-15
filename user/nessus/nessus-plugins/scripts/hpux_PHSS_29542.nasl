#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(17500);
 script_version ("$Revision: 1.6 $");

 name["english"] = "HP-UX Security patch : PHSS_29542";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHSS_29542 .
(SSRT3642 Potential Security Vulnerabilities Apache web server HP-UX VVOS and Web)

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/11.X/PHSS_29542
See also : HPUX security bulletin 285
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHSS_29542";
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

if ( ! hpux_check_ctx ( ctx:"800:11.04 700:11.04 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHSS_29542 PHSS_29893 PHSS_30153 PHSS_30643 PHSS_30946 PHSS_31825 PHSS_32139 PHSS_32206 PHSS_34170 PHSS_35105 PHSS_35307 PHSS_35459 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"VaultTS.VV-IWS", version:"A.04.60") )
{
 security_hole(0);
 exit(0);
}
