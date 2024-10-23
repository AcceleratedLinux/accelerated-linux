URL = https://git.code.sf.net/p/net-tools/code
VERSION = v2.10
METHOD = git
AUTORECONF = touch config.h
include $(ROOTDIR)/tools/automake.inc

SBINDIR = sbin
BINDIR  = bin

romfs:
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_ARP		build/code/arp 		/$(SBINDIR)/arp
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_HOSTNAME		build/code/hostname 	/$(BINDIR)/hostname
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_IFCONFIG		build/code/ifconfig	/$(SBINDIR)/ifconfig
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_NAMEIF		build/code/nameif 	/$(SBINDIR)/nameif
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_NETSTAT		build/code/netstat 	/$(BINDIR)/netstat
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_PLIPCONFIG	build/code/plipconfig	/$(SBINDIR)/plipconfig
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_RARP		build/code/rarp 	/$(SBINDIR)/rarp
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_ROUTE		build/code/route 	/$(SBINDIR)/route
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_SLATTACH		build/code/slattach	/$(SBINDIR)/slattach
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_IPMADDR		build/code/ipmaddr	/$(SBINDIR)/ipmaddr
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_IPTUNNEL		build/code/iptunnel	/$(SBINDIR)/iptunnel
	$(ROMFSINST) -e CONFIG_USER_NET_TOOLS_MII_TOOL		build/code/mii-tool	/$(SBINDIR)/mii-tool

