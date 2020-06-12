
URL       = http://downloads.sourceforge.net/project/tcl/Tcl/8.6.10/tcl8.6.10-src.tar.gz
BUILDDIR  = tcl8.6.10/unix
CONFVARS  = ac_cv_func_strtod=yes tcl_cv_strtod_unbroken=ok
CONFOPTS  = --disable-symbols --disable-langinfo --disable-framework
ifdef USER_TCL_STATIC
CONFOPTS += --disable-shared
endif

AUTOMAKE_ROMFS = tcl86_romfs

include $(ROOTDIR)/tools/automake.inc

tcl86_romfs:
ifndef USER_TCL_LIBTCL_ONLY
	$(ROMFSINST) build/tcl8.6.10-src-install/bin/tclsh8.6 /bin/tclsh8.6
	$(ROMFSINST) -s /bin/tclsh8.6 /bin/tclsh
endif
ifndef USER_TCL_STATIC
	$(ROMFSINST) build/tcl8.6.10-src-install/lib/libtcl8.6.so /lib/libtcl8.6.so
endif
	$(ROMFSINST) build/tcl8.6.10-src-install/lib/tcl8.6 /lib/tcl8.6
ifndef USER_TCL_INSTALL_ENCODINGS
	rm -rf $(ROMFSDIR)/lib/tcl8.6/encoding
endif

