# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200608-19.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2006 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(22218);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200608-19");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200608-19
(WordPress: Privilege escalation)


    The WordPress developers have confirmed a vulnerability in capability
    checking for plugins.
  
Impact

    By exploiting a flaw, a user can circumvent WordPress access
    restrictions when using plugins. The actual impact depends on the
    configuration of WordPress and may range from trivial to critical,
    possibly even the execution of arbitrary PHP code.
  
Workaround

    There is no known workaround at this time.
  

Solution: 
    All WordPress users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=www-apps/wordpress-2.0.4"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200608-19] WordPress: Privilege escalation");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'WordPress: Privilege escalation');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "www-apps/wordpress", unaffected: make_list("ge 2.0.4"), vulnerable: make_list("lt 2.0.4")
)) { security_warning(0); exit(0); }
