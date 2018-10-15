# This script was automatically generated from the 20-1 Ubuntu Security Notice
# It is released under the Nessus Script Licence.
# Ubuntu Security Notices are (C) 2005 Canonical, Inc.
# USN2nasl Convertor is (C) 2005 Michel Arboi
# See http://www.ubuntulinux.org/usn/
# Ubuntu(R) is a registered trademark of Canonical, Inc.

if (! defined_func("bn_random")) exit(0);
desc = '
Synopsis :

These remote packages are missing security patches :
- irb1.8 
- libbigdecimal-ruby1.8 
- libcurses-ruby1.8 
- libdbm-ruby1.8 
- libdl-ruby1.8 
- libdrb-ruby1.8 
- liberb-ruby1.8 
- libgdbm-ruby1.8 
- libiconv-ruby1.8 
- libopenssl-ruby1.8 
- libpty-ruby1.8 
- libracc-runtime-ruby1.8 
- libreadline-ruby1.8 
- librexml-ruby1.8 
- libruby1.8 
- libruby1.8-dbg 
- libsdbm-ruby1.8 
- libsoap-ruby1.8 
- libstrscan-ruby1.8 
- libsyslog-ruby1.8 
- libtcltk-ruby1.8 
- libtest-unit-ruby1.8 
- libtk-ruby1.8 
- libweb
[...]

Description :

The Ruby developers discovered a potential Denial of Service
vulnerability in the CGI module (cgi.rb). Specially crafted CGI
requests could cause an infinite loop in the server process.
Repetitive attacks could use most of the available processor
resources, exhaust the number of allowed parallel connections in web
servers, or cause similar effects which render the service
unavailable.

There is no possibility of privilege escalation or data loss.

Solution :

Upgrade to : 
- irb1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libbigdecimal-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libcurses-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libdbm-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libdl-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libdrb-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- liberb-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libgdbm-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1 (Ubuntu 4.10)
- libiconv-ruby1.8-1.8.1+1.8.2p
[...]


Risk factor : High
';

if (description) {
script_id(20615);
script_version("$Revision: 1.3 $");
script_copyright("Ubuntu Security Notice (C) 2005 Canonical, Inc. / NASL script (C) 2005 Michel Arboi <mikhail@nessus.org>");
script_category(ACT_GATHER_INFO);
script_family(english: "Ubuntu Local Security Checks");
script_dependencies("ssh_get_info.nasl");
script_require_keys("Host/Ubuntu", "Host/Ubuntu/release", "Host/Debian/dpkg-l");
script_description(english: desc);

script_xref(name: "USN", value: "20-1");
script_summary(english:"ruby1.8 vulnerability");
script_name(english:"USN20-1 : ruby1.8 vulnerability");
script_cve_id("CVE-2004-0983");
exit(0);
}

include('ubuntu.inc');

found = ubuntu_check(osver: "4.10", pkgname: "irb1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package irb1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to irb1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libbigdecimal-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libbigdecimal-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libbigdecimal-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libcurses-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libcurses-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libcurses-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libdbm-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libdbm-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libdbm-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libdl-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libdl-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libdl-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libdrb-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libdrb-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libdrb-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "liberb-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package liberb-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to liberb-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libgdbm-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libgdbm-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libgdbm-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libiconv-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libiconv-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libiconv-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libopenssl-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libopenssl-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libopenssl-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libpty-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libpty-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libpty-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libracc-runtime-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libracc-runtime-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libracc-runtime-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libreadline-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libreadline-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libreadline-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "librexml-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package librexml-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to librexml-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libruby1.8-dbg", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libruby1.8-dbg-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libruby1.8-dbg-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libsdbm-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libsdbm-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libsdbm-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libsoap-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libsoap-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libsoap-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libstrscan-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libstrscan-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libstrscan-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libsyslog-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libsyslog-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libsyslog-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libtcltk-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtcltk-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtcltk-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libtest-unit-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtest-unit-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtest-unit-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libtk-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libtk-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libtk-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libwebrick-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libwebrick-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libwebrick-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libxmlrpc-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libxmlrpc-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libxmlrpc-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libyaml-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libyaml-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libyaml-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "libzlib-ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package libzlib-ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to libzlib-ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "rdoc1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package rdoc1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to rdoc1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "ri1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package ri1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to ri1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "ruby1.8", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package ruby1.8-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to ruby1.8-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "ruby1.8-dev", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package ruby1.8-dev-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to ruby1.8-dev-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "ruby1.8-elisp", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package ruby1.8-elisp-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to ruby1.8-elisp-1.8.1+1.8.2pre2-3ubuntu0.1
');
}
found = ubuntu_check(osver: "4.10", pkgname: "ruby1.8-examples", pkgver: "1.8.1+1.8.2pre2-3ubuntu0.1");
if (! isnull(found)) {
w++;
desc = strcat(desc, '
The package ruby1.8-examples-',found,' is vulnerable in Ubuntu 4.10
Upgrade it to ruby1.8-examples-1.8.1+1.8.2pre2-3ubuntu0.1
');
}

if (w) { security_hole(port: 0, data: desc); }
