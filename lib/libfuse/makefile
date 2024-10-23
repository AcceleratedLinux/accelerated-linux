VERSION = 2.9.7
URL = https://github.com/libfuse/libfuse/releases/download/fuse-$(VERSION)/fuse-$(VERSION).tar.gz

CONFOPTS += --disable-example
CONFOPTS += --enable-lib
CONFOPTS += --enable-util
CONFOPTS += --program-prefix=

MAKEVARS  = LIBS=-lpthread

include $(ROOTDIR)/tools/automake.inc
