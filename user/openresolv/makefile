URL = https://roy.marples.name/downloads/openresolv/openresolv-3.11.0.tar.xz

DOWNLOADHASH=eaea9d1d1e07b3fe81305178d7ef6a8c9e2a9f3b751c9b52077941809058fc3c

AUTOMAKE_ROMFS = openresolv_romfs

include $(ROOTDIR)/tools/automake.inc

CONFOPTS += --os=linux
CONFOPTS += --libexecdir=/etc/resolvconf
CONFOPTS += --rundir=/var/run
CONFOPTS += --sbindir=/bin
CONFOPTS += --restartcmd=true

I=build/$(PKG)-install
openresolv_romfs:
	$(ROMFSINST) -d $(I)/bin/resolvconf /bin/resolvconf
	$(ROMFSINST) -d $(I)/etc/resolvconf/libc /etc/resolvconf/libc
	$(ROMFSINST) -d $(I)/etc/resolvconf/dnsmasq /etc/resolvconf/dnsmasq
#	$(ROMFSINST) -d $(I)/etc/resolvconf/pdnsd /etc/resolvconf/pdnsd
#	$(ROMFSINST) -d $(I)/etc/resolvconf/named /etc/resolvconf/named
#	$(ROMFSINST) -d $(I)/etc/resolvconf/unbound /etc/resolvconf/unbound

