
URL := http://ftp.gnome.org/pub/GNOME/sources/glib/2.37/glib-2.37.93.tar.xz

MAKEVARS = 

CONFVARS :=
CONFVARS += glib_cv_stack_grows=no
CONFVARS += glib_cv_uscore=no
CONFVARS += ac_cv_func_posix_getpwuid_r=yes
CONFVARS += ac_cv_func_posix_getgrgid_r=yes
CONFVARS += ZLIB_CFLAGS=" "

# CONFOPTS += --enable-gc-friendly
# CONFOPTS += --disable-mem-pools
CONFOPTS += --disable-rebuilds
CONFOPTS += --disable-modular-tests
CONFOPTS += --disable-largefile
# CONFOPTS += --enable-iconv-cache= # [yes/no/auto]
CONFOPTS += --disable-selinux       # build without selinux support
CONFOPTS += --disable-fam           # don't use fam for file system monitoring
CONFOPTS += --disable-xattr         # build without xattr support
CONFOPTS += --disable-gtk-doc       # use gtk-doc to build documentation [[default=no]]
CONFOPTS += --disable-gtk-doc-html  # build documentation in html format [[default=yes]]
CONFOPTS += --disable-gtk-doc-pdf   # build documentation in pdf format [[default=no]]
CONFOPTS += --disable-man           # generate man pages [default=auto]
CONFOPTS += --disable-dtrace        # include tracing support for dtrace
CONFOPTS += --disable-systemtap     # include tracing support for systemtap
CONFOPTS += --disable-gcov          # enable coverage testing with gcov
CONFOPTS += --disable-libelf
CONFOPTS += --disable-largefile
ifdef CONFIG_LIB_LIBICONV
CONFOPTS += --with-libiconv=gnu
CFLAGS += -I$(ROOTDIR)/lib/libiconv/build/libiconv-1.14-install/include
else
ifdef CONFIG_LIB_MINI_ICONV
CONFOPTS += --with-libiconv=native
else
CONFOPTS += --with-libiconv=no
endif
endif

ifdef CONFIG_LIB_GLIB_STATIC
CONFOPTS += --enable-static --disable-shared
endif

ROMFS = glib_cleanup

include $(ROOTDIR)/tools/automake.inc

glib_cleanup:
	rm -rf $(ROMFSDIR)/lib/glib-2.0
ifndef CONFIG_LIB_GLIB_TOOLS
	@for i in build/glib-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done
endif

