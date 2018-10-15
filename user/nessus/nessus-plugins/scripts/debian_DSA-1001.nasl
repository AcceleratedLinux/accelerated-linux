# This script was automatically generated from the dsa-1001
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
It was discovered that Crossfire, a multiplayer adventure game, performs
insufficient bounds checking on network packets when run in "oldsocketmode",
which may possibly lead to the execution of arbitrary code.
For the old stable distribution (woody) this problem has been fixed in
version 1.1.0-1woody1.
For the stable distribution (sarge) this problem has been fixed in
version 1.6.0.dfsg.1-4sarge1.
For the unstable distribution (sid) this problem has been fixed in
version 1.9.0-1.
We recommend that you upgrade your crossfire packages.


Solution : http://www.debian.org/security/2006/dsa-1001
Risk factor : High';

if (description) {
 script_id(22543);
 script_version("$Revision: 1.1 $");
 script_xref(name: "DSA", value: "1001");
 script_cve_id("CVE-2006-1010");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA1001] DSA-1001-1 crossfire");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-1001-1 crossfire");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'crossfire', release: '', reference: '1.9.0-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire is vulnerable in Debian .\nUpgrade to crossfire_1.9.0-1\n');
}
if (deb_check(prefix: 'crossfire-doc', release: '3.0', reference: '1.1.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-doc is vulnerable in Debian 3.0.\nUpgrade to crossfire-doc_1.1.0-1woody1\n');
}
if (deb_check(prefix: 'crossfire-edit', release: '3.0', reference: '1.1.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-edit is vulnerable in Debian 3.0.\nUpgrade to crossfire-edit_1.1.0-1woody1\n');
}
if (deb_check(prefix: 'crossfire-server', release: '3.0', reference: '1.1.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-server is vulnerable in Debian 3.0.\nUpgrade to crossfire-server_1.1.0-1woody1\n');
}
if (deb_check(prefix: 'crossfire-doc', release: '3.1', reference: '1.6.0.dfsg.1-4sarge1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-doc is vulnerable in Debian 3.1.\nUpgrade to crossfire-doc_1.6.0.dfsg.1-4sarge1\n');
}
if (deb_check(prefix: 'crossfire-edit', release: '3.1', reference: '1.6.0.dfsg.1-4sarge1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-edit is vulnerable in Debian 3.1.\nUpgrade to crossfire-edit_1.6.0.dfsg.1-4sarge1\n');
}
if (deb_check(prefix: 'crossfire-server', release: '3.1', reference: '1.6.0.dfsg.1-4sarge1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire-server is vulnerable in Debian 3.1.\nUpgrade to crossfire-server_1.6.0.dfsg.1-4sarge1\n');
}
if (deb_check(prefix: 'crossfire', release: '3.1', reference: '1.6.0.dfsg.1-4sarge1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire is vulnerable in Debian sarge.\nUpgrade to crossfire_1.6.0.dfsg.1-4sarge1\n');
}
if (deb_check(prefix: 'crossfire', release: '3.0', reference: '1.1.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package crossfire is vulnerable in Debian woody.\nUpgrade to crossfire_1.1.0-1woody1\n');
}
if (w) { security_hole(port: 0, data: desc); }
