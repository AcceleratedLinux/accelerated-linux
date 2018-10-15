# This script was automatically generated from the dsa-200
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Steve Langasek found an exploitable bug in the password handling
code in samba: when converting from DOS code-page to little endian
UCS2 unicode a buffer length was not checked and a buffer could
be overflowed. There is no known exploit for this, but an upgrade
is strongly recommended.
This problem has been fixed in version 2.2.3a-12 of the Debian
samba packages and upstream version 2.2.7.


Solution : http://www.debian.org/security/2002/dsa-200
Risk factor : High';

if (description) {
 script_id(15037);
 script_version("$Revision: 1.5 $");
 script_xref(name: "DSA", value: "200");
 script_cve_id("CVE-2002-1318");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA200] DSA-200-1 samba");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-200-1 samba");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'libpam-smbpass', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libpam-smbpass is vulnerable in Debian 3.0.\nUpgrade to libpam-smbpass_2.2.3a-12\n');
}
if (deb_check(prefix: 'libsmbclient', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libsmbclient is vulnerable in Debian 3.0.\nUpgrade to libsmbclient_2.2.3a-12\n');
}
if (deb_check(prefix: 'libsmbclient-dev', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libsmbclient-dev is vulnerable in Debian 3.0.\nUpgrade to libsmbclient-dev_2.2.3a-12\n');
}
if (deb_check(prefix: 'samba', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package samba is vulnerable in Debian 3.0.\nUpgrade to samba_2.2.3a-12\n');
}
if (deb_check(prefix: 'samba-common', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package samba-common is vulnerable in Debian 3.0.\nUpgrade to samba-common_2.2.3a-12\n');
}
if (deb_check(prefix: 'samba-doc', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package samba-doc is vulnerable in Debian 3.0.\nUpgrade to samba-doc_2.2.3a-12\n');
}
if (deb_check(prefix: 'smbclient', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package smbclient is vulnerable in Debian 3.0.\nUpgrade to smbclient_2.2.3a-12\n');
}
if (deb_check(prefix: 'smbfs', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package smbfs is vulnerable in Debian 3.0.\nUpgrade to smbfs_2.2.3a-12\n');
}
if (deb_check(prefix: 'swat', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package swat is vulnerable in Debian 3.0.\nUpgrade to swat_2.2.3a-12\n');
}
if (deb_check(prefix: 'winbind', release: '3.0', reference: '2.2.3a-12')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package winbind is vulnerable in Debian 3.0.\nUpgrade to winbind_2.2.3a-12\n');
}
if (w) { security_hole(port: 0, data: desc); }
