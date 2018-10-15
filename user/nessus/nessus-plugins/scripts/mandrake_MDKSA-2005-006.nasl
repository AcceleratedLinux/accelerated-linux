#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2005:006
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16157);
 script_version ("$Revision: 1.3 $");
 script_cve_id("CVE-2004-1182");
 
 name["english"] = "MDKSA-2005:006: hylafax";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2005:006 (hylafax).



Patrice Fournier discovered a vulnerability in the authorization sub-system of
hylafax. A local or remote user guessing the contents of the hosts.hfaxd
database could gain unauthorized access to the fax system.

The updated packages are provided to prevent this issue. Note that the packages
included with Corporate Server 2.1 do not require this fix.



Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2005:006
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the hylafax package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "Mandrake Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Mandrake/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"hylafax-4.1.8-2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"hylafax-client-4.1.8-2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"hylafax-server-4.1.8-2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libhylafax4.1.1-4.1.8-2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libhylafax4.1.1-devel-4.1.8-2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"hylafax-4.2.0-1.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"hylafax-client-4.2.0-1.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"hylafax-server-4.2.0-1.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libhylafax4.2.0-4.2.0-1.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libhylafax4.2.0-devel-4.2.0-1.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"hylafax-", release:"MDK10.0")
 || rpm_exists(rpm:"hylafax-", release:"MDK10.1") )
{
 set_kb_item(name:"CVE-2004-1182", value:TRUE);
}
