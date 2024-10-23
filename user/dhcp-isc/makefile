
VERSION  = 4.4.3
URL      = https://downloads.isc.org/isc/dhcp/$(VERSION)/dhcp-$(VERSION).tar.gz
MAKEVARS = 

CFLAGS += -D_GNU_SOURCE
CFLAGS += -DBACKTRACE_DISABLED=1
LDFLAGS += -latomic

CONVARS   = enable_backtrace=no
BUILDVARS = enable_backtrace=no

CONFOPTS += --with-randomdev=/dev/random
CONFOPTS += --disable-backtrace
ifndef CONFIG_IPV6
CONFOPTS += --disable-dhcpv6
endif
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
endif

AUTOMAKE_ROMFS = dhcp-isc_romfs

include $(ROOTDIR)/tools/automake.inc

dhcp-isc_romfs:
	$(ROMFSINST) -e CONFIG_USER_DHCP_ISC_CLIENT_DHCLIENT \
		build/dhcp-$(VERSION)/client/dhclient  /bin/dhclient
	$(ROMFSINST) -e CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY \
		build/dhcp-$(VERSION)/relay/dhcrelay   /bin/dhcrelay
	$(ROMFSINST) -e CONFIG_USER_DHCP_ISC_SERVER_DHCPD \
		build/dhcp-$(VERSION)/server/dhcpd     /bin/dhcpd
	$(ROMFSINST) -e CONFIG_USER_DHCP_ISC_DHCPCTL_OMSHELL \
		build/dhcp-$(VERSION)/dhcpctl/omshell  /bin/omshell
