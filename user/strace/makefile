VERSION=6.8
URL = https://strace.io/files/$(VERSION)/strace-$(VERSION).tar.xz

ifeq ($(ARCH),arm64)
MAKEVARS += ARCH=aarch64
endif

CONFOPTS += --enable-mpers=no

CFLAGS += -std=c99

ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -R /bin/strace-graph
