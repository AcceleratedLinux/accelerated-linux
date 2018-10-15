#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#
# Ref:
# From: "euronymous" <just-a-user@yandex.ru>
# To: vuln@security.nnov.ru, bugtraq@securityfocus.com
# Subject: Justice Guestbook 1.3 vulnerabilities



if(description)
{
 script_id(11501);
 script_bugtraq_id(7233, 7234);
 script_version ("$Revision: 1.7 $");


 name["english"] = "Justice guestbook";

 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is running Justice Guestbook.

This set of CGI has two vulnerabilities :

	- It is vulnerable to cross site scripting attacks (in jgb.php3)
	- If the user requests the file cfooter.php3, he will obtain
	  the physical path of the remote CGI
	
	
An attacker may use these flaws to steal the cookies of your users
or to gain better knowledge about this host.


Solution : Delete this package
Risk factor : Low";




 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the presence of cfooter.php3";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2003 Renaud Deraison");
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
if(!can_host_php(port:port)) exit(0);




gdir = make_list(cgi_dirs());

dirs = make_list("", "/guestbook");
foreach d (gdir)
{
  dirs = make_list(dirs, string(d, "/guestbook"), d);
}


foreach dir (dirs)
{
 req = http_get(item:string(dir, "/cfooter.php3"), port:port);
 res = http_keepalive_send_recv(port:port, data:req);

 if( res == NULL ) exit(0);

 if(egrep(pattern:"getsets().*cfooter\.php3", string:res))
 	{
	security_warning(port);
	exit(0);
	}
}
