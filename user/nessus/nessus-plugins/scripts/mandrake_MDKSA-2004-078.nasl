#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2004:078
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(14176);
 script_bugtraq_id(10136);
 script_version ("$Revision: 1.5 $");
 script_cve_id("CVE-2004-0179", "CVE-2004-0398");
 
 name["english"] = "MDKSA-2004:078: OpenOffice.org";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2004:078 (OpenOffice.org).


The OpenOffice.org office suite contains an internal libneon library which
allows it to connect to WebDAV servers. This internal library is subject to the
same vulnerabilities that were fixed in libneon recently. These updated packages
contain fixes to libneon to correct the several format string vulnerabilities in
it, as well as a heap-based buffer overflow vulnerability.


Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2004:078
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the OpenOffice.org package";
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
if ( rpm_check( reference:"OpenOffice.org-1.1.2-3.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"OpenOffice.org-libs-1.1.2-3.1.100mdk", release:"MDK10.0", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"OpenOffice.org-", release:"MDK10.0") )
{
 set_kb_item(name:"CVE-2004-0179", value:TRUE);
 set_kb_item(name:"CVE-2004-0398", value:TRUE);
}
