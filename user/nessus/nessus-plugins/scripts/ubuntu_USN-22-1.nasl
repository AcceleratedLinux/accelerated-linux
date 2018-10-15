# This script was automatically generated from the 22-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- libpam-smbpass 
- libsmbclient 
- libsmbclient-dev 
- python2.3-samba 
- samba 
- samba-common 
- samba-doc 
- smbclient 
- smbfs 
- swat 
- winbind 


Description :

Karol Wiesek discovered a Denial of Service vulnerability in samba. A
flaw in the input validation routines used to match filename strings
containing wildcard characters may allow a remote user to consume more
than normal amounts of CPU resources, thus impacting the performance
and response of the server.  In some circumstances the server can
become entirely unresponsive.

Solution :

Upgrade to : 
- libpam-smbpass-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- libsmbclient-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- libsmbclient-dev-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- python2.3-samba-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- samba-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- samba-common-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- samba-doc-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- smbclient-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- smbfs-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- swat-3.0.7-1ubuntu6.1 (Ubuntu 4.10)
- winbind-3.0.7-1ubuntu6.1 (Ubuntu 4.10)



Risk factor : High
';

if (description) {
script_id(20637);
script_version("$Revision: 1.3 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "22-1");
script_summary(english:"samba vulnerability");
script_name(english:"USN22-1 : samba vulnerability");
script_cve_id("CVE-2004-0930");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "4.10", pkgname: "libpam-smbpass", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libpam-smbpass-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libpam-smbpass-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libsmbclient", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libsmbclient-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libsmbclient-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libsmbclient-dev", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libsmbclient-dev-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libsmbclient-dev-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "python2.3-samba", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package python2.3-samba-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to python2.3-samba-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "samba", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package samba-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to samba-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "samba-common", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package samba-common-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to samba-common-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "samba-doc", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package samba-doc-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to samba-doc-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "smbclient", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package smbclient-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to smbclient-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "smbfs", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package smbfs-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to smbfs-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "swat", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package swat-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to swat-3.0.7-1ubuntu6.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "winbind", pkgver: "3.0.7-1ubuntu6.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package winbind-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to winbind-3.0.7-1ubuntu6.1
');
}

if (w) { security_hole(port: 0, data: desc); }
