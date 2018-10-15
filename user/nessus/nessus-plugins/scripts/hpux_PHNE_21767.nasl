#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16764);
 script_version ("$Revision: 1.2 $");

 name["english"] = "HP-UX Security patch : PHNE_21767";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHNE_21767 .
(Sec. Vulnerability in net.init)

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/11.X/PHNE_21767
See also : HPUX security bulletin 123
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHNE_21767";
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

if ( hpux_patch_installed (patches:"PHNE_21767 PHNE_22397 PHNE_23456 PHNE_24715 PHNE_25423 PHNE_26771 PHNE_27886 PHNE_28538 PHNE_29473 PHNE_32041 PHNE_33395 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"OS-Core.CORE2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"Networking.NET-RUN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"Networking.NET2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"Networking.NMS2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OS-Core.CORE2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"Networking.NET2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"Networking.NMS2-KRN", version:"B.11.00") )
{
 security_hole(0);
 exit(0);
}
