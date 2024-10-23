VERSION=1.28.1
URL = https://github.com/c-ares/c-ares/releases/download/cares-$(subst .,_,$(VERSION))/c-ares-$(VERSION).tar.gz
include $(ROOTDIR)/tools/automake.inc

CFLAGS:=$(filter-out -D%,$(CFLAGS))
