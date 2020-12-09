URL = https://strace.io/files/5.8/strace-5.8.tar.xz

ifeq ($(ARCH),arm64)
MAKEVARS += ARCH=aarch64
endif

CONFOPTS += --enable-mpers=no

CFLAGS += -std=c99

include $(ROOTDIR)/tools/automake.inc
