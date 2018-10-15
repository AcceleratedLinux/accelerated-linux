#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Script License for details
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Fixed broken link
#


if(description)
{
 script_id(10974);
 script_bugtraq_id(703);
 script_version("$Revision: 1.5 $");
 script_cve_id("CVE-1999-0161");

 name["english"] = "CSCdi36962";

 script_name(english:name["english"]);

 desc["english"] = "

The remote seems to be vulnerable to a flaw in IOS when
the keyword 'tacacs-ds' or 'tacacs' is being used in
extended ACLs.

This bug can, under very specific circumstances and only with
certain IP host implementations, allow unauthorized packets to
circumvent a filtering router.

This vulnerability is documented as Cisco Bug ID CSCdi36962.

Solution : http://www.cisco.com/warp/public/707/1.html
Risk factor : High

*** As Nessus solely relied on the banner of the remote host
*** this might be a false positive
";
 script_description(english:desc["english"]);

 summary["english"] = "Uses SNMP to determine if a flaw is present";
 script_summary(english:summary["english"]);

 script_category(ACT_GATHER_INFO);

 script_copyright(english:"This script is (C) 2002 Renaud Deraison");

 script_family(english:"CISCO");

 script_dependencie("snmp_sysDesc.nasl",
			 "snmp_cisco_type.nasl");
 script_require_keys("SNMP/community",
			  "SNMP/sysDesc",
			  "CISCO/model");
 exit(0);
}


# The code starts here
ok=0;
os = get_kb_item("SNMP/sysDesc"); if(!os)exit(0);
hardware = get_kb_item("CISCO/model"); if(!hardware)exit(0);




# Check for the required operating system...
#----------------------------------------------------------------
# Is this IOS ?
if(!egrep(pattern:".*(Internetwork Operating|IOS).*", string:os))exit(0);
# 10.3
if(egrep(string:os, pattern:"(10\.3\([0-4]\)|10\.3),"))ok=1;


#----------------------------------------------

if(ok)security_hole(port:161, proto:"udp");
