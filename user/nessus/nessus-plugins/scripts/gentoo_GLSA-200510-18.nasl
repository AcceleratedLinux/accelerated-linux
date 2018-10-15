# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200510-18.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(20080);
 script_version("$Revision: 1.1 $");
 script_xref(name: "GLSA", value: "200510-18");
 script_cve_id("CAN-2005-2978");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200510-18
(Netpbm: Buffer overflow in pnmtopng)


    RedHat reported that pnmtopng is vulnerable to a buffer overflow.
  
Impact

    An attacker could craft a malicious PNM file and entice a user to
    run pnmtopng on it, potentially resulting in the execution of arbitrary
    code with the permissions of the user running pnmtopng.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2005-2978


Solution: 
    All Netpbm users should upgrade to the latest version:
    # emerge --sync
    # emerge --ask --oneshot --verbose ">=media-libs/netpbm-10.29"
  

Risk factor : Medium
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200510-18] Netpbm: Buffer overflow in pnmtopng");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'Netpbm: Buffer overflow in pnmtopng');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "media-libs/netpbm", unaffected: make_list("ge 10.29"), vulnerable: make_list("lt 10.29")
)) { security_warning(0); exit(0); }
