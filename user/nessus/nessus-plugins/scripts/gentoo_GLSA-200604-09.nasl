# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200604-09.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2006 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(21255);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200604-09");
 script_cve_id("CVE-2006-1721");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200604-09
(Cyrus-SASL: DIGEST-MD5 Pre-Authentication Denial of Service)


    Cyrus-SASL contains an unspecified vulnerability in the DIGEST-MD5
    process that could lead to a Denial of Service.
  
Impact

    An attacker could possibly exploit this vulnerability by sending
    specially crafted data stream to the Cyrus-SASL server, resulting in a
    Denial of Service even if the attacker is not able to authenticate.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2006-1721


Solution: 
    All Cyrus-SASL users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=dev-libs/cyrus-sasl-2.1.21-r2"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200604-09] Cyrus-SASL: DIGEST-MD5 Pre-Authentication Denial of Service");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Cyrus-SASL: DIGEST-MD5 Pre-Authentication Denial of Service');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "dev-libs/cyrus-sasl", unaffected: make_list("ge 2.1.21-r2"), vulnerable: make_list("lt 2.1.21-r2")
)) { security_warning(0); exit(0); }
