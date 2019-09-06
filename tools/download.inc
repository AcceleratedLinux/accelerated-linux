# Makefile to download and patch tarball's etc

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= patched

#
# You can override the downloads cache dir in case you would prefer
# have all the files in tree or elsewhere.  The default is ~/.downloads
#
ifndef UC_DOWNLOADS
UC_DOWNLOADS = $(HOME)/.downloads
endif

#
# we use wget to download
#
WGET ?= wget
GIT ?= git
METHOD ?= default

define download_pkg_def
  $(1)_URL ?= $(URL)
  $(1)_METHOD ?= $(METHOD)
  $(1)_METHOD_OPTS ?= $(METHOD_OPTS)
  $(1)_VERSION ?= $(VERSION)
  $(1)_SUBDIR ?= $(SUBDIR)
  $(1)_DOWNLOADNAME ?= $(if $(DOWNLOADNAME),$(DOWNLOADNAME),$$(shell basename "$$($(1)_URL)"))
endef
$(foreach pkg,$(PKG_y),$(eval $(call download_pkg_def,$(pkg))))

# put arguments on their own line
define oneline
$(1)
endef

# Backwards compat for patch command
PATCH = patch $(shell if patch --follow-symlinks < /dev/null > /dev/null 2>&1 ; then echo --follow-symlinks ; fi)
PATCH_SERIES ?= series

ifeq ($(PATCH_LEVEL),)
  PATCH_LEVEL = -p1 -b
endif

.PRECIOUS: $(addprefix build/,$(addsuffix -extracted,$(PKG_y)))
.PRECIOUS: $(addprefix build/,$(addsuffix -patched,$(PKG_y)))

#
# extract an archive ready for patching/building
#
define extract_dep_default

build/$(1)-extracted: downloads/$($(1)_DOWNLOADNAME)
	@echo "Extracting $$< ..."
	$(AT)rm -rf build/$($(1)_SRCDIR)
	$(AT)rm -rf build/$($(1)_BUILDDIR)
	$(AT)mkdir -p build
	$(AT)case "$$<" in \
	*zip) rm -rf build/$(1); mkdir -p build/$(1); pkg=`pwd`/$$<; (cd build/$(1); unzip $$$$pkg);; \
	*bz2) bunzip2 < $$< | (cd build; tar xf -);; \
	*xz) unxz < $$< | (cd build; tar xf -);; \
	*lz) lzip -d < $$< | (cd build; tar xf -);; \
	*) gunzip < $$< | (cd build; tar xf -);; \
	esac || exit 1
	$(AT)touch $$@

endef

#
# download support for tarballs and the like from a URL
#
define download_dep_default

.PRECIOUS: downloads/$$($(1)_DOWNLOADNAME)
downloads/$$($(1)_DOWNLOADNAME):
	@echo "Downloading $$($(1)_DOWNLOADNAME) from $($(1)_URL) ..."
	$(AT)[ -d "$(UC_DOWNLOADS)" ] || mkdir -p "$(UC_DOWNLOADS)"
	$(AT)if [ ! -f "$(UC_DOWNLOADS)/`basename $$@`" ]; then \
		cd "$(UC_DOWNLOADS)"; \
		$(WGET) $(WGETOPTS) -O `basename $$@` "$($(1)_URL)" || rm -f `basename $$@`; \
	fi
	$(AT)mkdir -p `dirname $$@`
	$(AT)if [ -f "$(UC_DOWNLOADS)/`basename $$@`" ]; then \
			ln -fs "$(UC_DOWNLOADS)/`basename $$@`" $$@; \
	fi
	$(AT)if [ ! -f "$$@" ]; then \
		echo "Cannot find download for $$@" >&2 ; \
		exit 1; \
	fi

$(call extract_dep_default,$1)

endef

#
# Support for remote GIT repos, github is special and does not support
# git-archive
#

define download_dep_git

