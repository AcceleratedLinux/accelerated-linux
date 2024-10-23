VERSION = 4.0
URL = https://salsa.debian.org/dskoll/rp-pppoe.git
METHOD = git

BUILDDIR = ./rp-pppoe/src
CONFVARS += PPPD=/bin/pppd
AUTOMAKE_ROMFS = rp-pppoe_romfs

include $(ROOTDIR)/tools/automake.inc

rp-pppoe_romfs:
	$(ROMFSINST) build/rp-pppoe-install/sbin/pppoe-server /sbin/pppoe-server
