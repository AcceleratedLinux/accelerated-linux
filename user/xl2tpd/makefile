PKG = xl2tpd
VERSION = 1.3.16
URL = https://github.com/xelerance/$(PKG)/archive/refs/tags/v$(VERSION).tar.gz
DOWNLOADNAME = $(PKG)-$(VERSION).tar.gz
BUILDDIR = $(PKG)-$(VERSION)

PREFIX = /usr
CFLAGS += -DPPPD='"/bin/pppd"'

include $(ROOTDIR)/tools/automake.inc
