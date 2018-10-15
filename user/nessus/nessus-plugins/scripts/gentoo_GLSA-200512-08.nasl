# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200512-08.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(20328);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200512-08");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200512-08
(Xpdf, GPdf, CUPS, Poppler: Multiple vulnerabilities)


    infamous41md discovered that several Xpdf functions lack sufficient
    boundary checking, resulting in multiple exploitable buffer overflows.
  
Impact

    An attacker could entice a user to open a specially-crafted PDF file
    which would trigger an overflow, potentially resulting in execution of
    arbitrary code with the rights of the user running Xpdf, CUPS, GPdf or
    Poppler.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-3191
    http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-3192
    http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-3193


Solution: 
    All Xpdf users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=app-text/xpdf-3.01-r2"
    All GPdf users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=app-text/gpdf-2.10.0-r2"
    All Poppler users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose app-text/poppler
    All CUPS users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=net-print/cups-1.1.23-r3"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200512-08] Xpdf, GPdf, CUPS, Poppler: Multiple vulnerabilities");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Xpdf, GPdf, CUPS, Poppler: Multiple vulnerabilities');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-print/cups", unaffected: make_list("ge 1.1.23-r3"), vulnerable: make_list("lt 1.1.23-r3")
)) { security_warning(0); exit(0); }
if (qpkg_check(package: "app-text/xpdf", unaffected: make_list("ge 3.01-r2"), vulnerable: make_list("lt 3.01-r2")
)) { security_warning(0); exit(0); }
if (qpkg_check(package: "app-text/poppler", unaffected: make_list("ge 0.4.2-r1", "rge 0.3.0-r1"), vulnerable: make_list("lt 0.4.2-r1")
)) { security_warning(0); exit(0); }
if (qpkg_check(package: "app-text/gpdf", unaffected: make_list("ge 2.10.0-r2"), vulnerable: make_list("lt 2.10.0-r2")
)) { security_warning(0); exit(0); }
