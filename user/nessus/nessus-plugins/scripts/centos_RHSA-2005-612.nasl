#
# (C) Tenable Network Security, Inc.
#

if ( ! defined_func("bn_random") ) exit(0);
desc = "
Synopsis :

The remote host is missing a security update.

Description :

The remote CentOS system is missing a security update which has been 
documented in Red Hat advisory RHSA-2005-612.

See also :

https://rhn.redhat.com/errata/RHSA-2005-612.html

Solution :

Upgrade to the newest packages by doing :

  yum update

Risk factor :

High";

if ( description )
{
 script_id(21953);
 script_version("$Revision: 1.3 $");
 script_name(english:"CentOS : RHSA-2005-612");
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

if ( rpm_check(reference:"fetchmail-5.9.0-21.7.3.el2.1.1", release:"CentOS-4", cpu:"i386") )  faulty += '- fetchmail-5.9.0-21.7.3.el2.1.1\n';
if ( rpm_check(reference:"fetchmailconf-5.9.0-21.7.3.el2.1.1", release:"CentOS-4", cpu:"i386") )  faulty += '- fetchmailconf-5.9.0-21.7.3.el2.1.1\n';
if ( rpm_check(reference:"dhcpcd-1.3.20pl0-2", release:"CentOS-4", cpu:"i386") )  faulty += '- dhcpcd-1.3.20pl0-2\n';
if ( rpm_check(reference:"kdelibs-3.3.1-3.11", release:"CentOS-4", cpu:"i386") )  faulty += '- kdelibs-3.3.1-3.11\n';
if ( rpm_check(reference:"kdelibs-devel-3.3.1-3.11", release:"CentOS-4", cpu:"i386") )  faulty += '- kdelibs-devel-3.3.1-3.11\n';
if ( rpm_check(reference:"kdelibs-3.3.1-3.11", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kdelibs-3.3.1-3.11\n';
if ( rpm_check(reference:"kdelibs-devel-3.3.1-3.11", release:"CentOS-4", cpu:"x86_64") )  faulty += '- kdelibs-devel-3.3.1-3.11\n';
if ( rpm_check(reference:"kdelibs-3.3.1-3.11", release:"CentOS-4", cpu:"ia64") )  faulty += '- kdelibs-3.3.1-3.11\n';
if ( rpm_check(reference:"kdelibs-devel-3.3.1-3.11", release:"CentOS-4", cpu:"ia64") )  faulty += '- kdelibs-devel-3.3.1-3.11\n';
if ( faulty ) security_hole(port:0, data:desc + '\n\nPlugin output:\n\nThe following RPMs need to be updated :\n' + faulty);
