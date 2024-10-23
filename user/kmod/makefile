
VERSION = 32
URL = https://www.kernel.org/pub/linux/utils/kernel/kmod/kmod-$(VERSION).tar.gz

CONFOPTS += --disable-dependency-tracking
CONFOPTS += --enable-fast-install
CONFOPTS += --disable-manpages
CONFOPTS += --disable-gtk-doc
CONFOPTS += --disable-gtk-doc-html
CONFOPTS += --disable-gtk-doc-pdf

ifdef CONFIG_USER_DEBUG
  CONFOPTS += --enable-debug
endif
ifndef CONFIG_USER_KMOD_TOOLS
  CONFOPTS += --disable-tools
endif
ifndef CONFIG_USER_KMOD_LIBKMOD
  CONFOPTS += --disable-shared
endif

# kmod now installs tools under /bin, but much of uClinux expects /sbin/modprobe
CONFOPTS += --bindir=/sbin

# avoid jlibtool
MAKEVARS=

ifeq ($(CONFIG_DEFAULTS_LIBC_UCLIBC),y)
LDFLAGS += -lrt
endif

include $(ROOTDIR)/tools/automake.inc
