# This script was automatically generated from the SSA-2004-278-01
# Slackware Security Advisory
# It is released under the Nessus Script Licence.
# Slackware Security Advisories are copyright 1999-2004 Slackware Linux, Inc.
# SSA2nasl Convertor is copyright 2004 Michel Arboi
# See http://www.slackware.com/about/ or http://www.slackware.com/security/
# Slackware(R) is a registered trademark of Slackware Linux, Inc.

if (! defined_func("bn_random")) exit(0);
desc='
New getmail packages are available for Slackware 9.1, 10.0 and -current to
fix a security issue.  If getmail is used as root to deliver to user owned
files or directories, it can be made to overwrite system files.

More details about this issue may be found in the Common
Vulnerabilities and Exposures (CVE) database:

  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2004-880
  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2004-881

';
if (description) {
script_id(18776);
script_version("$Revision: 1.4 $");
script_category(ACT_GATHER_INFO);
script_family(english: "Slackware Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_copyright("This script is Copyright (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_require_keys("Host/Slackware/release", "Host/Slackware/packages");
script_description(english: desc);

script_xref(name: "SSA", value: "2004-278-01");
script_summary("SSA-2004-278-01 getmail ");
name["english"] = "SSA-2004-278-01 getmail ";
script_name(english:name["english"]);
script_cve_id("CAN-2004-880","CAN-2004-881");
exit(0);
}

include('slackware.inc');
include('global_settings.inc');

if (slackware_check(osver: "9.1", pkgname: "getmail", pkgver: "3.2.5", pkgnum:  "1", pkgarch: "noarch")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package getmail is vulnerable in Slackware 9.1
Upgrade to getmail-3.2.5-noarch-1 or newer.
');
}
if (slackware_check(osver: "10.0", pkgname: "getmail", pkgver: "4.2.0", pkgnum:  "1", pkgarch: "noarch")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package getmail is vulnerable in Slackware 10.0
Upgrade to getmail-4.2.0-noarch-1 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "getmail", pkgver: "4.2.0", pkgnum:  "1", pkgarch: "noarch")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package getmail is vulnerable in Slackware -current
Upgrade to getmail-4.2.0-noarch-1 or newer.
');
}

if (w) { security_hole(port: 0, data: desc); }
