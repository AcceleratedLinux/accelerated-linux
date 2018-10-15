# Makefile to download and patch tarball's etc

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= patched
include $(ROOTDIR)/tools/base.inc

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

build/$(1)-extracted: downloads/$($(1)_DOWNLOADNAME)
	@echo "Extracting $$< ..."
	$(AT)rm -rf build/$($(1)_SRCDIR)
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
# Support for remote GIT subdirectories of sorts, we do not remember master
# downloads as they change from load to load so just save it until the next
# clean.  The clone will save us downloading it all again.
#
# we do not update our clone if we are using the same version as previous
# clones.  Include the URL in the version test to be sure its the same
#
define download_dep_gitarchive

.PRECIOUS: downloads/$(1)$(if $($(1)_VERSION),-$($(1)_VERSION)).tar
downloads/$(1)$$(if $$($(1)_VERSION),-$$($(1)_VERSION)).tar:
	@echo "Downloading `basename $$@` ..."
	$(AT)[ -d "$(UC_DOWNLOADS)" ] || mkdir -p "$(UC_DOWNLOADS)"
	$(AT)if [ ! -f "$(UC_DOWNLOADS)/`basename $$@`" ]; then \
		echo "Updating/Cloning `basename $$($(1)_URL)` for archive ..."; \
		if [ -d "archive/`basename $$($(1)_URL)`" ]; then \
			if [ "$$$$(cat archive/`basename $$($(1)_URL)`-version)" = "$$($(1)_URL) $$(if $$($(1)_VERSION),$$($(1)_VERSION),master)" ]; then \
				(\
				cd "archive/`basename $$($(1)_URL)`"; \
				$(GIT) checkout -q master || exit 1; \
				$(GIT) pull $$($(1)_METHOD_OPTS) || exit 1; \
				$(GIT) checkout -q $$(if $$($(1)_VERSION),$$($(1)_VERSION),master) || exit 1; \
				$(GIT) submodule update; \
				echo "$$($(1)_URL) $$(if $$($(1)_VERSION),$$($(1)_VERSION),master)" > archive/`basename $$($(1)_URL)`-version; \
				) || exit 1; \
			else \
				echo "$(1) clone is up to date"; \
			fi ; \
		else \
			$(GIT) clone $$($(1)_METHOD_OPTS) $$($(1)_URL) archive/`basename $$($(1)_URL)` || exit 1; \
			(\
			cd "archive/`basename $$($(1)_URL)`"; \
			$(GIT) checkout -q $$(if $$($(1)_VERSION),$$($(1)_VERSION),master) || exit 1; \
			) || exit 1; \
		fi ; \
		cd archive/`basename $$($(1)_URL)`; \
			$(GIT) archive -o $(UC_DOWNLOADS)/`basename $$@` HEAD:$$(if $$($(1)_SUBDIR),$$($(1)_SUBDIR)/,) || exit 1; \
	fi
	$(AT)mkdir -p `dirname $$@`
	$(AT)if [ "$$(if $$($(1)_VERSION),$$($(1)_VERSION),master)" = "master" ]; then \
			mv -f "$(UC_DOWNLOADS)/`basename $$@`" $$@ || exit 1; \
	elif [ -f "$(UC_DOWNLOADS)/`basename $$@`" ]; then \
			ln -fs "$(UC_DOWNLOADS)/`basename $$@`" $$@; \
	fi
	$(AT)if [ ! -f "$$@" ]; then \
		echo "Cannot find download for $$@" >&2 ; \
		exit 1; \
	fi

build/$(1)-extracted: downloads/$(1)$(if $($(1)_VERSION),-$($(1)_VERSION)).tar
	@echo "Extracting $$< ..."
	$(AT)rm -rf build/$(1)
	$(AT)mkdir -p build/$(1)
	$(AT)cat < $$< | (cd build/$(1); tar xf -); exit $?
	$(AT)touch $$@

endef

#
# Support for remote GIT repos
#
define download_dep_git

build/$(1)-extracted:
	@echo "Cloning $(1) ..."
	$(AT)if [ -z "$$($(1)_URL)" ]; then \
		echo "*** URL for git clone of $(1) not specified." >&2; \
		exit 1; \
	fi
	$(AT)mkdir -p clone
	$(AT)if [ "$$($(1)_URL) $$(if $$($(1)_VERSION),$$($(1)_VERSION),master)" != "`cat clone/$(1)-version`" ]; then \
		if [ -f "clone/$(1)/.git/config" ]; then \
			cd "clone/$(1)" || exit 1; \
			$(GIT) remote set-url origin "$$($(1)_URL)" || exit 1; \
			$(GIT) fetch $$($(1)_METHOD_OPTS) || exit 1; \
			$(GIT) clean -d -f || exit 1; \
		else \
			rm -rf "clone/$(1)"; \
			$(GIT) clone $$($(1)_METHOD_OPTS) $$($(1)_URL) clone/$(1) || exit 1; \
		fi; \
		pwd; \
		rm -f clone/$(1)-version; \
	else \
		echo "$(1) already cloned at correct version, pulling updates."; \
		cd "clone/$(1)" || exit 1; \
		$(GIT) checkout -q master || exit 1; \
		$(GIT) pull $$($(1)_METHOD_OPTS) || exit 1; \
	fi
	$(AT)cd "clone/$(1)" &&	$(GIT) checkout -q $$(if $$($(1)_VERSION),$$($(1)_VERSION),origin/master) && $(GIT) submodule update
	$(AT)echo "$$($(1)_URL) $$(if $$($(1)_VERSION),$$($(1)_VERSION),master)" > clone/$(1)-version
	$(AT)rm -rf build/$(1)
	$(AT)mkdir -p build/$(1)
	$(AT)lndir -silent `pwd`/clone/$(1) build/$(1)
	$(AT)touch $$@

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
	$(AT)touch $@
