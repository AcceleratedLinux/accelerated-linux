URL = https://strace.io/files/4.25/strace-4.25.tar.xz

ifeq ($(ARCH),arm64)
MAKEVARS += ARCH=aarch64
endif

CONFOPTS += --enable-mpers=check
CFLAGS += -std=c99

include $(ROOTDIR)/tools/automake.inc
