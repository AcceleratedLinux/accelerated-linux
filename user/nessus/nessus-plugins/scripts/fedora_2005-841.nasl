#
# (C) Tenable Network Security
#
# This plugin text is was extracted from the Fedora Security Advisory
#


if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(19725);
 script_version ("$Revision: 1.1 $");
 
 name["english"] = "Fedora Core 3 2005-841: perl-DBI";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing the patch for the advisory FEDORA-2005-841 (perl-DBI).

DBI is a database access Application Programming Interface (API) for
the Perl programming language. The DBI API specification defines a set
of functions, variables and conventions that provide a consistent
database interface independent of the actual database being used.

Update Information:

Old and low priority security update that we forgot to push
a while ago.


Solution : Get the newest Fedora Updates
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Check for the version of the perl-DBI package";
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
if ( rpm_check( reference:"perl-DBI-1.40-6.fc3", release:"FC3") )
{
 security_hole(0);
 exit(0);
}
