#
# (C) Tenable Network Security
#


if (description) {
  script_id(18568);
  script_version("$Revision: 1.4 $");

  script_cve_id("CVE-2005-2047");
  script_bugtraq_id(14034);
  if (defined_func("script_xref")) {
    script_xref(name:"OSVDB", value:"17596");
  }

  name["english"] = "DUpaypal Pro Multiple SQL Injection Vulnerabilities";
  script_name(english:name["english"]);
 
  desc["english"] = "
Synopsis :

The remote web server contains an ASP application that is vulnerable
to multiple SQL injection attacks. 

Description :

The remote host is running DUpaypal Pro, an ASP-based storefront from
DUware for Paypal. 

The installed version of DUpaypal Pro fails to properly sanitize user-
supplied input in several instances before using it in SQL queries. 
By exploiting these flaws, an attacker can affect database queries,
possibly disclosing sensitive data and launching attacks against the
underlying database. 

See also : 

http://echo.or.id/adv/adv19-theday-2005.txt
http://archives.neohapsis.com/archives/bugtraq/2005-06/0172.html

Solution : 

Unknown at this time.

Risk factor : 

Medium / CVSS Base Score : 5
(AV:R/AC:L/Au:NR/C:P/A:N/I:P/B:N)";
  script_description(english:desc["english"]);
 
  summary["english"] = "Checks for multiple SQL injection vulnerabilities in DUpaypal Pro";
  script_summary(english:summary["english"]);
 
  script_category(ACT_GATHER_INFO);
  script_family(english:"CGI abuses");

  script_copyright(english:"This script is Copyright (C) 2005-2006 Tenable Network Security");

  script_dependencies("http_version.nasl");
  script_exclude_keys("Settings/disable_cgi_scanning");
  script_require_ports("Services/www", 80);

  exit(0);
}


include("http_func.inc");
include("http_keepalive.inc");


port = get_http_port(default:80);
if (!get_port_state(port)) exit(0);
if (!can_host_asp(port:port)) exit(0);


# Loop through CGI directories.
foreach dir (cgi_dirs()) {
  # Try to exploit one of the flaws.
  req = http_get(
    item:string(
      dir, "/shops/sub.asp?",
      "iSub=", SCRIPT_NAME, "'"
    ), 
    port:port
  );
  res = http_keepalive_send_recv(port:port, data:req, bodyonly:TRUE);
  if (res == NULL) exit(0);

  # There's a problem if...
  if (
    # it looks like DUpaypal Pro and...
    'href="../css/DUpaypalPro.css" rel="stylesheet" ' >< res && 
    # there's a syntax error.
    egrep(string:res, pattern:string("Syntax error .+ AND PRO_SUBS.SUB_ID = ", SCRIPT_NAME, "'"))
  ) {
    security_warning(port);
    exit(0);
  }
}
