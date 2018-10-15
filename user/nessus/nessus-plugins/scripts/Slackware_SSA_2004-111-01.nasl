# This script was automatically generated from the SSA-2004-111-01
# Slackware Security Advisory
# It is released under the Nessus Script Licence.
# Slackware Security Advisories are copyright 1999-2004 Slackware Linux, Inc.
# SSA2nasl Convertor is copyright 2004 Michel Arboi
# See http://www.slackware.com/about/ or http://www.slackware.com/security/
# Slackware(R) is a registered trademark of Slackware Linux, Inc.

if (! defined_func("bn_random")) exit(0);
desc='
New xine packages are available for Slackware 9.1 and -current to
fix security issues.

';
if (description) {
script_id(18758);
script_version("$Revision: 1.2 $");
script_category(ACT_GATHER_INFO);
script_family(english: "Slackware Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_copyright("This script is Copyright (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_require_keys("Host/Slackware/release", "Host/Slackware/packages");
script_description(english: desc);

script_xref(name: "SSA", value: "2004-111-01");
script_summary("SSA-2004-111-01 xine security update ");
name["english"] = "SSA-2004-111-01 xine security update ";
script_name(english:name["english"]);
exit(0);
}

include('slackware.inc');
include('global_settings.inc');

if (slackware_check(osver: "9.1", pkgname: "xine-lib", pkgver: "1rc3c", pkgnum:  "1", pkgarch: "i686")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package xine-lib is vulnerable in Slackware 9.1
Upgrade to xine-lib-1rc3c-i686-1 or newer.
');
}
if (slackware_check(osver: "9.1", pkgname: "xine-ui", pkgver: "0.99.1", pkgnum:  "1", pkgarch: "i686")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package xine-ui is vulnerable in Slackware 9.1
Upgrade to xine-ui-0.99.1-i686-1 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "xine-lib", pkgver: "1rc3c", pkgnum:  "2", pkgarch: "i686")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package xine-lib is vulnerable in Slackware -current
Upgrade to xine-lib-1rc3c-i686-2 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "xine-ui", pkgver: "0.99.1", pkgnum:  "1", pkgarch: "i686")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package xine-ui is vulnerable in Slackware -current
Upgrade to xine-ui-0.99.1-i686-1 or newer.
');
}

if (w) { security_hole(port: 0, data: desc); }
