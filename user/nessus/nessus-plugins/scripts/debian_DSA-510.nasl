# This script was automatically generated from the dsa-510
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
jaguar@felinemenace.org discovered a vulnerability in jftpgw, an FTP
proxy program, whereby a remote user could potentially cause arbitrary
code to be executed with the privileges of the jftpgw server process.
By default, the server runs as user "nobody".
CVE-2004-0448: format string vulnerability via syslog(3) in log()
function
For the current stable distribution (woody) this problem has been
fixed in version 0.13.1-1woody1.
For the unstable distribution (sid), this problem has been fixed in
version 0.13.4-1.
We recommend that you update your jftpgw package.


Solution : http://www.debian.org/security/2004/dsa-510
Risk factor : High';

if (description) {
 script_id(15347);
 script_version("$Revision: 1.7 $");
 script_xref(name: "DSA", value: "510");
 script_cve_id("CVE-2004-0448");
 script_bugtraq_id(10438);

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA510] DSA-510-1 jftpgw");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-510-1 jftpgw");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'jftpgw', release: '3.0', reference: '0.13.1-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package jftpgw is vulnerable in Debian 3.0.\nUpgrade to jftpgw_0.13.1-1woody1\n');
}
if (deb_check(prefix: 'jftpgw', release: '3.1', reference: '0.13.4-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package jftpgw is vulnerable in Debian 3.1.\nUpgrade to jftpgw_0.13.4-1\n');
}
if (deb_check(prefix: 'jftpgw', release: '3.0', reference: '0.13.1-1woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package jftpgw is vulnerable in Debian woody.\nUpgrade to jftpgw_0.13.1-1woody1\n');
}
if (w) { security_hole(port: 0, data: desc); }
