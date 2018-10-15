# This script was automatically generated from the 8-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

The remote package "gaim" is missing a security patch.

Description :

A buffer overflow and two remote crashes were recently discovered in
gaim\'s MSN protocol handler. An attacker could potentially execute
arbitrary code with the user\'s privileges by crafting and sending a
particular MSN message.

Solution :

Upgrade to : 
- gaim-1.0.0-1ubuntu1.1 (Ubuntu 4.10)



Risk factor : High
';

if (description) {
script_id(20703);
script_version("$Revision: 1.3 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "8-1");
script_summary(english:"gaim vulnerabilities");
script_name(english:"USN8-1 : gaim vulnerabilities");
script_cve_id("CVE-2004-0891");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "4.10", pkgname: "gaim", pkgver: "1.0.0-1ubuntu1.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package gaim-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to gaim-1.0.0-1ubuntu1.1
');
}

if (w) { security_hole(port: 0, data: desc); }
