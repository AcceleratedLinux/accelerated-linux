# This script was automatically generated from the SSA-2004-247-01
# Slackware Security Advisory
# It is released under the Nessus Script Licence.
# Slackware Security Advisories are copyright 1999-2004 Slackware Linux, Inc.
# SSA2nasl Convertor is copyright 2004 Michel Arboi
# See http://www.slackware.com/about/ or http://www.slackware.com/security/
# Slackware(R) is a registered trademark of Slackware Linux, Inc.

if (! defined_func("bn_random")) exit(0);
desc='
New kdelibs and kdebase packages are available for Slackware 9.1, 10.0,
and -current to fix security issues.

More details about this issues may be found in the Common
Vulnerabilities and Exposures (CVE) database:

  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-0689
  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-0690
  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-0721
  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-0746


';
if (description) {
script_id(18782);
script_version("$Revision: 1.3 $");
script_category(ACT_GATHER_INFO);
script_family(english: "Slackware Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_copyright("This script is Copyright (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_require_keys("Host/Slackware/release", "Host/Slackware/packages");
script_description(english: desc);

script_xref(name: "SSA", value: "2004-247-01");
script_summary("SSA-2004-247-01 kde ");
name["english"] = "SSA-2004-247-01 kde ";
script_name(english:name["english"]);
script_cve_id("CVE-2004-0689","CVE-2004-0690","CVE-2004-0721","CVE-2004-0746");
exit(0);
}

include('slackware.inc');
include('global_settings.inc');

if (slackware_check(osver: "9.1", pkgname: "kdebase", pkgver: "3.1.4", pkgnum:  "2", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdebase is vulnerable in Slackware 9.1
Upgrade to kdebase-3.1.4-i486-2 or newer.
');
}
if (slackware_check(osver: "9.1", pkgname: "kdelibs", pkgver: "3.1.4", pkgnum:  "3", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdelibs is vulnerable in Slackware 9.1
Upgrade to kdelibs-3.1.4-i486-3 or newer.
');
}
if (slackware_check(osver: "10.0", pkgname: "kdebase", pkgver: "3.2.3", pkgnum:  "2", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdebase is vulnerable in Slackware 10.0
Upgrade to kdebase-3.2.3-i486-2 or newer.
');
}
if (slackware_check(osver: "10.0", pkgname: "kdelibs", pkgver: "3.2.3", pkgnum:  "2", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdelibs is vulnerable in Slackware 10.0
Upgrade to kdelibs-3.2.3-i486-2 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "kdebase", pkgver: "3.2.3", pkgnum:  "2", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdebase is vulnerable in Slackware -current
Upgrade to kdebase-3.2.3-i486-2 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "kdelibs", pkgver: "3.2.3", pkgnum:  "2", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kdelibs is vulnerable in Slackware -current
Upgrade to kdelibs-3.2.3-i486-2 or newer.
');
}

if (w) { security_hole(port: 0, data: desc); }
