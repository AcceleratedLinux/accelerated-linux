# This script was automatically generated from the 46-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- libtiff-tools 
- libtiff4 
- libtiff4-dev 


Description :

A buffer overflow was discovered in the TIFF library. A TIFF file
includes a value indicating the number of "directory entry" header
fields contained in the file. If this value is -1, an invalid memory
allocation was performed. A malicious image could be constructed
which, when decoded, would have resulted in execution of arbitrary
code with the privileges of the process using the library.

Since this library is used in many applications like "ghostscript" and
the "CUPS" printing system, this vulnerability may lead to remotely
induced privilege escalation.

Solution :

Upgrade to : 
- libtiff-tools-3.6.1-1.1ubuntu1.1 (Ubuntu 4.10)
- libtiff4-3.6.1-1.1ubuntu1.1 (Ubuntu 4.10)
- libtiff4-dev-3.6.1-1.1ubuntu1.1 (Ubuntu 4.10)



Risk factor : High
';

if (description) {
script_id(20663);
if(defined_func("script_xref"))script_xref(name:"IAVA", value:"2005-t-0015");
script_version("$Revision: 1.4 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "46-1");
script_summary(english:"tiff vulnerability");
script_name(english:"USN46-1 : tiff vulnerability");
script_cve_id("CVE-2004-1308");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "4.10", pkgname: "libtiff-tools", pkgver: "3.6.1-1.1ubuntu1.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtiff-tools-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtiff-tools-3.6.1-1.1ubuntu1.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libtiff4", pkgver: "3.6.1-1.1ubuntu1.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtiff4-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtiff4-3.6.1-1.1ubuntu1.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libtiff4-dev", pkgver: "3.6.1-1.1ubuntu1.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtiff4-dev-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtiff4-dev-3.6.1-1.1ubuntu1.1
');
}

if (w) { security_hole(port: 0, data: desc); }
