# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200403-11.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(14462);
 script_version("$Revision: 1.3 $");
 script_xref(name: "GLSA", value: "200403-11");
 script_cve_id("CVE-2004-0189");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200403-11
(Squid ACL [url_regex] bypass vulnerability)


    A bug in Squid allows users to bypass certain access controls by passing a
    URL containing "%00" which exploits the Squid decoding function.
    This may insert a NUL character into decoded URLs, which may allow users to
    bypass url_regex access control lists that are enforced upon them.
    In such a scenario, Squid will insert a NUL character after
    the"%00" and it will make a comparison between the URL to the end
    of the NUL character rather than the contents after it: the comparison does
    not result in a match, and the user\'s request is not denied.
  
Impact

    Restricted users may be able to bypass url_regex access control lists that
    are enforced upon them which may cause unwanted network traffic as well as
    a route for other possible exploits. Users of Squid 2.5STABLE4 and below
    who require the url_regex features are recommended to upgrade to 2.5STABLE5
    to maintain the security of their infrastructure.
  
Workaround

    A workaround is not currently known for this issue. All users are advised
    to upgrade to the latest version of Squid.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2004-0189
    http://www.squid-cache.org/Advisories/SQUID-2004_1.txt


Solution: 
    Squid can be updated as follows:
    # emerge sync
    # emerge -pv ">=www-proxy/squid-2.5.5"
    # emerge ">=www-proxy/squid-2.5.5"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200403-11] Squid ACL [url_regex] bypass vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Squid ACL [url_regex] bypass vulnerability');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "www-proxy/squid", unaffected: make_list("ge 2.5.5"), vulnerable: make_list("lt 2.5.5")
)) { security_warning(0); exit(0); }
