BUSYBOX = busybox-1.36.1
URL = https://busybox.net/downloads/$(BUSYBOX).tar.bz2

BUILDVARS = ./mkconfig > build/$($*_BUILDDIR)/.config ;
BUILDVARS += make -C build/$($*_BUILDDIR) oldconfig ;

$(BUSYBOX)_BUILD = SKIP_STRIP=y
$(BUSYBOX)_INSTALL = install SKIP_STRIP=y
$(BUSYBOX)_ROMFS = secure_symlinks

ifneq ($(CONFIG_DEFAULTS_LIBC_UCLIBC),y)
LDLIBS += tirpc
endif

MAKEVARS = CONFIG_PREFIX=`pwd`/build/$($*_BUILDDIR)-install

# Building busybox LDFLAGS must be real linker flags, not CFLAGS
#LDFLAGS =

include $(ROOTDIR)/tools/automake.inc

# a list of binaries that we want to handle specifically,  so symlinking
# to busybox is not appropriate (ie., apparmor profile use)

SPECIAL_BINARIES =
ifeq ($(CONFIG_USER_BUSYBOX_SH_IS_NONE),)
SPECIAL_BINARIES += /bin/sh
endif
ifdef CONFIG_USER_BUSYBOX_HTTPD
SPECIAL_BINARIES += /usr/sbin/httpd
endif
ifdef CONFIG_USER_BUSYBOX_UDHCPC
SPECIAL_BINARIES += /sbin/udhcpc
endif
ifdef CONFIG_USER_BUSYBOX_STTY
SPECIAL_BINARIES += /bin/stty
endif

secure_symlinks:
	@for i in $(SPECIAL_BINARIES); do \
		echo "Hardlinking $$i ..."; \
		$(ROMFSINST) -d -l /bin/busybox $$i; \
	done

