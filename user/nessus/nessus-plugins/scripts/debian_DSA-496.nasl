# This script was automatically generated from the dsa-496
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
H.D. Moore discovered several terminal emulator security issues.  One
of them covers escape codes that are interpreted by the terminal
emulator.  This could be exploited by an attacker to insert malicious
commands hidden for the user, who has to hit enter to continue, which
would also execute the hidden commands.
For the stable distribution (woody) this problem has been fixed in
version 0.9.2-0pre2002042903.3.
For the unstable distribution (sid) this problem has been fixed in
version 0.9.2-6.
We recommend that you upgrade your eterm package.


Solution : http://www.debian.org/security/2004/dsa-496
Risk factor : High';

if (description) {
 script_id(15333);
 script_version("$Revision: 1.6 $");
 script_xref(name: "DSA", value: "496");
 script_cve_id("CVE-2003-0068");
 script_bugtraq_id(10237);

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA496] DSA-496-1 eterm");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-496-1 eterm");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'eterm', release: '3.0', reference: '0.9.2-0pre2002042903.3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eterm is vulnerable in Debian 3.0.\nUpgrade to eterm_0.9.2-0pre2002042903.3\n');
}
if (deb_check(prefix: 'eterm', release: '3.1', reference: '0.9.2-6')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eterm is vulnerable in Debian 3.1.\nUpgrade to eterm_0.9.2-6\n');
}
if (deb_check(prefix: 'eterm', release: '3.0', reference: '0.9.2-0pre2002042903.3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package eterm is vulnerable in Debian woody.\nUpgrade to eterm_0.9.2-0pre2002042903.3\n');
}
if (w) { security_hole(port: 0, data: desc); }
