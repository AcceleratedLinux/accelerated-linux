# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200508-05.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(19389);
 script_version("$Revision: 1.2 $");
 script_xref(name: "GLSA", value: "200508-05");
 script_cve_id("CVE-2005-2231");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200508-05
(Heartbeat: Insecure temporary file creation)


    Eric Romang has discovered that Heartbeat insecurely creates
    temporary files with predictable filenames.
  
Impact

    A local attacker could create symbolic links in the temporary file
    directory, pointing to a valid file somewhere on the filesystem. When a
    vulnerable script is executed, this could lead to the file being
    overwritten with the rights of the user running the affected
    application.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2005-2231


Solution: 
    All Heartbeat users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=sys-cluster/heartbeat-1.2.3-r1"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200508-05] Heartbeat: Insecure temporary file creation");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Heartbeat: Insecure temporary file creation');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "sys-cluster/heartbeat", unaffected: make_list("ge 1.2.3-r1"), vulnerable: make_list("lt 1.2.3-r1")
)) { security_warning(0); exit(0); }
