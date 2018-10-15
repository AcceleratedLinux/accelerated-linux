#
# (C) Tenable Network Security
#
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(11650);
 script_bugtraq_id(10937, 7562);
 script_version ("$Revision: 1.7 $");
 

 name["english"] = "MAILsweeper PowerPoint DoS";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is running MAILsweeper - a content security solution 
for SMTP.

There is a flaw in the remote version of MAILsweeper which may allow
an attacker to disable this service by sending a malformed PowerPoint
file to any recipient through this SMTP server.

While processing the file, the server will enter into an infinite
loop which will consume all available CPU resources on this host.
	  
*** Nessus did not actually test for this flaw - it just relied
*** on the banner to identify them. Therefore, this warning may be
*** a false positive

Solution : Upgrade to MAILsweeper 4.3.15 or newer
Risk factor : Medium";




 script_description(english:desc["english"]);
		    
 
 summary["english"] = "Checks the remote banner";
 summary["francais"] = "V�rfie la banni�re distante";
 script_summary(english:summary["english"],
 		 francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2003 Tenable Network Security",
 		  francais:"Ce script est Copyright (C) 2003 Tenable Network Security");
 
 family["english"] = "Denial of Service";
 family["francais"] = "D�ni de service";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("smtpserver_detect.nasl", "sendmail_expn.nasl");
 script_exclude_keys("SMTP/wrapped");
 script_require_ports("Services/smtp", 25);
 exit(0);
}


include("smtp_func.inc");

port = get_kb_item("Services/smtp");
if(!port)port = 25;
if (get_kb_item('SMTP/'+port+'/broken')) exit(0);

banner = get_smtp_banner(port:port);
if ( ! banner ) exit(0);
if(egrep(string:banner, pattern:"^220 .* MAILsweeper ESMTP Receiver Version ([0-3]\.|4\.([0-2]\.|3\.([0-9]|1[0-4])[^0-9])).*$")) security_warning(port);
