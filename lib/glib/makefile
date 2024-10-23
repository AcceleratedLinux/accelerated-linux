VERSION = 2.81.0
URL := https://download.gnome.org/sources/glib/$(basename $(VERSION))/glib-$(VERSION).tar.xz
DOWNLOADHASH = 1665188ed9cc941c0a189dc6295e6859872523d1bfc84a5a84732a7ae87b02e4
WGETOPTS = --no-check-certificate

CONFOPTS += -Dlibmount=disabled
CONFOPTS += -Dtests=false
CONFOPTS += -Dinstalled_tests=false
CONFOPTS += -Dselinux=disabled		# build without selinux support
CONFOPTS += -Dxattr=false		# build without xattr support
CONFOPTS += -Ddocumentation=false	# Build API reference and tools documentation
CONFOPTS += -Dman-pages=disabled	# generate man pages [default=auto]
CONFOPTS += -Ddtrace=disabled		# include tracing support for dtrace
CONFOPTS += -Dsystemtap=disabled	# include tracing support for systemtap
CONFOPTS += -Dlibelf=disabled
CONFOPTS += -Dglib_debug=disabled
CONFOPTS += -Doss_fuzz=disabled

ifdef CONFIG_LIB_GLIB_STATIC
CONFOPTS += -Ddefault_library=static
endif

INSTALLTARGET = my_install
ROMFS = glib_cleanup

include $(ROOTDIR)/tools/meson.inc

build/%-my_install: build/%-meson_installed
	@# Remove '${bindir}' prefix before tools in pkg-config files to make
	@# them working with cross-compilation
	for f in ${addprefix build/glib-$(VERSION)-install/lib/pkgconfig/, glib-2.0.pc gio-2.0.pc}; do \
		sed -i 's%$${bindir}/%%g' $$f; \
	done

glib_cleanup:
	rm -rf $(ROMFSDIR)/lib/glib-2.0
	rm -rf $(ROMFSDIR)/share/locale
ifndef CONFIG_LIB_GLIB_TOOLS
	@for i in build/glib-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done
endif

