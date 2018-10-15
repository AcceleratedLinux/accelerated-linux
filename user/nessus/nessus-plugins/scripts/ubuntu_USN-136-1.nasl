# This script was automatically generated from the 136-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- binutils 
- binutils-dev 
- binutils-doc 
- binutils-multiarch 


Description :

Tavis Ormandy found an integer overflow in the Binary File Descriptor
(BFD) parser in the GNU debugger. The same vulnerable code is also
present in binutils. By tricking an user into processing a specially
crafted executable with the binutils tools (strings, objdump, nm,
readelf, etc.), an attacker could exploit this to execute arbitrary
code with the privileges of the user running the affected program.

Solution :

Upgrade to : 
- binutils-2.15-5ubuntu2.1 (Ubuntu 5.04)
- binutils-dev-2.15-5ubuntu2.1 (Ubuntu 5.04)
- binutils-doc-2.15-5ubuntu2.1 (Ubuntu 5.04)
- binutils-multiarch-2.15-5ubuntu2.1 (Ubuntu 5.04)



Risk factor : High
';

if (description) {
script_id(20527);
script_version("$Revision: 1.3 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "136-1");
script_summary(english:"binutils vulnerability");
script_name(english:"USN136-1 : binutils vulnerability");
script_cve_id("CVE-2005-1704");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "5.04", pkgname: "binutils", pkgver: "2.15-5ubuntu2.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package binutils-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to binutils-2.15-5ubuntu2.1
');
}
found = ubuntu_check(osver: "5.04", pkgname: "binutils-dev", pkgver: "2.15-5ubuntu2.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package binutils-dev-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to binutils-dev-2.15-5ubuntu2.1
');
}
found = ubuntu_check(osver: "5.04", pkgname: "binutils-doc", pkgver: "2.15-5ubuntu2.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package binutils-doc-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to binutils-doc-2.15-5ubuntu2.1
');
}
found = ubuntu_check(osver: "5.04", pkgname: "binutils-multiarch", pkgver: "2.15-5ubuntu2.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package binutils-multiarch-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to binutils-multiarch-2.15-5ubuntu2.1
');
}

if (w) { security_hole(port: 0, data: desc); }
