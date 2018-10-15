# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200601-15.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2006 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(20823);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200601-15");
 script_cve_id("CVE-2005-3280");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200601-15
(Paros: Default administrator password)


    Andrew Christensen discovered that in older versions of Paros the
    database component HSQLDB is installed with an empty password for the
    database administrator "sa".
  
Impact

    Since the database listens globally by default, an attacker can
    connect and issue arbitrary commands, including execution of binaries
    installed on the host.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-3280


Solution: 
    All Paros users should upgrade to the latest version:
    # emerge --snyc
    # emerge --ask --oneshot --verbose ">=net-proxy/paros-3.2.8"
  

Risk factor : High
';
 script_description(english: desc);
 script_copyright(english: "(C) 2006 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200601-15] Paros: Default administrator password");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Paros: Default administrator password');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-proxy/paros", unaffected: make_list("gt 3.2.5"), vulnerable: make_list("le 3.2.5")
)) { security_hole(0); exit(0); }
