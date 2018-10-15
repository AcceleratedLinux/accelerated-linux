# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200504-25.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(18146);
 script_version("$Revision: 1.3 $");
 script_xref(name: "GLSA", value: "200504-25");
 script_cve_id("CVE-2005-1270");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200504-25
(Rootkit Hunter: Insecure temporary file creation)


    Sune Kloppenborg Jeppesen and Tavis Ormandy of the Gentoo Linux
    Security Team have reported that the check_update.sh script and the
    main rkhunter script insecurely creates several temporary files with
    predictable filenames.
  
Impact

    A local attacker could create symbolic links in the temporary
    files directory, pointing to a valid file somewhere on the filesystem.
    When rkhunter or the check_update.sh script runs, this would result in
    the file being overwritten with the rights of the user running the
    utility, which could be the root user.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-1270


Solution: 
    All Rootkit Hunter users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=app-forensics/rkhunter-1.2.3-r1"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200504-25] Rootkit Hunter: Insecure temporary file creation");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Rootkit Hunter: Insecure temporary file creation');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "app-forensics/rkhunter", unaffected: make_list("ge 1.2.3-r1"), vulnerable: make_list("lt 1.2.3-r1")
)) { security_warning(0); exit(0); }
