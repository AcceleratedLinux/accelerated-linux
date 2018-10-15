#
# (C) Tenable Network Security
#
#
# The text of this plugin is (C) Red Hat Inc.

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(19989);
 script_version ("$Revision: 1.2 $");
 script_cve_id("CVE-2005-0756", "CVE-2005-1265", "CVE-2005-1761", "CVE-2005-1762", "CVE-2005-1763", "CVE-2005-2098", "CVE-2005-2099", "CVE-2005-2100", "CVE-2005-2456", "CVE-2005-2490", "CVE-2005-2492", "CVE-2005-2555", "CVE-2005-2801", "CVE-2005-2872");

 name["english"] = "RHSA-2005-514:   kernel";
 
 script_name(english:name["english"]);
 
 desc["english"] = '

  Updated kernel packages are now available as part of ongoing support
  and maintenance of Red Hat Enterprise Linux version 4. This is the
  second regular update.

  This update has been rated as having important security impact by the
  Red Hat Security Response Team.


Solution : http://rhn.redhat.com/errata/RHSA-2005-514.html
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the   kernel packages";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "Red Hat Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"kernel-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-devel-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-doc-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-hugemem-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-hugemem-devel-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-smp-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"kernel-smp-devel-2.6.9-22.EL", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}

if ( rpm_exists(rpm:"  kernel-", release:"RHEL4") )
{
 set_kb_item(name:"CVE-2005-0756", value:TRUE);
 set_kb_item(name:"CVE-2005-1265", value:TRUE);
 set_kb_item(name:"CVE-2005-1761", value:TRUE);
 set_kb_item(name:"CVE-2005-1762", value:TRUE);
 set_kb_item(name:"CVE-2005-1763", value:TRUE);
 set_kb_item(name:"CVE-2005-2098", value:TRUE);
 set_kb_item(name:"CVE-2005-2099", value:TRUE);
 set_kb_item(name:"CVE-2005-2100", value:TRUE);
 set_kb_item(name:"CVE-2005-2456", value:TRUE);
 set_kb_item(name:"CVE-2005-2490", value:TRUE);
 set_kb_item(name:"CVE-2005-2492", value:TRUE);
 set_kb_item(name:"CVE-2005-2555", value:TRUE);
 set_kb_item(name:"CVE-2005-2801", value:TRUE);
 set_kb_item(name:"CVE-2005-2872", value:TRUE);
}

set_kb_item(name:"RHSA-2005-514", value:TRUE);
