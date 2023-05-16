FLASHROM_VERSION = 0.9.8
URL = https://download.flashrom.org/releases/flashrom-$(FLASHROM_VERSION).tar.bz2

MAKEVARS = PREFIX="/"
CFLAGS += -D__GLIBC__=2
CFLAGS += -D__GLIBC_MINOR__=10
CFLAGS += -D_GNU_SOURCE

LIBS += -ludev

include $(ROOTDIR)/tools/automake.inc

