#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2004:130
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(15698);
 script_version ("$Revision: 1.3 $");
 script_cve_id("CVE-2004-0834");
 
 name["english"] = "MDKSA-2004:130: speedtouch";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2004:130 (speedtouch).



The Speedtouch USB driver contains a number of format string vulnerabilities
due to improperly made syslog() system calls. These vulnerabilities can be
abused by a local user to potentially allow the execution of arbitray code with
elevated privileges.

The updated packages have been patched to prevent this problem.



Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2004:130
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the speedtouch package";
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
if ( rpm_check( reference:"speedtouch-1.2-2.beta3.2.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"speedtouch-1.3-8.1.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"speedtouch-1.2-1.1.92mdk", release:"MDK9.2", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"speedtouch-", release:"MDK10.0")
 || rpm_exists(rpm:"speedtouch-", release:"MDK10.1")
 || rpm_exists(rpm:"speedtouch-", release:"MDK9.2") )
{
 set_kb_item(name:"CVE-2004-0834", value:TRUE);
}
