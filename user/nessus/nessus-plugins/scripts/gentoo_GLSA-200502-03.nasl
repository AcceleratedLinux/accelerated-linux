# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200502-03.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(16440);
 script_version("$Revision: 1.3 $");
 script_xref(name: "GLSA", value: "200502-03");
 script_cve_id("CVE-2004-1184", "CVE-2004-1185", "CVE-2004-1186");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200502-03
(enscript: Multiple vulnerabilities)


    Erik Sjolund discovered several issues in enscript: it suffers
    from several buffer overflows (CVE-2004-1186), quotes and shell escape
    characters are insufficiently sanitized in filenames (CVE-2004-1185),
    and it supported taking input from an arbitrary command pipe, with
    unwanted side effects (CVE-2004-1184).
  
Impact

    An attacker could design malicious files or input data which, once
    feeded into enscript, would trigger the execution of arbitrary code
    with the rights of the user running enscript.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-1184
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-1185
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-1186


Solution: 
    All enscript users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=app-text/enscript-1.6.3-r3"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200502-03] enscript: Multiple vulnerabilities");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'enscript: Multiple vulnerabilities');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "app-text/enscript", unaffected: make_list("ge 1.6.3-r3"), vulnerable: make_list("lt 1.6.3-r3")
)) { security_warning(0); exit(0); }
