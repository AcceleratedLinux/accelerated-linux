#
# (C) Tenable Network Security
#
# This plugin text was extracted from Mandrake Linux Security Advisory MDKSA-2005:219
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(20450);
 script_version ("$Revision: 1.1 $");
 script_cve_id("CVE-2004-1333", "CVE-2004-2302", "CVE-2005-0180", "CVE-2005-0210", "CVE-2005-1589", "CVE-2005-2456", "CVE-2005-2457", "CVE-2005-2458", "CVE-2005-2459", "CVE-2005-2490", "CVE-2005-2548", "CVE-2005-2555", "CVE-2005-2800", "CVE-2005-2801", "CVE-2005-2872", "CVE-2005-2873", "CVE-2005-3044", "CVE-2005-3053", "CVE-2005-3055", "CVE-2005-3180", "CVE-2005-3181", "CVE-2005-3257", "CVE-2005-3271", "CVE-2005-3273", "CVE-2005-3274", "CVE-2005-3275", "CVE-2005-3276");
 
 name["english"] = "MDKSA-2005:219: kernel";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory MDKSA-2005:219 (kernel).



Multiple vulnerabilities in the Linux 2.6 kernel have been discovered and
corrected in this update. 

Solution : http://wwwnew.mandriva.com/security/advisories?name=MDKSA-2005:219
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the kernel package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");
 family["english"] = "Mandrake Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Mandrake/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"kernel-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-enterprise-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-i586-up-1GB-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-i686-up-64GB-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-secure-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-smp-2.6.8.1.26mdk-1-1mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-source-2.6-2.6.8.1-26mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-source-stripped-2.6-2.6.8.1-26mdk", release:"MDK10.1", yank:"mdk") )
{
 security_hole(0);
 exit(0);
}
if (rpm_exists(rpm:"kernel-", release:"MDK10.1") )
{
 set_kb_item(name:"CVE-2004-1333", value:TRUE);
 set_kb_item(name:"CVE-2004-2302", value:TRUE);
 set_kb_item(name:"CVE-2005-0180", value:TRUE);
 set_kb_item(name:"CVE-2005-0210", value:TRUE);
 set_kb_item(name:"CVE-2005-1589", value:TRUE);
 set_kb_item(name:"CVE-2005-2456", value:TRUE);
 set_kb_item(name:"CVE-2005-2457", value:TRUE);
 set_kb_item(name:"CVE-2005-2458", value:TRUE);
 set_kb_item(name:"CVE-2005-2459", value:TRUE);
 set_kb_item(name:"CVE-2005-2490", value:TRUE);
 set_kb_item(name:"CVE-2005-2548", value:TRUE);
 set_kb_item(name:"CVE-2005-2555", value:TRUE);
 set_kb_item(name:"CVE-2005-2800", value:TRUE);
 set_kb_item(name:"CVE-2005-2801", value:TRUE);
 set_kb_item(name:"CVE-2005-2872", value:TRUE);
 set_kb_item(name:"CVE-2005-2873", value:TRUE);
 set_kb_item(name:"CVE-2005-3044", value:TRUE);
 set_kb_item(name:"CVE-2005-3053", value:TRUE);
 set_kb_item(name:"CVE-2005-3055", value:TRUE);
 set_kb_item(name:"CVE-2005-3180", value:TRUE);
 set_kb_item(name:"CVE-2005-3181", value:TRUE);
 set_kb_item(name:"CVE-2005-3257", value:TRUE);
 set_kb_item(name:"CVE-2005-3271", value:TRUE);
 set_kb_item(name:"CVE-2005-3273", value:TRUE);
 set_kb_item(name:"CVE-2005-3274", value:TRUE);
 set_kb_item(name:"CVE-2005-3275", value:TRUE);
 set_kb_item(name:"CVE-2005-3276", value:TRUE);
}
