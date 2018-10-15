#
#  (C) Tenable Network Security
#


 desc = "
Synopsis :

There is a peer-to-peer file sharing application installed on the
remote Windows host. 

Description :

BitComet is installed on the remote host.  BitComet is a freeware
peer-to-peer file sharing application for Windows.

Make sure the use of this program fits with your corporate security
policy.

See also :

http://www.bitcomet.com/

Solution :

Deinstall this software if its use does not match your corporate security
policy.

Risk factor : 

None";


if (description) {
  script_id(20748);
  script_version("$Revision: 1.3 $");

  script_name(english:"BitComet Detection");
  script_summary(english:"Checks for BitComet"); 
 
  script_description(english:desc);
 
  script_category(ACT_GATHER_INFO);
  script_family(english:"Peer-To-Peer File Sharing");

  script_copyright(english:"This script is Copyright (C) 2006 Tenable Network Security");

  script_dependencies("smb_hotfixes.nasl");
  script_require_keys("SMB/Registry/Enumerated");
  script_require_ports(139, 445);

  exit(0);
}


include("global_settings.inc");
include("smb_func.inc");


# Connect to the appropriate share.
if (!get_kb_item("SMB/Registry/Enumerated")) exit(0);
name    =  kb_smb_name();
port    =  kb_smb_transport();
if (!get_port_state(port)) exit(0);
login   =  kb_smb_login();
pass    =  kb_smb_password();
domain  =  kb_smb_domain();

soc = open_sock_tcp(port);
if (!soc) exit(0);

session_init(socket:soc, hostname:name);
rc = NetUseAdd(login:login, password:pass, domain:domain, share:"IPC$");
if (rc != 1) {
  if (log_verbosity > 1) debug_print("can't connect to the remote share (rc)!", level:0);
  NetUseDel();
  exit(0);
}


# Connect to remote registry.
hklm = RegConnectRegistry(hkey:HKEY_LOCAL_MACHINE);
if (isnull(hklm)) {
  if (log_verbosity > 1) debug_print("can't connect to the remote registry!", level:0);
  NetUseDel();
  exit(0);
}


# Determine if it's installed.
key = "SOFTWARE\Classes\bctp\shell\open\command";
key_h = RegOpenKey(handle:hklm, key:key, mode:MAXIMUM_ALLOWED);
if (!isnull(key_h)) {
  value = RegQueryValue(handle:key_h, item:NULL);
  if (!isnull(value)) {
    # nb: the value may appear in quotes.
    exe = ereg_replace(pattern:'"(.+)"', replace:"\1", string:value[1]);
  }
  RegCloseKey(handle:key_h);
}


# If it is...
if (exe) {
  # Determine its version from the executable itself.
  share = ereg_replace(pattern:"([A-Z]):.*", replace:"\1$", string:exe);
  path =  ereg_replace(pattern:"[A-Z]:(.*)", replace:"\1", string:exe);
  NetUseDel(close:FALSE);

  rc = NetUseAdd(login:login, password:pass, domain:domain, share:share);
  if (rc != 1) {
    if (log_verbosity > 1) debug_print("can't connect to the remote share (", rc, ")!", level:0);
    NetUseDel();
    exit(0);
  }

  fh = CreateFile(
    file:path,
    desired_access:GENERIC_READ,
    file_attributes:FILE_ATTRIBUTE_NORMAL,
    share_mode:FILE_SHARE_READ,
    create_disposition:OPEN_EXISTING
  );
  if (!isnull(fh)) {
    ver = GetFileVersion(handle:fh);
    CloseFile(handle:fh);
  }

  # If the version number's available, save and report it.
  if (!isnull(ver)) {
    version = string(ver[0], ".", ver[1], ".", ver[2], ".", ver[3]);

    set_kb_item(name:"SMB/BitComet/Version", value: version);

      report = string(
        desc,
        "\n\n",
        "Plugin output :\n",
        "\n",
        "Version ", version, " of BitComet is installed as :\n",
        "  ", exe, "\n"
      );

    security_note(port:0, data:report);
  }
}


# Clean up.
RegCloseKey(handle:hklm);
NetUseDel();
