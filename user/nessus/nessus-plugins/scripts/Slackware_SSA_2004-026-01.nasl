# This script was automatically generated from the SSA-2004-026-01
# Slackware Security Advisory
# It is released under the Nessus Script Licence.
# Slackware Security Advisories are copyright 1999-2004 Slackware Linux, Inc.
# SSA2nasl Convertor is copyright 2004 Michel Arboi
# See http://www.slackware.com/about/ or http://www.slackware.com/security/
# Slackware(R) is a registered trademark of Slackware Linux, Inc.

if (! defined_func("bn_random")) exit(0);
desc='
GAIM is a GTK2-based Instant Messaging (IM) client.

New GAIM packages are available for Slackware 9.0, 9.1, and -current.
12 vulnerabilities were found in the instant messenger GAIM that
allow remote compromise.  All sites using GAIM should upgrade to these
new packages.  These are based on GAIM 0.75 with patches for all 12
security issues.  Thanks to Stefan Esser of e-matters GmbH for
finding and reporting these bugs.

For more details, see the e-matters GmbH advisory here:
  http://security.e-matters.de/advisories/012004.html


';
if (description) {
script_id(18750);
script_version("$Revision: 1.2 $");
script_category(ACT_GATHER_INFO);
script_family(english: "Slackware Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_copyright("This script is Copyright (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_require_keys("Host/Slackware/release", "Host/Slackware/packages");
script_description(english: desc);

script_xref(name: "SSA", value: "2004-026-01");
script_summary("SSA-2004-026-01 GAIM security update ");
name["english"] = "SSA-2004-026-01 GAIM security update ";
script_name(english:name["english"]);
exit(0);
}

include('slackware.inc');
include('global_settings.inc');

if (slackware_check(osver: "9.0", pkgname: "gaim", pkgver: "0.75", pkgnum:  "1", pkgarch: "i386")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package gaim is vulnerable in Slackware 9.0
Upgrade to gaim-0.75-i386-1 or newer.
');
}
if (slackware_check(osver: "9.1", pkgname: "gaim", pkgver: "0.75", pkgnum:  "1", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package gaim is vulnerable in Slackware 9.1
Upgrade to gaim-0.75-i486-1 or newer.
');
}
if (slackware_check(osver: "-current", pkgname: "gaim", pkgver: "0.75", pkgnum:  "1", pkgarch: "i486")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package gaim is vulnerable in Slackware -current
Upgrade to gaim-0.75-i486-1 or newer.
');
}

if (w) { security_hole(port: 0, data: desc); }
