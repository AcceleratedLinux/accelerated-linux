VERSION = 2.71.0
URL := https://download.gnome.org/sources/glib/$(basename $(VERSION))/glib-$(VERSION).tar.xz
DOWNLOADHASH = 926816526f6e4bba9af726970ff87be7dac0b70d5805050c6207b7bb17ea4fca
WGETOPTS = --no-check-certificate

CONFOPTS += -Dlibmount=disabled
CONFOPTS += -Dtests=false -Dinstalled_tests=false
CONFOPTS += -Dselinux=disabled		# build without selinux support
CONFOPTS += -Dfam=false			# don't use fam for file system monitoring
CONFOPTS += -Dxattr=false		# build without xattr support
CONFOPTS += -Dgtk_doc=false		# use gtk-doc to build documentation [[default=no]]
CONFOPTS += -Dman=false			# generate man pages [default=auto]
CONFOPTS += -Ddtrace=false		# include tracing support for dtrace
CONFOPTS += -Dsystemtap=false		# include tracing support for systemtap
CONFOPTS += -Dlibelf=disabled
ifdef CONFIG_LIB_LIBICONV
CONFOPTS += -Diconv=external
CFLAGS += -I$(ROOTDIR)/lib/libiconv/build/libiconv-1.14-install/include
else
ifdef CONFIG_LIB_MINI_ICONV
CONFOPTS += -Diconv=external
else
CONFOPTS += -Diconv=auto
endif
endif
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

