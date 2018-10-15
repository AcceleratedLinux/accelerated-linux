# This script was automatically generated from the 286-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- dia 
- dia-common 
- dia-gnome 
- dia-libs 


Description :

Several format string vulnerabilities have been discovered in dia. By
tricking a user into opening a specially crafted dia file, or a
file with a specially crafted name, this could be exploited to execute
arbitrary code with the user\'s privileges.

Solution :

Upgrade to : 
- dia-0.94.0-11ubuntu1.2 (Ubuntu 5.10)
- dia-common-0.94.0-11ubuntu1.2 (Ubuntu 5.10)
- dia-gnome-0.94.0-11ubuntu1.2 (Ubuntu 5.10)
- dia-libs-0.94.0-11ubuntu1.2 (Ubuntu 5.10)



Risk factor : High
';

if (description) {
script_id(21604);
script_version("$Revision: 1.1 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "286-1");
script_summary(english:"dia vulnerabilities");
script_name(english:"USN286-1 : dia vulnerabilities");
script_cve_id("CVE-2006-2453","CVE-2006-2480");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "5.10", pkgname: "dia", pkgver: "0.94.0-11ubuntu1.2");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package dia-',found,' is vulnerable in Ubuntu 5.10
Upgrade it to dia-0.94.0-11ubuntu1.2
');
}
found = ubuntu_check(osver: "5.10", pkgname: "dia-common", pkgver: "0.94.0-11ubuntu1.2");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package dia-common-',found,' is vulnerable in Ubuntu 5.10
Upgrade it to dia-common-0.94.0-11ubuntu1.2
');
}
found = ubuntu_check(osver: "5.10", pkgname: "dia-gnome", pkgver: "0.94.0-11ubuntu1.2");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package dia-gnome-',found,' is vulnerable in Ubuntu 5.10
Upgrade it to dia-gnome-0.94.0-11ubuntu1.2
');
}
found = ubuntu_check(osver: "5.10", pkgname: "dia-libs", pkgver: "0.94.0-11ubuntu1.2");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package dia-libs-',found,' is vulnerable in Ubuntu 5.10
Upgrade it to dia-libs-0.94.0-11ubuntu1.2
');
}

if (w) { security_hole(port: 0, data: desc); }
