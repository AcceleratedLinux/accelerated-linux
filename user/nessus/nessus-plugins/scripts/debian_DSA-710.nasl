# This script was automatically generated from the dsa-710
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Alan Cox discovered a problem in gtkhtml, an HTML rendering widget
used by the Evolution mail reader.  Certain malformed messages could
cause a crash due to a null pointer dereference.
For the stable distribution (woody) this problem has been fixed in
version 1.0.2-1.woody1.
For the unstable distribution (sid) this problem has been fixed in
version 1.0.4-6.2.
We recommend that you upgrade your gtkhtml package and restart
Evolution.


Solution : http://www.debian.org/security/2005/dsa-710
Risk factor : High';

if (description) {
 script_id(18080);
 script_version("$Revision: 1.5 $");
 script_xref(name: "DSA", value: "710");
 script_cve_id("CVE-2003-0541");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA710] DSA-710-1 gtkhtml");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-710-1 gtkhtml");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'gtkhtml', release: '3.0', reference: '1.0.2-1.woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gtkhtml is vulnerable in Debian 3.0.\nUpgrade to gtkhtml_1.0.2-1.woody1\n');
}
if (deb_check(prefix: 'libgtkhtml-data', release: '3.0', reference: '1.0.2-1.woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libgtkhtml-data is vulnerable in Debian 3.0.\nUpgrade to libgtkhtml-data_1.0.2-1.woody1\n');
}
if (deb_check(prefix: 'libgtkhtml-dev', release: '3.0', reference: '1.0.2-1.woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libgtkhtml-dev is vulnerable in Debian 3.0.\nUpgrade to libgtkhtml-dev_1.0.2-1.woody1\n');
}
if (deb_check(prefix: 'libgtkhtml20', release: '3.0', reference: '1.0.2-1.woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libgtkhtml20 is vulnerable in Debian 3.0.\nUpgrade to libgtkhtml20_1.0.2-1.woody1\n');
}
if (deb_check(prefix: 'gtkhtml', release: '3.1', reference: '1.0.4-6.2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gtkhtml is vulnerable in Debian 3.1.\nUpgrade to gtkhtml_1.0.4-6.2\n');
}
if (deb_check(prefix: 'gtkhtml', release: '3.0', reference: '1.0.2-1.woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gtkhtml is vulnerable in Debian woody.\nUpgrade to gtkhtml_1.0.2-1.woody1\n');
}
if (w) { security_hole(port: 0, data: desc); }
