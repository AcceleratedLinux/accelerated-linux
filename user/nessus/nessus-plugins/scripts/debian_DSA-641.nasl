# This script was automatically generated from the dsa-641
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Erik Sj�lund discovered that playmidi, a MIDI player, contains a
setuid root program with a buffer overflow that can be exploited by a
local attacker.
For the stable distribution (woody) this problem has been fixed in
version 2.4-4woody1.
For the unstable distribution (sid) this problem has been fixed in
version 2.4debian-3.
We recommend that you upgrade your playmidi package.


Solution : http://www.debian.org/security/2005/dsa-641
Risk factor : High';

if (description) {
 script_id(16181);
 script_version("$Revision: 1.4 $");
 script_xref(name: "DSA", value: "641");
 script_cve_id("CVE-2005-0020");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA641] DSA-641-1 playmidi");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-641-1 playmidi");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'playmidi', release: '3.0', reference: '2.4-4woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package playmidi is vulnerable in Debian 3.0.\nUpgrade to playmidi_2.4-4woody1\n');
}
if (deb_check(prefix: 'playmidi', release: '3.1', reference: '2.4debian-3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package playmidi is vulnerable in Debian 3.1.\nUpgrade to playmidi_2.4debian-3\n');
}
if (deb_check(prefix: 'playmidi', release: '3.0', reference: '2.4-4woody1')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package playmidi is vulnerable in Debian woody.\nUpgrade to playmidi_2.4-4woody1\n');
}
if (w) { security_hole(port: 0, data: desc); }
