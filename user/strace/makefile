URL = https://strace.io/files/5.5/strace-5.5.tar.xz

ifeq ($(ARCH),arm64)
MAKEVARS += ARCH=aarch64
endif

CONFOPTS += --enable-mpers=check
CFLAGS += -std=c99

include $(ROOTDIR)/tools/automake.inc
