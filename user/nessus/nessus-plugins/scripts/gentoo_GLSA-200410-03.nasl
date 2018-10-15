# This script was automatically generated from 
#  http://www.gentoo.org/security/en/glsa/glsa-200410-03.xml
# It is released under the Nessus Script Licence.
# The messages are release under the Creative Commons - Attribution /
# Share Alike license. See http://creativecommons.org/licenses/by-sa/2.0/
#
# Avisory is copyright 2001-2005 Gentoo Foundation, Inc.
# GLSA2nasl Convertor is copyright 2004 Michel Arboi <mikhail@nessus.org>

if (! defined_func('bn_random')) exit(0);

if (description)
{
 script_id(15424);
 if(defined_func("script_xref"))script_xref(name:"IAVA", value:"2001-t-0008");
 script_version("$Revision: 1.3 $");
 script_xref(name: "GLSA", value: "200410-03");

 desc = 'The remote host is affected by the vulnerability described in GLSA-200410-03
(NetKit-telnetd: buffer overflows in telnet and telnetd)


    A possible buffer overflow exists in the parsing of option strings by the
    telnet daemon, where proper bounds checking is not applied when writing to
    a buffer. Additionaly, another possible buffer overflow has been found by
    Josh Martin in the handling of the environment variable HOME.
  
Impact

    A remote attacker sending a specially-crafted options string to the telnet
    daemon could be able to run arbitrary code with the privileges of the user
    running the telnet daemon, usually root. Furthermore, an attacker could
    make use of an overlong HOME variable to cause a buffer overflow in the
    telnet client, potentially leading to the local execution of arbitrary
    code.
  
Workaround

    There is no known workaround at this time.
  
References:
    http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2001-0554
    http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=264846


Solution: 
    All NetKit-telnetd users should upgrade to the latest version:
    # emerge sync
    # emerge -pv ">=net-misc/netkit-telnetd-0.17-r4"
    # emerge ">=net-misc/netkit-telnetd-0.17-r4"
  

Risk factor : High
';
 script_description(english: desc);
 script_copyright(english: "(C) 2005 Michel Arboi <mikhail@nessus.org>");
 script_name(english: "[GLSA-200410-03] NetKit-telnetd: buffer overflows in telnet and telnetd");
 script_category(ACT_GATHER_INFO);
 script_family(english: "Gentoo Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys('Host/Gentoo/qpkg-list');
 script_summary(english: 'NetKit-telnetd: buffer overflows in telnet and telnetd');
 exit(0);
}

include('qpkg.inc');
if (qpkg_check(package: "net-misc/netkit-telnetd", unaffected: make_list("ge 0.17-r4"), vulnerable: make_list("le 0.17-r3")
)) { security_hole(0); exit(0); }
