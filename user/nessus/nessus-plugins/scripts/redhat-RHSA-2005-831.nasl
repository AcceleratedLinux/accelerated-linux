#
# (C) Tenable Network Security
#
#
# The text of this plugin is (C) Red Hat Inc.

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(20206);
 script_version ("$Revision: 1.1 $");
 script_cve_id("CVE-2005-3353", "CVE-2005-3388", "CVE-2005-3389", "CVE-2005-3390");

 name["english"] = "RHSA-2005-831: php";
 
 script_name(english:name["english"]);
 
 desc["english"] = '

  Updated PHP packages that fix multiple security issues are now available
  for Red Hat Enterprise Linux 3 and 4.

  This update has been rated as having moderate security impact by the Red
  Hat Security Response Team.

  PHP is an HTML-embedded scripting language commonly used with the Apache
  HTTP Web server.

  A flaw was found in the way PHP registers global variables during a file
  upload request. A remote attacker could submit a carefully crafted
  multipart/form-data POST request that would overwrite the $GLOBALS array,
  altering expected script behavior, and possibly leading to the execution of
  arbitrary PHP commands. Please note that this vulnerability only affects
  installations which have register_globals enabled in the PHP configuration
  file, which is not a default or recommended option. The Common
  Vulnerabilities and Exposures project assigned the name CVE-2005-3390 to
  this issue.

  A flaw was found in the PHP parse_str() function. If a PHP script passes
  only one argument to the parse_str() function, and the script can be forced
  to abort execution during operation (for example due to the memory_limit
  setting), the register_globals may be enabled even if it is disabled in the
  PHP configuration file. This vulnerability only affects installations that
  have PHP scripts using the parse_str function in this way. (CVE-2005-3389)

  A Cross-Site Scripting flaw was found in the phpinfo() function. If a
  victim can be tricked into following a malicious URL to a site with a page
  displaying the phpinfo() output, it may be possible to inject javascript
  or HTML content into the displayed page or steal data such as cookies.
  This vulnerability only affects installations which allow users to view the
  output of the phpinfo() function. As the phpinfo() function outputs a
  large amount of information about the current state of PHP, it should only
  be used during debugging or if protected by authentication. (CVE-2005-3388)

  A denial of service flaw was found in the way PHP processes EXIF image
  data. It is possible for an attacker to cause PHP to crash by supplying
  carefully crafted EXIF image data. (CVE-2005-3353)

  Users of PHP should upgrade to these updated packages, which contain
  backported patches that resolve these issues.




Solution : http://rhn.redhat.com/errata/RHSA-2005-831.html
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the php packages";
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
if ( rpm_check( reference:"php-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-devel-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-imap-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-ldap-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-mysql-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-odbc-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-pgsql-4.3.2-26.ent", release:"RHEL3") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-devel-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-domxml-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-gd-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-imap-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-ldap-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-mbstring-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-mysql-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-ncurses-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-odbc-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-pear-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-pgsql-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-snmp-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"php-xmlrpc-4.3.9-3.9", release:"RHEL4") )
{
 security_hole(0);
 exit(0);
}

if ( rpm_exists(rpm:"php-", release:"RHEL3") )
{
 set_kb_item(name:"CVE-2005-3353", value:TRUE);
 set_kb_item(name:"CVE-2005-3388", value:TRUE);
 set_kb_item(name:"CVE-2005-3389", value:TRUE);
 set_kb_item(name:"CVE-2005-3390", value:TRUE);
}
if ( rpm_exists(rpm:"php-", release:"RHEL4") )
{
 set_kb_item(name:"CVE-2005-3353", value:TRUE);
 set_kb_item(name:"CVE-2005-3388", value:TRUE);
 set_kb_item(name:"CVE-2005-3389", value:TRUE);
 set_kb_item(name:"CVE-2005-3390", value:TRUE);
}

set_kb_item(name:"RHSA-2005-831", value:TRUE);
