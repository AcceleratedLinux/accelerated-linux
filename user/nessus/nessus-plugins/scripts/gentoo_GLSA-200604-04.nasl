# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200604-04.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2006 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(21197);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200604-04");
 script_cve_id("CVE-2006-0051");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200604-04
(Kaffeine: Buffer overflow)


    Kaffeine uses an unchecked buffer when fetching remote RAM
    playlists via HTTP.
  
Impact

    A remote attacker could entice a user to play a specially-crafted
    RAM playlist resulting in the execution of arbitrary code with the
    permissions of the user running the application.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2006-0051
    http://www.kde.org/info/security/advisory-20060404-1.txt


Solution: 
    All Kaffeine users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=media-video/kaffeine-0.7.1-r2"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200604-04] Kaffeine: Buffer overflow");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Kaffeine: Buffer overflow');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "media-video/kaffeine", unaffected: make_list("ge 0.7.1-r2"), vulnerable: make_list("lt 0.7.1-r2")
)) { security_warning(0); exit(0); }
