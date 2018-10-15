#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16741);
 script_version ("$Revision: 1.3 $");

 name["english"] = "HP-UX Security patch : PHSS_24087";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHSS_24087 .
(Security Vulnerabilities in CDE on HP-UX)

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/11.X/PHSS_24087
See also : HPUX security bulletin 151
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHSS_24087";
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

if ( ! hpux_check_ctx ( ctx:"800:11.11 700:11.11 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHSS_24087 PHSS_25197 PHSS_25789 PHSS_26493 PHSS_27873 PHSS_28677 PHSS_29740 PHSS_30669 PHSS_30789 PHSS_32111 PHSS_32540 PHSS_33326 PHSS_34101 PHSS_35434 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"CDE.CDE-RUN", version:"B.11.11") )
{
 security_hole(0);
 exit(0);
}
