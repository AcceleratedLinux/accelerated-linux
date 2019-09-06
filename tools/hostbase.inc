# Host makefile targets
#
# Defines targets for building and installing host build tools

.EXPORT_ALL_VARIABLES:

AT := $(if $(V:0=),,@)

BUILD_CC = $(HOSTCC)
HOST_CC      = gcc
HOST_CXX     = g++
HOST_RANLIB  = $(shell which ranlib || echo :)

# Backwards compat
PKG_y += $(AUTOMAKE_y)
PKG_n += $(AUTOMAKE_n)
PKG_  += $(AUTOMAKE_)

#
# some short cut config,  just set URL = blah
# and we work the rest out
#
ifeq ($(AUTOMAKE_y)$(AUTOMAKE_n)$(AUTOMAKE_)$(PKG_y)$(PKG_n)$(PKG_),)
  ifdef URL
    ifndef PKG
      PKG := $(shell basename $(URL))
      PKG := $(PKG:.tar.gz=)
      PKG := $(PKG:.tar.bz2=)
      PKG := $(PKG:.tar.xz=)
      PKG := $(PKG:.tar.lz=)
      PKG := $(PKG:.tgz=)
      PKG := $(PKG:.tbz2=)
      PKG := $(PKG:.txz=)
      PKG := $(PKG:.tlz=)
      PKG := $(PKG:.zip=)
      PKG := $(PKG:.git=)
    endif
    # This adds to PKG_y too
    AUTOMAKE_y += $(PKG)
  endif
endif

#
# allow some targets to be overridden
#
CONFIGTARGET ?= noconfig
BUILDTARGET ?= built
INSTALLTARGET ?= installed
FINALTARGET ?= lndir
COMPLETETARGET ?= complete_target

#
# per package setting
#
define base_pkg_def
  $(1)_BUILD ?= $(if $(BUILD),$(BUILD),)
  $(1)_INSTALL ?= $(if $(INSTALL),$(INSTALL),install)
  $(1)_BUILDDIR ?= $(if $(BUILDDIR),$(BUILDDIR),$(1))
  $(1)_SRCDIR ?= $(if $(SRCDIR),$(SRCDIR),$$($(1)_BUILDDIR))
  $(1)_DSTDIR ?= $(if $(DSTDIR),$(DSTDIR),$$($(1)_BUILDDIR))
  $(1)_FINALTARGET ?= $(if $(FINALTARGET),$(FINALTARGET),)
endef
$(foreach pkg,$(PKG_y),$(eval $(call base_pkg_def,$(pkg))))

#
# the main target
#
all: $(foreach pkg,$(PKG_y),build/$(pkg)-$($(pkg)_FINALTARGET)) $(COMPLETETARGET)

define base_pkg_dep
build/%-$(CONFIGTARGET): $(foreach dep,$(1)_DEP,build/$(dep)-$($(dep)_FINALTARGET))
endef
$(foreach pkg,$(PKG_y),$(eval $(call base_pkg_dep,$(pkg))))

.PHONY: $(COMPLETETARGET)
$(COMPLETETARGET): $(foreach pkg,$(PKG_y),build/$(pkg)-$($(pkg)_FINALTARGET))

.PHONY: force_target
force_target:

.PRECIOUS: $(addprefix build/,$(addsuffix -noconfig,$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -built,$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -installed,$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -$(BUILDTARGET),$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -$(INSTALLTARGET),$(PKG_y)))

build/%-noconfig:
	$(AT)touch $@

build/%-built: build/%-$(CONFIGTARGET)
	@echo "Building $* ..."
	$(AT)$(BUILDVARS) \
	$($*_BUILDVARS) \
		$(MAKE) -C build/$($*_BUILDDIR) $($*_BUILD) $(MAKEVARS) $($*_MAKEVARS)
	$(AT)touch $@

build/%-installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)$(BUILDVARS) \
	$($*_BUILDVARS) \
	$($*_INSTALLVARS) \
		$(MAKE) -C build/$($*_BUILDDIR) $($*_INSTALL) DESTDIR=`pwd`/build/$*-install $(MAKEVARS) $($*_MAKEVARS)
	$(AT)touch $@

#
# The link tree construction for tools uses hard links. Some packages like
# gcc play funky tricks to find themselves and their support files. They
# follow soft links and get messed up trying to find them.
#
build/%-lndir: build/%-$(INSTALLTARGET) force_target $(if $(AUTOMAKE_PRESTAGE),$(AUTOMAKE_PRESTAGE),)
	@echo "Staging $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)cd build/$*-install/$(TOOLSDIR); find . \( -type f -o -type l \) | egrep -v '.la$$' | \
	while read t; do \
		rm -f $(TOOLSDIR)/$$t; \
		mkdir -p "`dirname \"$(TOOLSDIR)/$$t\"`"; \
		ln "`pwd`/$$t" "$(TOOLSDIR)/$$t"; \
		echo "ln \"`pwd`/$$t\" \"$(TOOLSDIR)/$$t\""; \
	done
	$(AT)touch $@

clean: $(foreach pkg,$(PKG_y),$($(pkg)_CLEAN))
	rm -rf build downloads