.PRECIOUS: downloads/$$($(1)_DOWNLOADNAME)
downloads/$$($(1)_DOWNLOADNAME):
	@echo "Git checking $$($(1)_DOWNLOADNAME) from $($(1)_URL) ..."
	$(AT)[ -d "$(UC_DOWNLOADS)" ] || mkdir -p "$(UC_DOWNLOADS)"
	$(AT) mkdir -p `dirname $$@`; \
	: quick check to see if we can avoid git use; \
	VERSION="$($(1)_VERSION)"; \
	HASH=$$$$(echo -n $($(1)_URL) $$$${VERSION} | openssl dgst -sha256 | sed 's/.* //'); \
	if [ ! -f "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" ]; then \
		: Can not avoid git,  but keep it lightweight; \
		[ "$$$${VERSION}" ] || VERSION=master; \
		REF=$$$$($(GIT) ls-remote $($(1)_URL) | grep "[^/]refs/[a-z]*/$$$${VERSION}$$$$" | awk '{ print $$$$1 }'); \
		[ "$$$${REF}" ] || REF=$$$${VERSION}; \
		if [ "$$$$(echo $$$${REF} | wc -w)" -gt 1 ]; then \
			echo "ERROR: Ambiguous VERSION could be branch or tag ($$$${REF})"; \
			exit 1; \
		fi; \
		HASH=$$$$(echo -n $($(1)_URL) $$$${REF} | openssl dgst -sha256 | sed 's/.* //'); \
	fi; \
	if [ ! -f "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" ]; then \
		: Ok, time to build our own hashed tarball ; \
		( \
			cd `dirname $$@`;\
			rm -rf $(1); \
			$(GIT) clone $($(1)_URL) $(1) || exit 1;\
			cd $(1); \
			$(GIT) checkout $$$${REF} || exit 1; \
			$(GIT) submodule update --init --recursive || exit 1; \
			cd ..; \
			rm -rf $(1)/.git; \
			tar cvzf "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" $(1) || exit 1;\
			rm -rf $(1); \
		) \
	fi; \
	if [ -f "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" ]; then \
			ln -fs "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" $$@; \
	fi
	$(AT)if [ ! -f "$$@" ]; then \
		echo "Git cannot fetch download for $$@" >&2 ; \
		exit 1; \
	fi

$(call extract_dep_default,$1)

endef

$(foreach pkg,$(PKG_y),$(eval $(call download_dep_$($(pkg)_METHOD),$(pkg))))

$(foreach pkg,$(PKG_y), $(eval $(call oneline,build/$(pkg)-extracted: $(wildcard patches/$(PATCH_SERIES) patches/$(pkg)*.patch) $(shell cat patches/$(PATCH_SERIES) 2> /dev/null| sed 's?^?patches/?') $(MAKEFILE_LIST))))

build/%-patched: build/%-extracted
	$(AT)if [ -f "patches/$(PATCH_SERIES)" ]; then \
		while read t; do \
			[ -f patches/$$t ] || continue; \
			echo "Patching $* with patches/$$t from $(PATCH_SERIES)"; \
			case "$$t" in \
			*.gz) (cd build/$($*_SRCDIR); gunzip | $(PATCH) -E $(PATCH_LEVEL)) < patches/$$t || exit 1;; \
			*)    (cd build/$($*_SRCDIR); $(PATCH) -E $(PATCH_LEVEL)) < patches/$$t || exit 1;; \
			esac || exit 1; \
		done < patches/$(PATCH_SERIES) || exit 1; \
	else \
		for t in patches/$**.patch*; do \
			[ -f "$$t" ] || continue; \
			echo "Patching $* with $$t"; \
			case "$$t" in \
			*.gz) (cd build/$($*_SRCDIR); gunzip | $(PATCH) -E $(PATCH_LEVEL)) < $$t || exit 1 ;; \
			*)    (cd build/$($*_SRCDIR); $(PATCH) -E $(PATCH_LEVEL)) < $$t || exit 1;; \
			esac || exit 1; \
		done || exit 1; \
	fi
	$(AT)mkdir -p build
	echo "ACL_LICENSE='$(if $($(*)_LICENSE),$($(*)_LICENSE),$(if $(ACL_LICENSE),$(ACL_LICENSE),`$(ROOTDIR)/bin/license-detect.sh build/$($(*)_BUILDDIR)`))'" > build/$(*)-license
	echo "ACL_URL='$($(*)_URL)'" >> build/$(*)-license
	echo "ACL_PKG='$(*)'" >> build/$(*)-license
	echo "export ACL_LICENSE ACL_URL ACL_PKG" >> build/$(*)-license
	$(AT)touch $@
