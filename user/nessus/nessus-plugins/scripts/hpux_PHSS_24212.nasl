#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(17463);
 script_version ("$Revision: 1.5 $");

 name["english"] = "HP-UX Security patch : PHSS_24212";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHSS_24212 .
(Sec. Vulnerability in VVOS mkacct)

Solution : ftp://ftp.itrc.hp.com/superseded_patches/hp-ux_patches/s700_800/11.X/PHSS_24212
See also : HPUX security bulletin 161
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHSS_24212";
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

if ( hpux_patch_installed (patches:"PHSS_24212 PHSS_24527 PHSS_27477 PHSS_28111 PHSS_28521 PHSS_29541 PHSS_29892 PHSS_30159 PHSS_30647 PHSS_31827 PHSS_32141 PHSS_34171 PHSS_35104 PHSS_35306 PHSS_35458 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"VaultTS.VV-CORE-CMN", version:"A.04.50") )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"VaultTS.VVOS-ADM-RUN", version:"A.04.50") )
{
 security_hole(0);
 exit(0);
}
