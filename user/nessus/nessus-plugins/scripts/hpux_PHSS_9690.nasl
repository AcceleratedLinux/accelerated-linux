#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(17044);
 script_version ("$Revision: 1.1 $");

 name["english"] = "HP-UX Security patch : PHSS_9690";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHSS_9690 .
(Security vulnerability bypassing proper authentication)

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/10.X/PHSS_9690
See also : HPUX security bulletin 046
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHSS_9690";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "HP-UX Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/HP-UX/swlist");
 exit(0);
}

include("hpux.inc");

if ( ! hpux_check_ctx ( ctx:"800:10.10 700:10.10 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHSS_9690 PHSS_9802 PHSS_11172 PHSS_12137 PHSS_13402 PHSS_13723 PHSS_14594 PHSS_16150 PHSS_23355 PHSS_25136 PHSS_25548 PHSS_25785 PHSS_26488 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"CDE.CDE-RUN", version:NULL) )
{
 security_hole(0);
 exit(0);
}
