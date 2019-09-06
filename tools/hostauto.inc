# Makefile to auto build tarball's with patches etc
# see Documentation/automake.txt for more info on using this file
# Copyright (C) 2009 David McCullough <davidm@uclinux.org>

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= configured
include $(ROOTDIR)/tools/hostbase.inc
include $(ROOTDIR)/tools/download.inc

CONFOPTS += --prefix=$(TOOLSPREFIX)

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
