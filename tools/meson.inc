# Makefile to build meson projects

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= meson_configured
BUILDTARGET ?= meson_built
INSTALLTARGET ?= meson_installed
include $(ROOTDIR)/tools/base.inc
include $(ROOTDIR)/tools/download.inc

NINJA ?= ninja

ifneq ($(findstring i386,$(MACHINE)),)
MESON_ARCH = x86
else ifneq ($(findstring arm64,$(MACHINE)),)
MESON_ARCH = aarch64
else
MESON_ARCH = $(MACHINE)
endif

comma := ,
empty :=
space := $(empty) $(empty)

# Build a comma-separated list of items, from a space-separated
# list of items:   a b c d  -->  a, b, c, d
make-comma-list = $(subst $(space),$(comma)$(space),$(strip $(1)))

# Build a comma-separated list of single-quoted items, from a space-separated
# list of unquoted items:   a b c d  -->  'a', 'b', 'c', 'd'
make-sq-comma-list = $(call make-comma-list,$(patsubst %,'%',$(strip $(1))))

PRECIOUS: $(addprefix build/,$(addsuffix -meson_configured,$(PKG_y)))
PRECIOUS: $(addprefix build/,$(addsuffix -meson_built,$(PKG_y)))
PRECIOUS: $(addprefix build/,$(addsuffix -meson_installed,$(PKG_y)))

build/%-meson_configured: build/%-patched
	@echo "Meson configuring $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)/build
	$(AT)sed \
	    -e 's%@CC@%$(CC)%g' \
	    -e 's%@CXX@%$(CXX)%g' \
	    -e 's%@AR@%$(AR)%g' \
	    -e 's%@STRIP@%$(STRIP)%g' \
	    -e 's%@NM@%$(NM)%g' \
	    -e 's%@PKGCONFIG@%pkg-config%g' \
	    -e 's%@CMAKE@%cmake%g' \
	    -e "s%@CFLAGS@%$(call make-sq-comma-list,$(CFLAGS) $($*_CFLAGS))%g" \
	    -e "s%@CXXFLAGS@%$(call make-sq-comma-list,$(CXXFLAGS) $($*_CXXFLAGS))%g" \
	    -e "s%@LDFLAGS@%$(call make-sq-comma-list,$(LDFLAGS) $($*_LDFLAGS))%g" \
	    -e 's%@ARCH@%$(MESON_ARCH)%g' \
	    -e 's%@CPU@%$(MESON_ARCH)%g' \
	    -e 's%@ENDIAN@%$(ENDIAN)%g' \
	    -e 's%@STAGING_DIR@%$(STAGEDIR)%g' \
	    $(ROOTDIR)/tools/meson/meson-cross.conf.in \
	    > build/meson-cross.conf
	$(AT)$(CONFVARS) $($*_CONFVARS) \
		$(ROOTDIR)/tools/meson/build/install/meson.pyz \
			--prefix=/ \
			--libdir=lib \
			--cross-file build/meson-cross.conf \
			--buildtype=plain \
			$(CONFOPTS) $($*_CONFOPTS) \
			build/$($*_BUILDDIR) \
			build/$($*_BUILDDIR)/build
	$(AT)touch $@

build/%-meson_built: build/%-$(CONFIGTARGET)
	@echo "Meson building $* ..."
	$(AT)$(BUILDVARS) $($*_BUILDVARS) \
		$(UCFRONT_ENV) $(NINJA) -j$(HOST_NCPU) $(MAKEVARS) $($*_MAKEVARS) -C build/$($*_BUILDDIR)/build
	$(AT)touch $@

build/%-meson_installed: build/%-$(BUILDTARGET)
	@echo "Meson installing $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)$(BUILDVARS) $($*_BUILDVARS) $($*_INSTALLVARS) \
		DESTDIR=`pwd`/build/$*-install \
		$(UCFRONT_ENV) $(NINJA) $(MAKEVARS) $($*_MAKEVARS) -C build/$($*_BUILDDIR)/build $($*_INSTALL)
	$(AT)touch $@
