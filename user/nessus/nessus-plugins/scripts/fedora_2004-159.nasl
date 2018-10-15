#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(13715);
 script_bugtraq_id(10397);
 script_version ("$Revision: 1.5 $");
 script_cve_id("CVE-2004-0520", "CVE-2004-0521");
 
 name["english"] = "Fedora Core 1 2004-159: squirrelmail";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2004-159 (squirrelmail).

SquirrelMail is a standards-based webmail package written in PHP4. It
includes built-in pure PHP support for the IMAP and SMTP protocols, and
all pages render in pure HTML 4.0 (with no Javascript) for maximum
compatibility across browsers.  It has very few requirements and is very
easy to configure and install. SquirrelMail has all the functionality
you would want from an email client, including strong MIME support,
address books, and folder manipulation.

Update Information:

An SQL injection flaw was found in SquirrelMail version 1.4.2 and
earlier.  If SquirrelMail is configured to store user addressbooks in
the database, a remote attacker could use this flaw to execute
arbitrary SQL statements.  The Common Vulnerabilities and Exposures
project has assigned the name CVE-2004-0521 to this issue.

A number of cross-site scripting (XSS) flaws in SquirrelMail version
1.4.2 and earlier could allow remote attackers to execute scripts as
other web users. The Common Vulnerabilities and Exposures project
(cve.mitre.org) has assigned the names CVE-2004-0519 and CVE-2004-0520
to these issues.

This update includes the SquirrelMail version 1.4.3a which is not
vulnerable to these issues.



Solution : http://www.fedoranews.org/updates/FEDORA-2004-159.shtml
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the squirrelmail package";
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
if ( rpm_check( reference:"squirrelmail-1.4.3-0.f1.1", prefix:"squirrelmail-", release:"FC1") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"squirrelmail-", release:"FC1") )
{
 set_kb_item(name:"CVE-2004-0520", value:TRUE);
 set_kb_item(name:"CVE-2004-0521", value:TRUE);
}
