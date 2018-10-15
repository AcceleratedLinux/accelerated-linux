# This script was automatically generated from the dsa-901
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Several vulnerabilities have been discovered in gnump3d, a streaming
server for MP3 and OGG files.  The Common Vulnerabilities and
Exposures Project identifies the following problems:
    Ludwig Nussel discovered several temporary files that are created
    with predictable filenames in an insecure fashion and allows local
    attackers to craft symlink attacks.
    Ludwig Nussel discovered that the theme parameter to HTTP
    requests may be used for path traversal.
The old stable distribution (woody) does not contain a gnump3d package.
For the stable distribution (sarge) these problems have been fixed in
version 2.9.3-1sarge3.
For the unstable distribution (sid) these problems have been fixed in
version 2.9.8-1.
We recommend that you upgrade your gnump3 package.


Solution : http://www.debian.org/security/2005/dsa-901
Risk factor : High';

if (description) {
 script_id(22767);
 script_version("$Revision: 1.1 $");
 script_xref(name: "DSA", value: "901");
 script_cve_id("CVE-2005-3349", "CVE-2005-3355");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA901] DSA-901-1 gnump3d");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-901-1 gnump3d");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'gnump3d', release: '', reference: '2.9.8-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gnump3d is vulnerable in Debian .\nUpgrade to gnump3d_2.9.8-1\n');
}
if (deb_check(prefix: 'gnump3d', release: '3.1', reference: '2.9.3-1sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gnump3d is vulnerable in Debian 3.1.\nUpgrade to gnump3d_2.9.3-1sarge3\n');
}
if (deb_check(prefix: 'gnump3d', release: '3.1', reference: '2.9.3-1sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package gnump3d is vulnerable in Debian sarge.\nUpgrade to gnump3d_2.9.3-1sarge3\n');
}
if (w) { security_hole(port: 0, data: desc); }
