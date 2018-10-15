# This script was automatically generated from the dsa-522
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Max Vozeler discovered a format string vulnerability in super, a
program to allow specified users to execute commands with root
privileges.  This vulnerability could potentially be exploited by a
local user to execute arbitrary code with root privileges.
For the current stable distribution (woody), this problem has been
fixed in version 3.16.1-1.2.
For the unstable distribution (sid), this problem has been fixed
in version 3.23.0-1.
We recommend that you update your super package.


Solution : http://www.debian.org/security/2004/dsa-522
Risk factor : High';

if (description) {
 script_id(15359);
 script_version("$Revision: 1.7 $");
 script_xref(name: "DSA", value: "522");
 script_cve_id("CVE-2004-0579");
 script_bugtraq_id(10575);

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA522] DSA-522-1 super");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-522-1 super");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'super', release: '3.0', reference: '3.16.1-1.2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package super is vulnerable in Debian 3.0.\nUpgrade to super_3.16.1-1.2\n');
}
if (deb_check(prefix: 'super', release: '3.1', reference: '3.23.0-1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package super is vulnerable in Debian 3.1.\nUpgrade to super_3.23.0-1\n');
}
if (deb_check(prefix: 'super', release: '3.0', reference: '3.16.1-1.2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package super is vulnerable in Debian woody.\nUpgrade to super_3.16.1-1.2\n');
}
if (w) { security_hole(port: 0, data: desc); }
