BUSYBOX = busybox-1.31.0
URL = https://busybox.net/downloads/$(BUSYBOX).tar.bz2

BUILDVARS = ./mkconfig > build/$($*_BUILDDIR)/.config ;
BUILDVARS += make -C build/$($*_BUILDDIR) oldconfig ;

$(BUSYBOX)_BUILD = SKIP_STRIP=y
$(BUSYBOX)_INSTALL = install SKIP_STRIP=y
$(BUSYBOX)_ROMFS = secure_symlinks
ifeq ($(CONFIG_DEFAULTS_LIBC_MUSL)$(CONFIG_DEFAULTS_LIBC_UCLIBC_NG),y)
LDLIBS += tirpc
endif

MAKEVARS = CONFIG_PREFIX=`pwd`/build/$($*_BUILDDIR)-install

# Building busybox LDFLAGS must be real linker flags, not CFLAGS
LDFLAGS =

include $(ROOTDIR)/tools/automake.inc

# a list of binaries that we want to handle specifically,  so symlinking
# to busybox is not appropriate (ie., apparmor profile use)

SPECIAL_BINARIES = /bin/sh

secure_symlinks:
	@for i in $(SPECIAL_BINARIES); do \
		echo "Hardlinking $$i ..."; \
		$(ROMFSINST) -l /bin/busybox $$i; \
	done

