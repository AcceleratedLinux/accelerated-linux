
URL       = http://downloads.sourceforge.net/project/tcl/Tcl/8.4.19/tcl8.4.19-src.tar.gz
BUILDDIR  = tcl8.4.19/unix
CONFVARS  = ac_cv_func_strtod=yes tcl_cv_strtod_unbroken=ok
CONFOPTS  = --disable-symbols --disable-langinfo --disable-framework
ifdef USER_TCL_STATIC
CONFOPTS += --disable-shared
endif

AUTOMAKE_ROMFS = tcl84_romfs

include $(ROOTDIR)/tools/automake.inc

tcl84_romfs:
ifndef USER_TCL_LIBTCL_ONLY
	$(ROMFSINST) build/tcl8.4.19-src-install/bin/tclsh8.4 /bin/tclsh8.4
	$(ROMFSINST) -s /bin/tclsh8.4 /bin/tclsh
endif
ifndef USER_TCL_STATIC
	$(ROMFSINST) build/tcl8.4.19-src-install/lib/libtcl8.4.so /lib/libtcl8.4.so
endif
	$(ROMFSINST) build/tcl8.4.19-src-install/lib/tcl8.4 /lib/tcl8.4
ifndef USER_TCL_INSTALL_ENCODINGS
	rm -rf $(ROMFSDIR)/lib/tcl8.4/encoding
endif

