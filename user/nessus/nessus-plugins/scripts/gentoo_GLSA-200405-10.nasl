# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200405-10.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(14496);
 script_version("$Revision: 1.2 $");
 script_xref(name: "GLSA", value: "200405-10");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200405-10
(Icecast denial of service vulnerability)


    There is an out-of-bounds read error in the web interface of Icecast when
    handling Basic Authorization requests. This vulnerability can theorically
    be exploited by sending a specially crafted Authorization header to the
    server.
  
Impact

    By exploiting this vulnerability, it is possible to crash the Icecast
    server remotely, resulting in a denial of service attack.
  
Workaround

    There is no known workaround at this time. All users are advised to upgrade
    to the latest available version of Icecast.
  
References:
    http://www.xiph.org/archives/icecast/7144.html


Solution: 
    All users of Icecast should upgrade to the latest stable version:
    # emerge sync
    # emerge -pv ">=net-misc/icecast-2.0.1"
    # emerge ">=net-misc/icecast-2.0.1"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200405-10] Icecast denial of service vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Icecast denial of service vulnerability');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-misc/icecast", unaffected: make_list("ge 2.0.1"), vulnerable: make_list("le 2.0.0")
)) { security_warning(0); exit(0); }
