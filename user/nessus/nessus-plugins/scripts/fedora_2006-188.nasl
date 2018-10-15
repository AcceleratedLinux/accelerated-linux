#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(21122);
 script_version ("$Revision: 1.1 $");
 script_cve_id("CVE-2006-1296");
 
 name["english"] = "Fedora Core 5 2006-188: beagle";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2006-188 (beagle).

A general infrastructure for making your data easy to find.

Update Information:

Some of the wrapper scripts (including beagle-status) looked
in the current directory for files with a specific name and
ran that instead of the binary in the path. All such cases
have been fixed in this release.


Solution : Get the newest Fedora Updates
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the beagle package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "Fedora Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"beagle-0.2.3-4", release:"FC5") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libbeagle-0.2.3-4", release:"FC5") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"libbeagle-devel-0.2.3-4", release:"FC5") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"beagle-", release:"FC5") )
{
 set_kb_item(name:"CVE-2006-1296", value:TRUE);
}
