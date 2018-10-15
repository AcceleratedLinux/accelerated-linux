# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200507-25.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(19327);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200507-25");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200507-25
(Clam AntiVirus: Integer overflows)


    Neel Mehta and Alex Wheeler discovered that Clam AntiVirus is
    vulnerable to integer overflows when handling the TNEF, CHM and FSG
    file formats.
  
Impact

    By sending a specially-crafted file an attacker could execute
    arbitrary code with the permissions of the user running Clam AntiVirus.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://www.securityfocus.com/archive/1/406377/30/
    http://sourceforge.net/project/shownotes.php?release_id=344514


Solution: 
    All Clam AntiVirus users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=app-antivirus/clamav-0.86.2"
  

Risk factor : High
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200507-25] Clam AntiVirus: Integer overflows");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Clam AntiVirus: Integer overflows');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "app-antivirus/clamav", unaffected: make_list("ge 0.86.2"), vulnerable: make_list("lt 0.86.2")
)) { security_hole(0); exit(0); }
