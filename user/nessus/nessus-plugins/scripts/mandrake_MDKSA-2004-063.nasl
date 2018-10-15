#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2004:063
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(14162);
 script_version ("$Revision: 1.4 $");
 script_cve_id("CVE-2002-1363");
 
 name["english"] = "MDKSA-2004:063: libpng";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2004:063 (libpng).


A buffer overflow vulnerability was discovered in libpng due to a wrong
calculation of some loop offset values. This buffer overflow can lead to Denial
of Service or even remote compromise.
This vulnerability was initially patched in January of 2003, but it has since
been noted that fixes were required in two additional places that had not been
corrected with the earlier patch. This update uses an updated patch to fix all
known issues.
After the upgrade, all applications that use libpng should be restarted. Many
applications are linked to libpng, so if you are unsure of what applications to
restart, you may wish to reboot the system. Mandrakesoft encourages all users to
upgrade immediately.


Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2004:063
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the libpng package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "Mandrake Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Mandrake/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"libpng3-1.2.5-10.3.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-devel-1.2.5-10.3.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-1.2.5-2.3.91mdk", release:"MDK9.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-devel-1.2.5-2.3.91mdk", release:"MDK9.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-static-devel-1.2.5-2.3.91mdk", release:"MDK9.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-1.2.5-7.3.92mdk", release:"MDK9.2", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-devel-1.2.5-7.3.92mdk", release:"MDK9.2", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libpng3-static-devel-1.2.5-7.3.92mdk", release:"MDK9.2", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"libpng-", release:"MDK10.0")
 || rpm_exists(rpm:"libpng-", release:"MDK9.1")
 || rpm_exists(rpm:"libpng-", release:"MDK9.2") )
{
 set_kb_item(name:"CVE-2002-1363", value:TRUE);
}
