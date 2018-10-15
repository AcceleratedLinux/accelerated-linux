# This script was automatically generated from the 127-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- bzip2 
- libbz2-1.0 
- libbz2-dev 


Description :

Imran Ghory discovered a race condition in the file permission restore
code of bunzip2. While a user was decompressing a file, a local
attacker with write permissions in the directory of that file could
replace the target file with a hard link. This would cause bzip2 to
restore the file permissions to the hard link target instead of to the
bzip2 output file, which could be exploited to gain read or even write
access to files of other users. (CVE-2005-0953)

Specially crafted bzip2 archives caused an infinite loop in the
decompressor which resulted in an indefinitively large output file
("decompression bomb"). This could be exploited to a Denial of Service
attack due to disk space exhaustion on systems which automatically
process user supplied bzip2 compressed files. (CVE-2005-1260)

Solution :

Upgrade to : 
- bzip2-1.0.2-2ubuntu0.1 (Ubuntu 5.04)
- libbz2-1.0-1.0.2-2ubuntu0.1 (Ubuntu 5.04)
- libbz2-dev-1.0.2-2ubuntu0.1 (Ubuntu 5.04)



Risk factor : High
';

if (description) {
script_id(20517);
script_version("$Revision: 1.3 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "127-1");
script_summary(english:"bzip2 vulnerabilities");
script_name(english:"USN127-1 : bzip2 vulnerabilities");
script_cve_id("CVE-2005-0953","CVE-2005-1260");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "5.04", pkgname: "bzip2", pkgver: "1.0.2-2ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package bzip2-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to bzip2-1.0.2-2ubuntu0.1
');
}
found = ubuntu_check(osver: "5.04", pkgname: "libbz2-1.0", pkgver: "1.0.2-2ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libbz2-1.0-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to libbz2-1.0-1.0.2-2ubuntu0.1
');
}
found = ubuntu_check(osver: "5.04", pkgname: "libbz2-dev", pkgver: "1.0.2-2ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libbz2-dev-',found,' is vulnerable in Ubuntu 5.04
Upgrade it to libbz2-dev-1.0.2-2ubuntu0.1
');
}

if (w) { security_hole(port: 0, data: desc); }
