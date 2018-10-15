#
# (C) Tenable Network Security
#


if(description)
{
 script_id(16154);
 script_cve_id("CVE-2005-0217");
 script_bugtraq_id(12205);
 script_version("$Revision: 1.4 $");
 name["english"] = "Invision Community Blog SQL Injection";
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host appears to be running Invision Community Blog, a
weblog utility.

There is a flaw in the remote software which may allow anyone
to inject arbitrary SQL commands, which may in turn be used to
gain administrative access on the remote host.

Solution : Upgrade to the latest version of this software
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "SQL Injection";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "CGI abuses";
 script_family(english:family["english"]);
 script_dependencie( "http_version.nasl");
 script_require_ports("Services/www", 80);
 script_exclude_keys("Settings/disable_cgi_scanning");
 exit(0);
}

# Check starts here

include("http_func.inc");
include("http_keepalive.inc");


function check(dir)
{
  host = get_host_name();
  req = http_get(item : dir + "/index.php?automodule=blog&blogid=1&cmd=showentry&eid=1'", port:port);

  buf = http_keepalive_send_recv(port:port, data:req);
  if(buf == NULL)exit(0);

  if("SELECT * FROM ibf_blog_entries WHERE blog_id=1 and entry_id" >< buf )
  	{
	security_hole(port);
	exit(0);
	}
 
 
 return(0);
}

port = get_http_port(default:80);

if(!get_port_state(port))exit(0);
if ( ! can_host_php(port:port) ) exit(0);


foreach dir (cgi_dirs()) 
 {
  check(dir:dir);
 }
