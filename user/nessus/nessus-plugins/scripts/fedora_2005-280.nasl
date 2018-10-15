#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(18331);
 script_version ("$Revision: 1.2 $");
 script_cve_id("CVE-2004-1772");
 
 name["english"] = "Fedora Core 2 2005-280: sharutils";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2005-280 (sharutils).

The sharutils package contains the GNU shar utilities, a set of tools
for encoding and decoding packages of files (in binary or text format)
in a special plain text format called shell archives (shar). This
format can be sent through e-mail (which can be problematic for
regular
binary files). The shar utility supports a wide range of capabilities
(compressing, uuencoding, splitting long files for multi-part
mailings, providing checksums), which make it very flexible at
creating shar files. After the files have been sent, the unshar tool
scans mail messages looking for shar files. Unshar automatically
strips off mail headers and introductory text and then unpacks the
shar files.

Install sharutils if you send binary files through e-mail.


* Thu Mar 31 2005 Than Ngo
4.2.1-18.1.FC2

- apply patch to fix multiple buffer overflows #152571
- apply patch to fix buffer overflow in handling of -o option,
CVE-2004-1772, #123230



Solution : http://www.fedoranews.org/blog/index.php?p=560
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the sharutils package";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "Fedora Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/RedHat/rpm-list");
 exit(0);
}

include("rpm.inc");
if ( rpm_check( reference:"sharutils-4.2.1-18.1.FC2", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"sharutils-debuginfo-4.2.1-18.1.FC2", release:"FC2") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"sharutils-", release:"FC2") )
{
 set_kb_item(name:"CVE-2004-1772", value:TRUE);
}
