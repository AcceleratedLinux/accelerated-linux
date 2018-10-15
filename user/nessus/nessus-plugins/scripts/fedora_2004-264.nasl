#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(15474);
 script_version ("$Revision: 1.2 $");
 script_cve_id("CVE-2004-0755");
 
 name["english"] = "Fedora Core 2 2004-264: ruby";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2004-264 (ruby).

Ruby is the interpreted scripting language for quick and easy
object-oriented programming.  It has many features to process text
files and to do system management tasks (as in Perl).  It is simple,
straight-forward, and extensible.


* Thu Aug 19 2004 Akira TAGOH <tagoh@redhat.com> 1.8.1-6

- security fix [CVE-2004-0755]
- ruby-1.8.1-cgi_session_perms.patch: sets the permission of the session data
 file to 0600. (#130063)



Solution : http://www.fedoranews.org/updates/FEDORA-2004-264.shtml
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the ruby package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "Fedora Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"ruby-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"irb-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-debuginfo-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-devel-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-docs-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-libs-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-mode-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"ruby-tcltk-1.8.1-6", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"ruby-", release:"FC2") )
{
 set_kb_item(name:"CVE-2004-0755", value:TRUE);
}
