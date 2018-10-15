#
# (C) Tenable Network Security
# based on work from David Maciejak
#
# This script is released under the GNU GPLv2


if(description)
{
 script_id(14323);
 script_cve_id("CVE-2004-1735");
 script_bugtraq_id(10992);
 name["english"] = "Sympa New List Cross Site Scripting";

 script_name(english:name["english"]);
 script_version ("$Revision: 1.7 $"); 
 desc["english"] = "
The remote host seems to be running sympa, an open source mailing list software.

This version of Sympa contains an HTML injection vulnerability which may
allow a user who has the privileges to create a new list to inject HTML 
tags in the list description field.

See also : http://www.sympa.org/
Solution : Update to version 4.1.3 or newer.
Risk factor : Low";

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for sympa version";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "CGI abuses : XSS";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "http_version.nasl");
 script_require_ports("Services/www", 80);
 script_exclude_keys("Settings/disable_cgi_scanning");
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");
include("http_keepalive.inc");

port = get_http_port(default:80);

if(!get_port_state(port))exit(0);


function check(url)
{
req = http_get(item:string(url, "home"), port:port);
r = http_keepalive_send_recv(port:port, data:req);
if ( r == NULL ) exit(0);

    #We need to match this
    
    #TD NOWRAP><I>Powered by</I></TD>
    #<TD><A HREF="http://www.sympa.org/">
    #       <IMG SRC="/icons/sympa/logo-s.png" ALT="Sympa 4.1.2" BORDER="0" >
	if ("http://www.sympa.org/" >< r)
	{
        	if(egrep(pattern:".*ALT=.Sympa (2\.|3\.|4\.0\.|4\.1\.[012][^0-9])", string:r))
 		{
 			security_note(port);
			exit(0);
		}
	}
 
}

check(url:"");
check(url:"/wws/");
check(url:"/wwsympa/");

foreach dir (cgi_dirs())
{
 check(url:dir);
}
