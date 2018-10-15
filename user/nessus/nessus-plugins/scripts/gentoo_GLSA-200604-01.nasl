# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200604-01.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2006 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(21194);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200604-01");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200604-01
(MediaWiki: Cross-site scripting vulnerability)


    MediaWiki fails to decode certain encoded URLs correctly.
  
Impact

    By supplying specially crafted links, a remote attacker could
    exploit this vulnerability to inject malicious HTML or JavaScript code
    that will be executed in a user\'s browser session in the context of the
    vulnerable site.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2006-1498
    http://sourceforge.net/project/shownotes.php?release_id=404869


Solution: 
    All MediaWiki users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=www-apps/mediawiki-1.4.15"
  

Risk factor : Low
';
 script_description(english: desc);
 script_copyright(english: "(C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200604-01] MediaWiki: Cross-site scripting vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'MediaWiki: Cross-site scripting vulnerability');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "www-apps/mediawiki", unaffected: make_list("ge 1.4.15"), vulnerable: make_list("lt 1.4.15")
)) { security_warning(0); exit(0); }
