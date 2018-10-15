VERSION = 1.2-rc4
URL = http://download.aircrack-ng.org/aircrack-ng-$(VERSION).tar.gz

AUTORECONF = sh ./autocfg "$(CC)"

CFLAGS += -D_GLIBCXX_HAVE_MBSTATE_T
LDFLAGS += -Wl,-rpath-link -Wl,$(ROOTDIR)/romfs/lib

include $(ROOTDIR)/tools/automake.inc
