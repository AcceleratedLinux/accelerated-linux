#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2005:162
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(19917);
 script_version ("$Revision: 1.3 $");
 script_cve_id("CVE-2005-2794", "CVE-2005-2796");
 
 name["english"] = "MDKSA-2005:162: squid";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2005:162 (squid).



Two vulnerabilities were recently discovered in squid:

The first is a DoS possible via certain aborted requests that trigger an
assertion error related to 'STOP_PENDING' (CVE-2005-2794).

The second is a DoS caused by certain crafted requests and SSL timeouts
(CVE-2005-2796).

The updated packages have been patched to address these issues.



Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2005:162
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the squid package";
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
if ( rpm_check( reference:"squid-2.5.STABLE9-1.3.101mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"squid-2.5.STABLE9-1.3.102mdk", release:"MDK10.2", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"squid-", release:"MDK10.1")
 || rpm_exists(rpm:"squid-", release:"MDK10.2") )
{
 set_kb_item(name:"CVE-2005-2794", value:TRUE);
 set_kb_item(name:"CVE-2005-2796", value:TRUE);
}
