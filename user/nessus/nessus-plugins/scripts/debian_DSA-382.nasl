# This script was automatically generated from the dsa-382
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
A bug has been found in OpenSSH\'s buffer handling where a buffer could
be marked as grown when the actual reallocation failed.
DSA-382-2:
This advisory is an addition to the earlier DSA-382-1 advisory: two more
buffer handling problems have been found in addition to the one
described in DSA-382-1. It is not known if these bugs are exploitable,
but as a precaution an upgrade is advised.
DSA-382-3:
This advisory is an addition to the earlier DSA-382-1 and DSA-382-2
advisories: Solar Designer found four more bugs in OpenSSH that may be
exploitable.
For the Debian stable distribution (woody) these bugs have been fixed 
in version
1:3.4p1-1.woody.3.


Solution : http://www.debian.org/security/2003/dsa-382
Risk factor : High';

if (description) {
 script_id(15219);
 script_version("$Revision: 1.8 $");
 script_xref(name: "DSA", value: "382");
 script_cve_id("CVE-2003-0682", "CVE-2003-0693", "CVE-2003-0695");
 script_xref(name: "CERT", value: "333628");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA382] DSA-382-3 ssh");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-382-3 ssh");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'ssh', release: '3.0', reference: '3.4p1-1.woody.3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ssh is vulnerable in Debian 3.0.\nUpgrade to ssh_3.4p1-1.woody.3\n');
}
if (deb_check(prefix: 'ssh-askpass-gnome', release: '3.0', reference: '3.4p1-1.woody.3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ssh-askpass-gnome is vulnerable in Debian 3.0.\nUpgrade to ssh-askpass-gnome_3.4p1-1.woody.3\n');
}
if (deb_check(prefix: 'ssh', release: '3.0', reference: '3.4p1-1.woody.3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ssh is vulnerable in Debian woody.\nUpgrade to ssh_3.4p1-1.woody.3\n');
}
if (w) { security_hole(port: 0, data: desc); }
