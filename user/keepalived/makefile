
VERSION = 2.2.2
URL = https://www.keepalived.org/software/keepalived-$(VERSION).tar.gz

# Investigate netfilter ipvs module before enabling keepalived's lvs
CONFOPTS=--disable-lvs --with-kernel-dir=$(STAGEDIR) --enable-reproducible-build
AUTORECONF = ./build_setup

ifdef CONFIG_USER_KEEPALIVED_MINIMAL
AUTOMAKE_ROMFS = my_romfs
endif

include $(ROOTDIR)/tools/automake.inc

ifdef CONFIG_USER_KEEPALIVED_MINIMAL
my_romfs:
	$(ROMFSINST) build/keepalived-*-install/sbin/keepalived /sbin/keepalived
endif
