
URL = http://www.keepalived.org/software/keepalived-1.2.23.tar.gz
LIBS=-lnl-3 -lnl-genl-3
# Investigate netfilter ipvs module before enabling keepalived's lvs
CONFOPTS=--disable-lvs --with-kernel-dir=$(STAGEDIR)/include
LDFLAGS=-z muldefs

ifdef CONFIG_USER_KEEPALIVED_MINIMAL
AUTOMAKE_ROMFS = my_romfs
endif

include $(ROOTDIR)/tools/automake.inc

ifdef CONFIG_USER_KEEPALIVED_MINIMAL
my_romfs:
	$(ROMFSINST) build/keepalived-*-install/sbin/keepalived /sbin/keepalived
endif
