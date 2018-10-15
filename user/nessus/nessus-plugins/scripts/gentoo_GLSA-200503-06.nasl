# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200503-06.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(17262);
 script_version("$Revision: 1.3 $");
 script_xref(name: "GLSA", value: "200503-06");
 script_cve_id("CVE-2005-0158");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200503-06
(BidWatcher: Format string vulnerability)


    Ulf Harnhammar discovered a format string vulnerability in
    "netstuff.cpp".
  
Impact

    Remote attackers can potentially exploit this vulnerability by
    sending specially crafted responses via an eBay HTTP server or a
    man-in-the-middle attack to execute arbitrary malicious code.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-0158


Solution: 
    All BidWatcher users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=net-misc/bidwatcher-1.13.17"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200503-06] BidWatcher: Format string vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'BidWatcher: Format string vulnerability');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-misc/bidwatcher", unaffected: make_list("ge 1.3.17"), vulnerable: make_list("lt 1.3.17")
)) { security_warning(0); exit(0); }
