#
# (C) Tenable Network Security
#

if(description)
{
 script_id(11404);
 script_bugtraq_id(6218, 6754, 6755, 6756, 6757, 6759, 6811, 6814, 6962, 7056);
 
 script_version("$Revision: 1.8 $");

 name["english"] = "Multiple flaws in the Opera web browser";

 script_name(english:name["english"]);
 
 desc["english"] = "
The version of Opera installed on the remote host is vulnerable to
various security flaws, ranging from cross site scripting to buffer
overflows. 

To exploit them, an attacker would need to set up a rogue web site,
then lure a user of this host visit it using Opera.  He would then be
able to execute arbitrary code on this host. 

Solution : Install Opera 7.0.3 or newer
Risk factor : High";



 script_description(english:desc["english"]);
 
 summary["english"] = "Determines the version of Opera.exe";

 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005-2006 Tenable Network Security");
 family["english"] = "Windows";
 script_family(english:family["english"]);
 
 script_dependencies("opera_installed.nasl");
 script_require_keys("SMB/Opera/Version");

 exit(0);
}


v = get_kb_item("SMB/Opera/Version");
if(strlen(v))
{
  report = "
We have determined that you are running Opera v." + v + ". This version
is vulnerable to various security flaws which may allow an attacker to
execute arbitrary code on this host. 

To exploit these flaws, an attacker would need to set up a rogue website
and lure a user of this host visit it using Opera. He would then be able
to execute arbitrary code on this host.

Solution : Upgrade to version 7.03 or newer
Risk factor : High";

  v2 = split(v, sep:".", keep:FALSE);

  if(int(v2[0]) < 7 || (int(v2[0]) == 7 && int(v2[1]) < 3))
    security_hole(port:get_kb_item("SMB/transport"), data:report);
}
