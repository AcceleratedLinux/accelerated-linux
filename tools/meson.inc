# Makefile to build meson projects

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= meson_configured
BUILDTARGET ?= meson_built
INSTALLTARGET ?= meson_installed
include $(ROOTDIR)/tools/base.inc
include $(ROOTDIR)/tools/download.inc

MESON ?= meson
NINJA ?= ninja
MESON_BUILD_OPTS ?= -j$(HOST_NCPU)
MESON_BUILD_OPTS += $(if $(V:0=),-v)
MESON_INSTALL_OPTS ?= --no-rebuild

ifneq ($(findstring i386,$(MACHINE)),)
MESON_ARCH = x86
else ifneq ($(findstring arm64,$(MACHINE)),)
MESON_ARCH = aarch64
else
MESON_ARCH = $(MACHINE)
endif

# Expands a make variable, intended for shell expansion, into a
# python list of quoted strings, which is then quoted for use in
# an unquoted sed expression, s%..%..%
# For example, the CPPFLAGS -DPATH='"/x[y%z\n"' quotes properly
make-python-list = "$$(python -c "import sys,re;print(re.escape(repr(sys.argv[1:])).replace('%','\\\\%'))" $1)"

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
	    -e  s%@CFLAGS@%$(call make-python-list,$(CFLAGS) $($*_CFLAGS))%g \
	    -e  s%@CXXFLAGS@%$(call make-python-list,$(CXXFLAGS) $($*_CXXFLAGS))%g \
	    -e  s%@LDFLAGS@%$(call make-python-list,$(LDFLAGS) $($*_LDFLAGS))%g \
	    -e 's%@ARCH@%$(MESON_ARCH)%g' \
	    -e 's%@CPU@%$(MESON_ARCH)%g' \
	    -e 's%@ENDIAN@%$(ENDIAN)%g' \
	    -e 's%@STAGING_DIR@%$(STAGEDIR)%g' \
	    $(ROOTDIR)/tools/meson/meson-cross.conf.in \
	    > build/meson-cross.conf
	$(AT)$(CONFVARS) $($*_CONFVARS) \
		$(MESON) \
			setup \
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
		$(UCFRONT_ENV) $(MESON) compile $(MESON_BUILD_OPTS) $(MAKEVARS) $($*_MAKEVARS) -C build/$($*_BUILDDIR)/build
	$(AT)touch $@

build/%-meson_installed: build/%-$(BUILDTARGET)
	@echo "Meson installing $* ..."
	$(AT)mkdir -p build/$*-install
	$(AT)$(BUILDVARS) $($*_BUILDVARS) $($*_INSTALLVARS) \
		DESTDIR=`pwd`/build/$*-install \
		$(UCFRONT_ENV) $(MESON) install $(MESON_INSTALL_OPTS) $(MAKEVARS) $($*_MAKEVARS) -C build/$($*_BUILDDIR)/build
	$(AT)touch $@
