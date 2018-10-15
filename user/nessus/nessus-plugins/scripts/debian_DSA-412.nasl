# This script was automatically generated from the dsa-412
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Multiple vulnerabilities were discovered in nd, a command-line WebDAV
interface, whereby long strings received from the remote server could
overflow fixed-length buffers.  This vulnerability could be exploited
by a remote attacker in control of a malicious WebDAV server to
execute arbitrary code if the server was accessed by a vulnerable
version of nd.
For the current stable distribution (woody) this problem has been
fixed in version 0.5.0-1woody1.
For the unstable distribution (sid) this problem has been fixed in
version 0.8.2-1.
We recommend that you update your nd package.


Solution : http://www.debian.org/security/2004/dsa-412
Risk factor : High';

if (description) {
 script_id(15249);
 script_version("$Revision: 1.8 $");
 script_xref(name: "DSA", value: "412");
 script_cve_id("CVE-2004-0014");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA412] DSA-412-1 nd");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-412-1 nd");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'nd', release: '3.0', reference: '0.5.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package nd is vulnerable in Debian 3.0.\nUpgrade to nd_0.5.0-1woody1\n');
}
if (deb_check(prefix: 'nd', release: '3.1', reference: '0.8.2-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package nd is vulnerable in Debian 3.1.\nUpgrade to nd_0.8.2-1\n');
}
if (deb_check(prefix: 'nd', release: '3.0', reference: '0.5.0-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package nd is vulnerable in Debian woody.\nUpgrade to nd_0.5.0-1woody1\n');
}
if (w) { security_hole(port: 0, data: desc); }
