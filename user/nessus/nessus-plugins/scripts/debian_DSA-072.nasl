# This script was automatically generated from the dsa-072
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Zenith Parse found a security problem in groff (the GNU version of
troff). The pic command was vulnerable to a printf format attack
which made it possible to circumvent the `-S\' option and execute
arbitrary code.

This has been fixed in version 1.15.2-2, and we recommend that you upgrade
your groff packages immediately.



Solution : http://www.debian.org/security/2001/dsa-072
Risk factor : High';

if (description) {
 script_id(14909);
 script_version("$Revision: 1.5 $");
 script_xref(name: "DSA", value: "072");
 script_cve_id("CVE-2001-1022");
 script_bugtraq_id(3103);

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA072] DSA-072-1 groff");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-072-1 groff");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'groff', release: '2.2', reference: '1.15.2-2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package groff is vulnerable in Debian 2.2.\nUpgrade to groff_1.15.2-2\n');
}
if (w) { security_hole(port: 0, data: desc); }
