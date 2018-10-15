#
# (C) Tenable Network Security
#
#
# The text of this plugin is (C) Red Hat Inc.

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16263);
 script_version ("$Revision: 1.2 $");
 script_cve_id("CVE-2005-0064");

 name["english"] = "RHSA-2005-059: xpdf";
 
 script_name(english:name["english"]);
 
 desc["english"] = '

  Updated Xpdf package that fixes a stack based buffer overflow security
  issue
  is now available.

  Xpdf is an X Window System based viewer for Portable Document Format (PDF)
  files.

  A buffer overflow flaw was found when processing the /Encrypt /Length tag.
  An attacker could construct a carefully crafted PDF file that could cause
  Xpdf to crash or possibly execute arbitrary code when opened. The Common
  Vulnerabilities and Exposures project (cve.mitre.org) has assigned the name
  CVE-2005-0064 to this issue.

  Red Hat believes that the Exec-Shield technology (enabled by default since
  Update 3) will block attempts to exploit this vulnerability on x86
  architectures.

  All users of the Xpdf package should upgrade to this updated package,
  which resolves this issue




Solution : http://rhn.redhat.com/errata/RHSA-2005-059.html
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the xpdf packages";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "Red Hat Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"xpdf-2.02-9.5", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}

if ( rpm_exists(rpm:"xpdf-", release:"RHEL3") )
{
 set_kb_item(name:"CVE-2005-0064", value:TRUE);
}

set_kb_item(name:"RHSA-2005-059", value:TRUE);
