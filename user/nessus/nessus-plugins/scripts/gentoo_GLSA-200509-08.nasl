# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200509-08.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(19687);
 script_version("$Revision: 1.2 $");
 script_xref(name: "GLSA", value: "200509-08");
 script_cve_id("CVE-2005-2491");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200509-08
(Python: Heap overflow in the included PCRE library)


    The "re" Python module makes use of a private copy of libpcre
    which is subject to an integer overflow leading to a heap overflow (see
    GLSA 200508-17).
  
Impact

    An attacker could target a Python-based web application (or SUID
    application) that would use untrusted data as regular expressions,
    potentially resulting in the execution of arbitrary code (or privilege
    escalation).
  
Workaround

    Python users that don\'t run any Python web application or SUID
    application (or that run one that wouldn\'t use untrusted inputs as
    regular expressions) are not affected by this issue.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-2491
    http://www.gentoo.org/security/en/glsa/glsa-200508-17.xml


Solution: 
    All Python users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=dev-lang/python-2.3.5-r2"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200509-08] Python: Heap overflow in the included PCRE library");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Python: Heap overflow in the included PCRE library');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "dev-lang/python", unaffected: make_list("ge 2.3.5-r2"), vulnerable: make_list("lt 2.3.5-r2")
)) { security_warning(0); exit(0); }
