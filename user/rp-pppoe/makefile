VERSION = 3.15
URL = https://dianne.skoll.ca/projects/rp-pppoe/download/rp-pppoe-$(VERSION).tar.gz

BUILDDIR = ./rp-pppoe-$(VERSION)/src
CONFVARS += PPPD=/bin/pppd
AUTOMAKE_ROMFS = rp-pppoe_romfs

include $(ROOTDIR)/tools/automake.inc

rp-pppoe_romfs:
	$(ROMFSINST) build/rp-pppoe-$(VERSION)-install/sbin/pppoe-server /sbin/pppoe-server
