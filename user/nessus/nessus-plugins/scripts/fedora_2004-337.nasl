#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(15578);
 script_version ("$Revision: 1.3 $");
 script_cve_id("CVE-2004-0888");
 
 name["english"] = "Fedora Core 2 2004-337: cups";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2004-337 (cups).

The Common UNIX Printing System provides a portable printing layer for
UNIX(r) operating systems. It has been developed by Easy Software Products
to promote a standard printing solution for all UNIX vendors and users.
CUPS provides the System V and Berkeley command-line interfaces.

Update Information:

A problem with PDF handling was discovered by Chris Evans, and has
been fixed.  The Common Vulnerabilities and Exposures project
(www.mitre.org) has assigned the name CVE-2004-0888 to this issue.



Solution : http://www.fedoranews.org/updates/FEDORA-2004-337.shtml
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the cups package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "Fedora Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"cups-1.1.20-11.6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"cups-devel-1.1.20-11.6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"cups-libs-1.1.20-11.6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"cups-debuginfo-1.1.20-11.6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"cups-", release:"FC2") )
{
 set_kb_item(name:"CVE-2004-0888", value:TRUE);
}
