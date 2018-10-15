#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(18577);
 script_version ("$Revision: 1.2 $");
 script_cve_id("CVE-2005-1934");
 
 name["english"] = "Fedora Core 4 2005-411: gaim";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2005-411 (gaim).

Gaim allows you to talk to anyone using a variety of messaging
protocols, including AIM (Oscar and TOC), ICQ, IRC, Yahoo!,
MSN Messenger, Jabber, Gadu-Gadu, Napster, and Zephyr.  These
protocols are implemented using a modular, easy to use design.
To use a protocol, just add an account using the account editor.

Gaim supports many common features of other clients, as well as many
unique features, such as perl scripting and C plugins.

Gaim is NOT affiliated with or endorsed by America Online, Inc.,
Microsoft Corporation, or Yahoo! Inc. or other messaging service
providers.

Update Information:

More bug and denial of service fixes.


Solution : http://www.redhat.com/archives/fedora-announce-list/2005-June/msg00023.html
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the gaim package";
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
if ( rpm_check( reference:"gaim-1.3.1-0.fc4", release:"FC4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_check( reference:"gaim-debuginfo-1.3.1-0.fc4", release:"FC4") )
{
 security_hole(0);
 exit(0);
}
if ( rpm_exists(rpm:"gaim-", release:"FC4") )
{
 set_kb_item(name:"CVE-2005-1934", value:TRUE);
}
