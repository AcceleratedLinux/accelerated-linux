#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2006:085
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(21360);
 script_version ("$Revision: 1.1 $");
 script_cve_id("CVE-2006-1905");
 
 name["english"] = "MDKSA-2006:085: xine-ui";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2006:085 (xine-ui).



Multiple format string vulnerabilities in xiTK (xitk/main.c) in xine allow

remote attackers to execute arbitrary code via format string specifiers in

a long filename on an EXTINFO line in a playlist file.



Packages have been patched to correct this issue.



Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2006:085
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the xine-ui package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "Mandrake Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Mandrake/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"xine-ui-0.99.4-1.1.20060mdk", release:"MDK2006.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"xine-ui-aa-0.99.4-1.1.20060mdk", release:"MDK2006.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"xine-ui-fb-0.99.4-1.1.20060mdk", release:"MDK2006.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"xine-ui-", release:"MDK2006.0") )
{
 set_kb_item(name:"CVE-2006-1905", value:TRUE);
}
