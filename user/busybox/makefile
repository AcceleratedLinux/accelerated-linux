BUSYBOX = busybox-1.31.0
URL = https://busybox.net/downloads/$(BUSYBOX).tar.bz2

BUILDVARS = ./mkconfig > build/$($*_BUILDDIR)/.config ;
BUILDVARS += make -C build/$($*_BUILDDIR) oldconfig ;

$(BUSYBOX)_BUILD = SKIP_STRIP=y
$(BUSYBOX)_INSTALL = install SKIP_STRIP=y
ifeq ($(CONFIG_DEFAULTS_LIBC_MUSL),y)
LDLIBS += tirpc
endif

MAKEVARS = CONFIG_PREFIX=`pwd`/build/$($*_BUILDDIR)-install

# Building busybox LDFLAGS must be real linker flags, not CFLAGS
LDFLAGS =

include $(ROOTDIR)/tools/automake.inc

