# This script was automatically generated from the dsa-853
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
Several security problems have been discovered in ethereal, a commonly
used network traffic analyser.  The Common Vulnerabilities and
Exposures project identifies the following problems:
    Memory allocation errors in the LDAP dissector can cause a denial
    of service.
    Various errors in the AgentX, PER, DOCSIS, RADIUS, Telnet, IS-IS,
    HTTP, DCERPC, DHCP and SCTP dissectors can cause a denial of
    service.
    Various errors in the SMPP, 802.3, H1 and DHCP dissectors can
    cause a denial of service.
    Null pointer dereferences in the WBXML and GIOP dissectors can
    cause a denial of service.
    A buffer overflow and null pointer dereferences in the SMB
    dissector can cause a denial of service.
    Wrong address calculation in the BER dissector can cause an
    infinite loop or abortion.
    Format string vulnerabilities in several dissectors allow
    remote attackers to write to arbitrary memory locations and thus
    gain privileges.
For the old stable distribution (woody) these problems have been fixed in
version 0.9.4-1woody13.
For the stable distribution (sarge) these problems have been fixed in
version 0.10.10-2sarge3.
For the unstable distribution (sid) these problems have been fixed in
version 0.10.12-2.
We recommend that you upgrade your ethereal packages.


Solution : http://www.debian.org/security/2005/dsa-853
Risk factor : High';

if (description) {
 script_id(19961);
 script_version("$Revision: 1.2 $");
 script_xref(name: "DSA", value: "853");
 script_cve_id("CVE-2005-2360", "CVE-2005-2361", "CVE-2005-2363", "CVE-2005-2364", "CVE-2005-2365", "CVE-2005-2366", "CVE-2005-2367");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA853] DSA-853-1 ethereal");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-853-1 ethereal");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'ethereal', release: '', reference: '0.10.12-2')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal is vulnerable in Debian .\nUpgrade to ethereal_0.10.12-2\n');
}
if (deb_check(prefix: 'ethereal', release: '3.0', reference: '0.9.4-1woody13')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal is vulnerable in Debian 3.0.\nUpgrade to ethereal_0.9.4-1woody13\n');
}
if (deb_check(prefix: 'ethereal-common', release: '3.0', reference: '0.9.4-1woody13')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal-common is vulnerable in Debian 3.0.\nUpgrade to ethereal-common_0.9.4-1woody13\n');
}
if (deb_check(prefix: 'ethereal-dev', release: '3.0', reference: '0.9.4-1woody13')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal-dev is vulnerable in Debian 3.0.\nUpgrade to ethereal-dev_0.9.4-1woody13\n');
}
if (deb_check(prefix: 'tethereal', release: '3.0', reference: '0.9.4-1woody13')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package tethereal is vulnerable in Debian 3.0.\nUpgrade to tethereal_0.9.4-1woody13\n');
}
if (deb_check(prefix: 'ethereal', release: '3.1', reference: '0.10.10-2sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal is vulnerable in Debian 3.1.\nUpgrade to ethereal_0.10.10-2sarge3\n');
}
if (deb_check(prefix: 'ethereal-common', release: '3.1', reference: '0.10.10-2sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal-common is vulnerable in Debian 3.1.\nUpgrade to ethereal-common_0.10.10-2sarge3\n');
}
if (deb_check(prefix: 'ethereal-dev', release: '3.1', reference: '0.10.10-2sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal-dev is vulnerable in Debian 3.1.\nUpgrade to ethereal-dev_0.10.10-2sarge3\n');
}
if (deb_check(prefix: 'tethereal', release: '3.1', reference: '0.10.10-2sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package tethereal is vulnerable in Debian 3.1.\nUpgrade to tethereal_0.10.10-2sarge3\n');
}
if (deb_check(prefix: 'ethereal', release: '3.1', reference: '0.10.10-2sarge3')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal is vulnerable in Debian sarge.\nUpgrade to ethereal_0.10.10-2sarge3\n');
}
if (deb_check(prefix: 'ethereal', release: '3.0', reference: '0.9.4-1woody13')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package ethereal is vulnerable in Debian woody.\nUpgrade to ethereal_0.9.4-1woody13\n');
}
if (w) { security_hole(port: 0, data: desc); }
