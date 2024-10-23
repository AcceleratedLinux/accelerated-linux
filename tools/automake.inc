# Makefile to auto build tarball's with patches etc
# see Documentation/automake.txt for more info on using this file
# Copyright (C) 2009 David McCullough <davidm@uclinux.org>

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= configured
include $(ROOTDIR)/tools/base.inc
include $(ROOTDIR)/tools/download.inc

#
# these config options apply to all things we build
#
CONFOPTS += --prefix=
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
ifeq ($(findstring --sysconfdir,$(CONFOPTS)),)
  CONFOPTS += --sysconfdir=/etc/config
endif
endif

CONFOPTS += --with-vendor-name=$(CONFIG_VENDOR)
CONFOPTS += --with-vendor-name-short=$(CONFIG_VENDOR)
CONFOPTS += --with-os-name=uClinux-dist
CONFOPTS += --with-os-vendor=uClinux
CONFOPTS += --disable-static

#
# for uClibc without glibc compat
#
CONFOPTS += --enable-malloc0returnsnull

#
# make sure LD does the right thing during configure if invoked directly
#
LDFLAGS  += -Wl,-L$(STAGEDIR)/lib -Wl,-rpath-link=$(STAGEDIR)/lib
LDFLAGS  += -Wl,-L$(STAGEDIR)/lib64 -Wl,-rpath-link=$(STAGEDIR)/lib64

#
# force using our special libtool
#
MAKEVARS     ?= LIBTOOL="$(LIBTOOL)"

define automake_pkg_def
  $(1)_NODEFCONF ?= $(NODEFCONF)
  $(1)_AUTOCONF ?= $(AUTOCONF)
  $(1)_AUTORECONF ?= $(if $(AUTORECONF),$(AUTORECONF),$$($(1)_AUTOCONF))
  $(1)_CONFIGURE ?= $(if $(CONFIGURE),$(CONFIGURE),configure)
endef
$(foreach pkg,$(PKG_y),$(eval $(call automake_pkg_def,$(pkg))))

.PRECIOUS: $(addprefix build/,$(addsuffix -autoconf,$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -configured,$(PKG_y)))

build/%-autoconf: build/%-patched
	@echo "Autoconf $* ..."
	$(if $($*_AUTORECONF),cd build/$($*_SRCDIR); $($*_AUTORECONF))
	$(AT)touch $@

build/%-configured: build/%-autoconf
	@echo "Configure $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)
	$(AT)[ ! -f config.cache ] || cp config.cache build/$($*_BUILDDIR)
	$(AT)cd build/$($*_BUILDDIR); \
		if [ -f '$($*_CONFIGURE)' ]; then \
			$(CONFVARS) \
			$($*_CONFVARS) \
			CFLAGS="$(CFLAGS) $($*_CFLAGS)" \
			bash $($*_CONFIGURE) $(if $($*_NODEFCONF),,$(CONFIGURE_OPTS) $(CONFOPTS)) $($*_CONFOPTS); \
		fi
	$(AT)touch $@
