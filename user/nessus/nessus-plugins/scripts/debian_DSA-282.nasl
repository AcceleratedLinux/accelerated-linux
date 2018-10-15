# This script was automatically generated from the dsa-282
# Debian Security Advisory
# It is released under the Nessus Script Licence.
# Advisory is copyright 1997-2004 Software in the Public Interest, Inc.
# See http://www.debian.org/license
# DSA2nasl Convertor is copyright 2004 Michel Arboi

if (! defined_func('bn_random')) exit(0);

desc = '
eEye Digital Security discovered an integer overflow in the
xdrmem_getbytes() function which is also present in GNU libc.  This
function is part of the XDR (external data representation)
encoder/decoder derived from Sun\'s RPC implementation.  Depending upon
the application, this vulnerability can cause buffer overflows and
could possibly be exploited to execute arbitrary code.
For the stable distribution (woody) this problem has been
fixed in version 2.2.5-11.5.
For the old stable distribution (potato) this problem has been
fixed in version 2.1.3-25.
For the unstable distribution (sid) this problem has been
fixed in version 2.3.1-16.
We recommend that you upgrade your libc6 packages.


Solution : http://www.debian.org/security/2003/dsa-282
Risk factor : High';

if (description) {
 script_id(15119);
 if(defined_func("script_xref"))script_xref(name:"IAVA", value:"2003-t-0007");
 script_version("$Revision: 1.8 $");
 script_xref(name: "DSA", value: "282");
 script_cve_id("CVE-2003-0028");
 script_bugtraq_id(7123);
 script_xref(name: "CERT", value: "516825");

 script_description(english: desc);
 script_copyright(english: "This script is (C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[DSA282] DSA-282-1 glibc");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Debian Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Debian/dpkg-l");
 script_summary(english: "DSA-282-1 glibc");
 exit(0);
}

include("debian_package.inc");

w = 0;
if (deb_check(prefix: 'glibc-doc', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package glibc-doc is vulnerable in Debian 2.2.\nUpgrade to glibc-doc_2.1.3-25\n');
}
if (deb_check(prefix: 'i18ndata', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package i18ndata is vulnerable in Debian 2.2.\nUpgrade to i18ndata_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6 is vulnerable in Debian 2.2.\nUpgrade to libc6_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6-dbg', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-dbg is vulnerable in Debian 2.2.\nUpgrade to libc6-dbg_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6-dev', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-dev is vulnerable in Debian 2.2.\nUpgrade to libc6-dev_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6-pic', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-pic is vulnerable in Debian 2.2.\nUpgrade to libc6-pic_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6-prof', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-prof is vulnerable in Debian 2.2.\nUpgrade to libc6-prof_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6.1', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1 is vulnerable in Debian 2.2.\nUpgrade to libc6.1_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6.1-dbg', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-dbg is vulnerable in Debian 2.2.\nUpgrade to libc6.1-dbg_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6.1-dev', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-dev is vulnerable in Debian 2.2.\nUpgrade to libc6.1-dev_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6.1-pic', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-pic is vulnerable in Debian 2.2.\nUpgrade to libc6.1-pic_2.1.3-25\n');
}
if (deb_check(prefix: 'libc6.1-prof', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-prof is vulnerable in Debian 2.2.\nUpgrade to libc6.1-prof_2.1.3-25\n');
}
if (deb_check(prefix: 'libnss1-compat', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libnss1-compat is vulnerable in Debian 2.2.\nUpgrade to libnss1-compat_2.1.3-25\n');
}
if (deb_check(prefix: 'locales', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package locales is vulnerable in Debian 2.2.\nUpgrade to locales_2.1.3-25\n');
}
if (deb_check(prefix: 'nscd', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package nscd is vulnerable in Debian 2.2.\nUpgrade to nscd_2.1.3-25\n');
}
if (deb_check(prefix: 'glibc-doc', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package glibc-doc is vulnerable in Debian 3.0.\nUpgrade to glibc-doc_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6 is vulnerable in Debian 3.0.\nUpgrade to libc6_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-dbg', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-dbg is vulnerable in Debian 3.0.\nUpgrade to libc6-dbg_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-dev', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-dev is vulnerable in Debian 3.0.\nUpgrade to libc6-dev_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-dev-sparc64', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-dev-sparc64 is vulnerable in Debian 3.0.\nUpgrade to libc6-dev-sparc64_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-pic', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-pic is vulnerable in Debian 3.0.\nUpgrade to libc6-pic_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-prof', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-prof is vulnerable in Debian 3.0.\nUpgrade to libc6-prof_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6-sparc64', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6-sparc64 is vulnerable in Debian 3.0.\nUpgrade to libc6-sparc64_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6.1', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1 is vulnerable in Debian 3.0.\nUpgrade to libc6.1_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6.1-dbg', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-dbg is vulnerable in Debian 3.0.\nUpgrade to libc6.1-dbg_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6.1-dev', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-dev is vulnerable in Debian 3.0.\nUpgrade to libc6.1-dev_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6.1-pic', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-pic is vulnerable in Debian 3.0.\nUpgrade to libc6.1-pic_2.2.5-11.5\n');
}
if (deb_check(prefix: 'libc6.1-prof', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package libc6.1-prof is vulnerable in Debian 3.0.\nUpgrade to libc6.1-prof_2.2.5-11.5\n');
}
if (deb_check(prefix: 'locales', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package locales is vulnerable in Debian 3.0.\nUpgrade to locales_2.2.5-11.5\n');
}
if (deb_check(prefix: 'nscd', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package nscd is vulnerable in Debian 3.0.\nUpgrade to nscd_2.2.5-11.5\n');
}
if (deb_check(prefix: 'glibc', release: '3.1', reference: '2.3.1-16')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package glibc is vulnerable in Debian 3.1.\nUpgrade to glibc_2.3.1-16\n');
}
if (deb_check(prefix: 'glibc', release: '2.2', reference: '2.1.3-25')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package glibc is vulnerable in Debian potato.\nUpgrade to glibc_2.1.3-25\n');
}
if (deb_check(prefix: 'glibc', release: '3.0', reference: '2.2.5-11.5')) {
 w ++;
 if (report_verbosity > 0) desc = strcat(desc, '\nThe package glibc is vulnerable in Debian woody.\nUpgrade to glibc_2.2.5-11.5\n');
}
if (w) { security_hole(port: 0, data: desc); }
