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
WGET ?= wget $(if $(V:0=),,--quiet)
AWS ?= aws $(if $(V:0=),,--quiet)
GIT ?= git
METHOD ?= default
PATCH_SERIES ?= series
ifeq ($(shell expr length "$(DOWNLOADHASH)"),32)
DOWNLOADHASH_METHOD ?= md5
else
DOWNLOADHASH_METHOD ?= sha256
endif

define download_pkg_def
  $(1)_URL ?= $(URL)
  $(1)_METHOD ?= $(METHOD)
  $(1)_METHOD_OPTS ?= $(METHOD_OPTS)
  $(1)_VERSION ?= $(VERSION)
  $(1)_SUBDIR ?= $(SUBDIR)
  $(1)_DOWNLOADNAME ?= $(if $(DOWNLOADNAME),$(DOWNLOADNAME),$$(shell basename "$$($(1)_URL)"))
  $(1)_DOWNLOADHASH ?= $(DOWNLOADHASH)
  $(1)_DOWNLOADHASH_METHOD ?= $(DOWNLOADHASH_METHOD)
  $(1)_PATCH_SERIES ?= $(PATCH_SERIES)
  $(1)_PATCHES ?= $(PATCHES)
endef
$(foreach pkg,$(PKG_y),$(eval $(call download_pkg_def,$(pkg))))

# put arguments on their own line
define oneline
$(1)
endef

# Backwards compat for patch command
PATCH = patch $(shell if patch --follow-symlinks < /dev/null > /dev/null 2>&1 ; then echo --follow-symlinks ; fi)

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
ifdef $(1)_DOWNLOADHASH
	@echo "Verifying $$< using $($(1)_DOWNLOADHASH_METHOD) hash ..."
	$(AT)HASH=`openssl dgst -$($(1)_DOWNLOADHASH_METHOD) $$< | cut -d' ' -f 2`; \
	if [ "$$$$HASH" = "$($(1)_DOWNLOADHASH)" ]; then \
		echo "Download file $$($(1)_DOWNLOADNAME) passed hash verification"; \
	else \
		echo "Download file $$($(1)_DOWNLOADNAME) failed hash verification"; \
		echo "    Expected = $($(1)_DOWNLOADHASH)"; \
		echo "    Got      = $$$$HASH"; \
		exit 1; \
	fi
endif
	@echo "Extracting $$< ..."
	$(AT)rm -rf build/$($(1)_SRCDIR)
	$(AT)rm -rf build/$($(1)_BUILDDIR)
	$(AT)mkdir -p build
	$(AT)case "$$<" in \
	*zip) rm -rf build/$(1); mkdir -p build/$(1); pkg=`pwd`/$$<; (cd build/$(1); unzip $(if $($(1)_SUBDIR),-d $($(1)_SUBDIR),) $$$$pkg);; \
	*bz2) bunzip2 < $$< | (cd build; tar xf - $(if $($(1)_SUBDIR),--one-top-level=$($(1)_SUBDIR),));; \
	*xz) unxz < $$< | (cd build; tar xf - $(if $($(1)_SUBDIR),--one-top-level=$($(1)_SUBDIR),));; \
	*lz) lzip -d < $$< | (cd build; tar xf - $(if $($(1)_SUBDIR),--one-top-level=$($(1)_SUBDIR),));; \
	*) gunzip < $$< | (cd build; tar xf - $(if $($(1)_SUBDIR),--one-top-level=$($(1)_SUBDIR),));; \
	esac || exit 1
	$(AT)touch $$@

endef

define beginswith
  case $2 in "$1"*) true;; *) false;; esac
endef

#
# download support for tarballs and the like from a URL
#
define download_dep_default

