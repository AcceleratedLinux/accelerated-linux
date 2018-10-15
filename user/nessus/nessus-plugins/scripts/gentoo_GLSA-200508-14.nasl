# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200508-14.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(19534);
 script_version("$Revision: 1.2 $");
 script_xref(name: "GLSA", value: "200508-14");
 script_cve_id("CVE-2005-2498");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200508-14
(TikiWiki, eGroupWare: Arbitrary command execution through XML-RPC)


    The XML-RPC library shipped in TikiWiki and eGroupWare improperly
    handles XML-RPC requests and responses with malformed nested tags.
  
Impact

    A remote attacker could exploit this vulnerability to inject
    arbitrary PHP script code into eval() statements by sending a specially
    crafted XML document to TikiWiki or eGroupWare.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-2498


Solution: 
    All TikiWiki users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=www-apps/tikiwiki-1.8.5-r2"
    All eGroupWare users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=www-apps/egroupware-1.0.0.009"
  

Risk factor : High
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200508-14] TikiWiki, eGroupWare: Arbitrary command execution through XML-RPC");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'TikiWiki, eGroupWare: Arbitrary command execution through XML-RPC');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "www-apps/egroupware", unaffected: make_list("ge 1.0.0.009"), vulnerable: make_list("lt 1.0.0.009")
)) { security_hole(0); exit(0); }
if (qpkg_check(package: "www-apps/tikiwiki", unaffected: make_list("ge 1.8.5-r2"), vulnerable: make_list("lt 1.8.5-r2")
)) { security_hole(0); exit(0); }
