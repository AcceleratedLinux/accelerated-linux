# Base makefile targets
#
# Defines targets for building and installing to staging and romfs.

.EXPORT_ALL_VARIABLES:

AT := $(if $(V:0=),,@)
ifndef STAGEDIR
STAGEDIR = $(ROOTDIR)
endif

#
# its generally a bad idea to fail on linker warnings for packages we
# aren't fully controlling,  though we could put this back conditionally
#
LDFLAGS := $(LDFLAGS:-Wl,--fatal-warnings=)

BUILD_CC = $(HOSTCC)
HOST_CC      = gcc
HOST_CXX     = g++
HOST_RANLIB  = $(shell which ranlib || echo :)

UCFRONT_ENV ?= ucfront-env

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
  $(1)_FINALTARGET ?= $(if $(FINALTARGET),$(FINALTARGET),)
  $(1)_ROMFS ?= $(ROMFS)
endef
$(foreach pkg,$(PKG_y),$(eval $(call base_pkg_def,$(pkg))))

#
# the main target
#
all: $(foreach pkg,$(PKG_y),build/$(pkg)-$($(pkg)_FINALTARGET)) $(COMPLETETARGET)

define base_pkg_dep
build/$(1)-$(CONFIGTARGET): $(foreach dep,$($(1)_DEP),build/$(dep)-$($(dep)_FINALTARGET))
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
		$(UCFRONT_ENV) $(MAKE) -C build/$($*_BUILDDIR) $($*_BUILD) $(MAKEVARS) $($*_MAKEVARS)
	$(AT)touch $@

build/%-installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)$(BUILDVARS) \
	$($*_BUILDVARS) \
	$($*_INSTALLVARS) \
		$(UCFRONT_ENV) $(MAKE) -C build/$($*_BUILDDIR) $($*_INSTALL) DESTDIR=`pwd`/build/$*-install $(MAKEVARS) $($*_MAKEVARS)
	$(AT)touch $@

build/%-lndir: build/%-$(INSTALLTARGET) force_target $(if $(AUTOMAKE_PRESTAGE),$(AUTOMAKE_PRESTAGE),)
	@echo "Staging $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)cd build/$*-install; find . \( -type f -o -type l \) | egrep -v '.la$$' | \
	while read t; do \
		case "$$t" in \
		./bin/*) continue;; \
		./sbin/*) continue;; \
		esac; \
		rm -f $(STAGEDIR)/$$t; \
		mkdir -p "`dirname \"$(STAGEDIR)/$$t\"`"; \
		ln -s "`pwd`/$$t" "$(STAGEDIR)/$$t"; \
		echo "ln -s \"`pwd`/$$t\" \"$(STAGEDIR)/$$t\""; \
	done
	$(AT)touch $@

clean: $(foreach pkg,$(PKG_y),$($(pkg)_CLEAN))
	rm -rf build downloads

romfs_pre:
ifneq ($(AUTOMAKE_ROMFS),)
	unset ACL_PKG; unset ACL_URL; unset ACL_LICENSE; \
	for i in build/*-license; do . $$i; done; \
	$(MAKE) $(AUTOMAKE_ROMFS)
else
	@echo "Romfs pre-install ..."
	$(AT)for i in build/*-install; do \
		[ "$$i" = "build/*-install" ] && continue; \
		echo "Loading license `basename $$i -install`-license ..."; \
		unset ACL_PKG; unset ACL_URL; unset ACL_LICENSE; \
		. build/`basename $$i -install`-license; \
		echo "Installing `basename $$i -install` ..."; \
		[ ! -d $$i/usr ] || $(ROMFSINST) -v -f $$i/usr /usr; \
		[ ! -d $$i/bin ] || $(ROMFSINST) -v -f $$i/bin /bin; \
		[ ! -d $$i/sbin ] || $(ROMFSINST) -v -f $$i/sbin /sbin; \
		[ ! -d $$i/lib ] || $(ROMFSINST) -v -f $$i/lib /lib; \
		[ ! -d $$i/lib64 ] || $(ROMFSINST) -v -f $$i/lib64 /lib; \
		[ ! -d $$i/libexec ] || $(ROMFSINST) -v -f $$i/libexec /libexec; \
		rm -rf $$i/etc/default; \
		[ ! -d $$i/etc/config ] || mv $$i/etc/config $$i/etc/default; \
		[ ! -d $$i/etc ] || $(ROMFSINST) -v -f $$i/etc /etc; \
		[ ! -d $$i/share ] || $(ROMFSINST) -v -f $$i/share /share; \
	done
	rm -f $(ROMFSDIR)/lib/*.a
	rm -f $(ROMFSDIR)/lib/*.la
endif

romfs_post:
	for i in $(foreach pkg,$(PKG_y),$(pkg):$($(pkg)_ROMFS)); do \
		pkg=`expr $$i : '\(.*\):'`; \
		tgt=`expr $$i : '.*:\(.*\)'`; \
		[ "$$tgt" ] || continue; \
		echo "Loading license $$pkg-license ..."; \
		unset ACL_PKG; unset ACL_URL; unset ACL_LICENSE; \
		. build/$$pkg-license; \
		echo "Romfs $$pkg - $$tgt..."; \
		$(MAKE) $$tgt; \
	done

romfs: romfs_pre romfs_post