.PRECIOUS: downloads/$$($(1)_DOWNLOADNAME)
downloads/$$($(1)_DOWNLOADNAME):
	@echo "Downloading $$($(1)_DOWNLOADNAME) ..."
	$(AT)[ -d "$(UC_DOWNLOADS)" ] || mkdir -p "$(UC_DOWNLOADS)"
	$(AT)if [ ! -f "$(UC_DOWNLOADS)/`basename $$@`" ]; then \
		cd "$(UC_DOWNLOADS)"; \
		for U in $$(if $$(UC_MIRROR),$$(addprefix $$(UC_MIRROR),$$(notdir $$($(1)_URL))),) $$($(1)_URL); do \
			echo " ... from $$$$U ..."; \
			if $(call beginswith,"s3://","$$$$U"); then \
				$(AWS) s3 cp "$$$$U" `basename $$@` && break || rm -f `basename $$@`;\
			else \
				$(WGET) $(WGETOPTS) -O `basename $$@` "$$$$U" && break || rm -f `basename $$@`; \
			fi \
		done \
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
downloads/$$($(1)_DOWNLOADNAME): $(MAKEFILE_LIST)
	@echo "Git checking $$($(1)_DOWNLOADNAME) from $($(1)_URL) ..."
	$(AT)[ -d "$(UC_DOWNLOADS)" ] || mkdir -p "$(UC_DOWNLOADS)"
	$(AT) mkdir -p `dirname $$@`; \
	: quick check to see if we can avoid git use; \
	VERSION="$($(1)_VERSION)"; \
	HASH=$$$$(echo -n $($(1)_URL) $$$${VERSION} | openssl dgst -sha256 | sed 's/.* //'); \
	if [ ! -f "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" ]; then \
		: Can not avoid git,  but keep it lightweight; \
		[ "$$$${VERSION}" ] || VERSION=master; \
		REFS=$$$$($(GIT) ls-remote $($(1)_URL) | grep "[^/]refs/[a-z]*/$$$${VERSION}$$$$"); \
		if [ "$$$$(echo $$$${REFS} | wc -l)" -gt 1 ]; then \
			echo "ERROR: Ambiguous VERSION could be branch or tag ($$$${REFS})"; \
			exit 1; \
		fi; \
		: check if its a branch and could change from build to build; \
		case "$$$$REFS" in \
		*"refs/tags/"*|"") REF="$$$$VERSION" : version is a unique tag or hash ;; \
		*)  REF=$$$$(echo "$$$$REFS" | awk '{ print $$$$1 }') ;; \
		esac; \
		HASH=$$$$(echo -n $($(1)_URL) $$$${REF} | openssl dgst -sha256 | sed 's/.* //'); \
	fi; \
	if [ ! -f "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" ]; then \
		: Ok, time to build our own hashed tarball ; \
		( \
			cd `dirname $$@`;\
			rm -rf $(1); \
			if [ "$($(1)_METHOD_OPTS)" = archive ]; then \
				$(GIT) archive --remote=$($(1)_URL) --format=tar.gz --output=$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz $$$$VERSION  $($(1)_SUBDIR) || \
				rm -f $(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz; \
			else \
				$(GIT) clone $($(1)_URL) $(1) || exit 1;\
				cd $(1); \
				$(GIT) checkout $$$${REF} || exit 1; \
				$(GIT) submodule update --init --recursive || exit 1; \
				cd ..; \
				rm -rf $(1)/.git; \
				tar cvzf "$(UC_DOWNLOADS)/$(1)-$$$${HASH}.tar.gz" $(1) || exit 1;\
			fi; \
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

$(foreach pkg,$(PKG_y), $(eval $(call oneline,build/$(pkg)-extracted: $(wildcard patches/$($(pkg)_PATCH_SERIES) patches/$(pkg)*.patch) $(shell cat patches/$($(pkg)_PATCH_SERIES) 2> /dev/null| sed 's?^?patches/?') $(addprefix patches/,$($(pkg)_PATCHES)) $(MAKEFILE_LIST))))

build/%-patched: build/%-extracted
	$(AT)if [ -f "patches/$($(*)_PATCH_SERIES)" ]; then \
		while read t; do \
			[ -f patches/$$t ] || continue; \
			echo "Patching $* with patches/$$t from $($(*)_PATCH_SERIES)"; \
			case "$$t" in \
			*.gz) (cd build/$($*_SRCDIR); gunzip | $(PATCH) -E $(PATCH_LEVEL)) < patches/$$t || exit 1;; \
			*)    (cd build/$($*_SRCDIR); $(PATCH) -E $(PATCH_LEVEL)) < patches/$$t || exit 1;; \
			esac || exit 1; \
		done < patches/$($(*)_PATCH_SERIES) || exit 1; \
	else \
		patches="$(addprefix patches/,$($(*)_PATCHES))"; \
		[ "$$patches" ] || patches=`find patches -iname '$(*)*.patch*' | sort`; \
		for t in $$patches; do \
			[ -f "$$t" ] || continue; \
			echo "Patching $* with $$t"; \
			case "$$t" in \
			*.gz) (cd build/$($*_SRCDIR); gunzip | $(PATCH) -E $(PATCH_LEVEL)) < $$t || exit 1 ;; \
			*)    (cd build/$($*_SRCDIR); $(PATCH) -E $(PATCH_LEVEL)) < $$t || exit 1;; \
			esac || exit 1; \
		done || exit 1; \
	fi
	$(AT)mkdir -p build
	echo "ACL_LICENSE='$(if $($(*)_LICENSE),$($(*)_LICENSE),$(if $(ACL_LICENSE),$(ACL_LICENSE),`$(ROOTDIR)/bin/license-detect.sh build/$($(*)_SRCDIR)`))'" > build/$(*)-license
	echo "ACL_URL='$($(*)_URL)'" >> build/$(*)-license
	echo "ACL_PKG='$(if $($(*)_PKG),$($(*)_PKG),$(if $(ACL_PKG),$(ACL_PKG),$(*)))'" >> build/$(*)-license
	echo "export ACL_LICENSE ACL_URL ACL_PKG" >> build/$(*)-license
	$(AT)touch $@
