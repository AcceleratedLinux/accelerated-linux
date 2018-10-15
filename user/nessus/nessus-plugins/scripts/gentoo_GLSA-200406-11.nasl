# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200406-11.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(14522);
 script_version("$Revision: 1.2 $");
 script_xref(name: "GLSA", value: "200406-11");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200406-11
(Horde-IMP: Input validation vulnerability)


    Horde-IMP fails to properly sanitize email messages that contain malicious
    HTML or script code.
  
Impact

    By enticing a user to read a specially crafted e-mail, an attacker can
    execute arbitrary scripts running in the context of the victim\'s browser.
    This could lead to a compromise of the user\'s webmail account, cookie
    theft, etc.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://www.securityfocus.com/bid/10501


Solution: 
    All Horde-IMP users should upgrade to the latest stable version:
    # emerge sync
    # emerge -pv ">=horde-imp-3.2.4"
    # emerge ">=horde-imp-3.2.4"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200406-11] Horde-IMP: Input validation vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Horde-IMP: Input validation vulnerability');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-www/horde-imp", unaffected: make_list("ge 3.2.4"), vulnerable: make_list("le 3.2.3")
)) { security_warning(0); exit(0); }
