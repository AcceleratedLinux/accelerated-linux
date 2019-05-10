VERSION = 1.4
URL = https://github.com/coova/coova-chilli.git
METHOD = git

AUTORECONF = ./bootstrap
CONFOPTS += --$(if $(CONFIG_LIB_LIBSSL),with,without)-openssl

include $(ROOTDIR)/tools/automake.inc


# TODO: Be more selective about required features
ifdef NOT_DEFINED
  --disable-chilliquery   Disable chilli_query
  --disable-leakybucket   disable use of leaky bucket shaping
  --disable-uamanyip      disable use of uamanyip
  --disable-uamuiport     disable use of uamuiport
  --disable-accounting-onoff
                          disable use of Accounting-On and Accounting-Off
  --disable-tap           Disable support for tap interface (tun only)
  --disable-tcpreset      Disable support for TCP reset of filtered
                          connections
  --disable-radproxy      Disable support RADIUS (EAP) Proxy
  --enable-json           Enable support for JSON
  --disable-debug         Disable debugging messages
  --disable-dhcpradius    Disable support DHCP/RADIUS integration
  --enable-wpad           Enable support WPAD
  --enable-gardenaccounting
                          Enable walled garden accounting
  --enable-gardenext      Enable extended walled garden features
  --enable-inspect        Enable inspect feature in cmdsock
  --disable-coa           Disable CoA RADIUS support
  --enable-dhcpopt        Enable support for DHCP option setting
  --enable-debug2         Enable verbose debugging
  --enable-sessgarden     Enable support for session-based walled garden
  --enable-sessproxy      Enable support for per session postauth proxy
  --enable-sessdhcp       Enable support for per session DHCP relay
  --enable-sessdns        Enable support for per session DNS enforcement
  --enable-chillixml      Enable use of chillixml
  --enable-proxyvsa       Enable support for VSA attribute proxy
  --enable-ipwhitelist    Enable file based IP white list
  --enable-uamdomainfile  Enable loading of mass uamdomains from file
  --enable-redirdnsreq    Enable the sending of a DNS query on redirect
  --disable-ieee8021q     disable support for IEEE 802.1Q
  --enable-largelimits    Enable larger limits for use with non-embedded
                          systems
  --enable-authedallowed  enable Authurized Garden
  --enable-binstatusfile  Enable support for binary status file
  --enable-statusfile     Enable support for status file
  --enable-chilliproxy    Enable support for HTTP AAA Proxy
  --enable-multiroute     Enable support for multiple routes
  --enable-multilan       Enable support for multiple LANs
  --enable-chilliradsec   Enable support for RadSec AAA Proxy
  --enable-chilliredir    Enable support for Redir server
  --enable-chilliscript   Enable support for chilli_script helper
  --enable-cluster        Enable support for clustering
  --enable-sessionstate   Enable extended use of the CoovaChilli-Session-State
                          attribute
  --enable-sessionid      Enable the use of the CoovaChilli-Session-Id
                          attribute
  --enable-apsessionid    Enable the use of the CoovaChilli-AP-Session-Id
                          attribute
  --disable-coovachilliconfig
                          Disable the use of the CoovaChilli-Config attribute
  --enable-mdns           Enable support for MDNS
  --enable-netbios        Enable support for NetBIOS
  --enable-ieee8023       Enable support for IEEE 802.3 Ethernet
  --enable-pppoe          Enable support for PPPoE
  --enable-l2tpppp        Enable support for L2TP/PPP Tunneling
  --enable-eapol          Enable support for EAPOL
  --enable-miniportal     Enable support Coova miniportal
  --enable-miniconfig     Enable support minimal cmdline config
  --enable-ewtapi         Enable support for CoovaEWT API
  --enable-libjson        Enable support for libjson
  --enable-ssdp           Enable support for Simple Service Discovery Protocol
  --enable-layer3         Enable support for Layer3 only operation
  --enable-modules        Enable dynamically loadable modules (experimental)
  --enable-extadmvsa      Enable admin-user VSA attribute support
  --enable-redirinject    Enable Redir content-injection support
  --enable-netnat         Enable net interface nat (experimental)
  --enable-useragent      Enable recording user-agent
  --enable-acceptlanguage Enable recording Accept-Language
  --enable-location       Enable Location Awareness
  --enable-forcedns       Enable the forcing (NAT) of DNS
endif
