#
# (C) Tenable Network Security, Inc.
#

if ( ! defined_func("bn_random") ) exit(0);
desc = "
Synopsis :

The remote host is missing a security update.

Description :

The remote CentOS system is missing a security update which has been 
documented in Red Hat advisory RHSA-2006-0574.

See also :

https://rhn.redhat.com/errata/RHSA-2006-0574.html

Solution :

Upgrade to the newest packages by doing :

  yum update

Risk factor :

High";

if ( description )
{
 script_id(22038);
 script_version("$Revision: 1.2 $");
 script_name(english:"CentOS : RHSA-2006-0574");
 script_description(english:desc);

 script_summary(english:"Checks for missing updates on the remote CentOS system");
 script_category(ACT_GATHER_INFO);
 script_copyright(english:"This script is (C) 2006 Tenable Network Security, Inc.");
 script_family(english:"CentOS Local Security Checks");
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/CentOS/rpm-list");
 exit(0);
}

include("rpm.inc");

if ( rpm_check(reference:"kernel-doc-2.6.9-34.0.2.EL", release:"CentOS-4") )  faulty += '- kernel-doc-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i586") )  faulty += '- kernel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i586") )  faulty += '- kernel-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-hugemem-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-hugemem-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-hugemem-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-hugemem-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i586") )  faulty += '- kernel-smp-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-smp-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i586") )  faulty += '- kernel-smp-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"i686") )  faulty += '- kernel-smp-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-largesmp-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-largesmp-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-largesmp-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-largesmp-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-smp-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-smp-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kernel-smp-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"ia64") )  faulty += '- kernel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"ia64") )  faulty += '- kernel-devel-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-largesmp-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"ia64") )  faulty += '- kernel-largesmp-2.6.9-34.0.2.EL\n';
if ( rpm_check(reference:"kernel-largesmp-devel-2.6.9-34.0.2.EL", release:"CentOS-4", cpu:"ia64") )  faulty += '- kernel-largesmp-devel-2.6.9-34.0.2.EL\n';
if ( faulty ) security_hole(port:0, data:desc + '\n\nPlugin output:\n\nThe following RPMs need to be updated :\n' + faulty);
