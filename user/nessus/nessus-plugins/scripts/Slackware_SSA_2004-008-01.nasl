# This script was automatically generated from the SSA-2004-008-01
# Slackware Security Advisory
# It is released under the Nessus Script Licence.
# Slackware Security Advisories are copyright 1999-2004 Slackware Linux, Inc.
# SSA2nasl Convertor is copyright 2004 Michel Arboi
# See http://www.slackware.com/about/ or http://www.slackware.com/security/
# Slackware(R) is a registered trademark of Slackware Linux, Inc.

if (! defined_func("bn_random")) exit(0);
desc='
New kernels are available for Slackware 8.1 containing a
backported fix from a bounds-checking problem in the kernel\'s
mremap() call which could be used by a local attacker to gain
root privileges.  This fix was previously issued for Slackware
9.0, 9.1, and -current (SSA:2004-006-01).

Sites running Slackware 8.1 should upgrade to the new kernel.
After installing the new kernel, be sure to run \'lilo\'.

More details about this issue may be found in the Common
Vulnerabilities and Exposures (CVE) database:

  http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2003-0985


';
if (description) {
script_id(18786);
script_version("$Revision: 1.3 $");
script_category(ACT_GATHER_INFO);
script_family(english: "Slackware Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_copyright("This script is Copyright (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_require_keys("Host/Slackware/release", "Host/Slackware/packages");
script_description(english: desc);

script_xref(name: "SSA", value: "2004-008-01");
script_summary("SSA-2004-008-01 Slackware 8.1 kernel security update  ");
name["english"] = "SSA-2004-008-01 Slackware 8.1 kernel security update  ";
script_name(english:name["english"]);
script_cve_id("CVE-2003-0985");
exit(0);
}

include('slackware.inc');
include('global_settings.inc');

if (slackware_check(osver: "8.1", pkgname: "kernel-ide", pkgver: "2.4.18", pkgnum:  "5", pkgarch: "i386")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kernel-ide is vulnerable in Slackware 8.1
Upgrade to kernel-ide-2.4.18-i386-5 or newer.
');
}
if (slackware_check(osver: "8.1", pkgname: "kernel-source", pkgver: "2.4.18", pkgnum:  "6", pkgarch: "noarch")) {
w++;
if (report_verbosity > 0) desc = strcat(desc, '
The package kernel-source is vulnerable in Slackware 8.1
Upgrade to kernel-source-2.4.18-noarch-6 or newer.
');
}

if (w) { security_hole(port: 0, data: desc); }
