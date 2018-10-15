# This script was automatically generated from the dsa-869
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
The developers of eric, a full featured Python IDE, have fixed a bug
in the processing of project files that could lead to the execution of
arbitrary code.
The old stable distribution (woody) does not contain an eric package.
For the stable distribution (sarge) this problem has been fixed in
version 3.6.2-2.
For the unstable distribution (sid) this problem has been fixed in
version 3.7.2-1.
We recommend that you upgrade your eric package.


Solution : http://www.debian.org/security/2005/dsa-869
Risk factor : High';

if (description) {
 script_id(20072);
 script_version("$Revision: 1.1 $");
 script_xref(name: "DSA", value: "869");
 script_cve_id("CVE-2005-3068");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA869] DSA-869-1 eric");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-869-1 eric");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'eric', release: '', reference: '3.7.2-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eric is vulnerable in Debian .\nUpgrade to eric_3.7.2-1\n');
}
if (deb_check(prefix: 'eric', release: '3.1', reference: '3.6.2-2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eric is vulnerable in Debian 3.1.\nUpgrade to eric_3.6.2-2\n');
}
if (deb_check(prefix: 'eric', release: '3.1', reference: '3.6.2-2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eric is vulnerable in Debian sarge.\nUpgrade to eric_3.6.2-2\n');
}
if (w) { security_hole(port: 0, data: desc); }
