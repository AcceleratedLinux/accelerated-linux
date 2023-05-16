
VERSION = 6.3
URL = http://ftp.gnu.org/pub/gnu/ncurses/ncurses-$(VERSION).tar.gz
COMMON_CONFOPTS  = --without-ada
COMMON_CONFOPTS += --without-cxx
COMMON_CONFOPTS += --without-manpages
COMMON_CONFOPTS += --without-profile
COMMON_CONFOPTS += --without-debug
COMMON_CONFOPTS += --without-gpm
COMMON_CONFOPTS += --without-sysmouse
COMMON_CONFOPTS += --enable-overwrite
COMMON_CONFOPTS += --disable-big-core
COMMON_CONFOPTS += --without-tests


PKG_y += ncurses-$(VERSION)
PKG_y += ncurses-host
ncurses-host_FINALTARGET = built
ncurses-host_METHOD = none

ncurses-$(VERSION)_DEP = ncurses-host
ncurses-$(VERSION)_CONFOPTS = $(COMMON_CONFOPTS)
ifdef CONFIG_USER_FLATFSD_FLATFSD
ncurses-$(VERSION)_CONFOPTS += --with-terminfo-dirs=/etc/config/terminfo:/etc/terminfo
else
ncurses-$(VERSION)_CONFOPTS += --with-terminfo-dirs=/etc/terminfo
endif
ncurses-$(VERSION)_CONFOPTS += --without-progs
ncurses-$(VERSION)_CONFOPTS += --with-shared
ncurses-$(VERSION)_CONFOPTS += --with-termlib
ncurses-$(VERSION)_CONFOPTS += --datadir=/usr/share
ncurses-$(VERSION)_CONFVARS = ac_cv_prog_ac_ct_TIC="$(CURDIR)/build/ncurses-host-install/tic"

include $(ROOTDIR)/tools/automake.inc

#
# ncurses build needs 'tic' from the same ncurses version, so we're building it
# first for the host, and we'll use that one later
#
.PRECIOUS: build/ncurses-host-built
build/ncurses-host-built:
	rm -Rf build/ncurses-host*
	cp -a build/ncurses-$(VERSION) build/ncurses-host
	ln -s ncurses-$(VERSION)-license build/ncurses-host-license

	cd build/ncurses-host; \
	export PKG_CONFIG_LIBDIR= PKG_CONFIG_PATH= EXTRA_CFLAGS= CPUFLAGS= CPU_CFLAGS= CXXFLAGS= CPPFLAGS= LDLIBS= LIBC= LIBS= CC=gcc CXX=g++ LD=gcc AR= RANLIB= CFLAGS= LDFLAGS= ; \
	$(CONFVARS) ./configure \
		--prefix=$$(pwd) \
		$(COMMON_CONFOPTS) \
		|| exit 1; \
	make libs || exit 1; \
	make -C progs tic || exit 1; \
	mkdir -p "$$(pwd)-install"; \
	cp -a progs/tic "$$(pwd)-install/" || exit 1

	touch $@

romfs:
ifdef CONFIG_LIB_NCURSES_MINIMAL
	mkdir -p $(ROMFSDIR)/usr/share/terminfo.new/a
	mkdir -p $(ROMFSDIR)/usr/share/terminfo.new/v
	mv $(ROMFSDIR)/usr/share/terminfo/v/vt100 $(ROMFSDIR)/usr/share/terminfo.new/v/
	mv $(ROMFSDIR)/usr/share/terminfo/a/ansi $(ROMFSDIR)/usr/share/terminfo.new/a/
	rm -rf $(ROMFSDIR)/usr/share/terminfo
	mv $(ROMFSDIR)/usr/share/terminfo.new $(ROMFSDIR)/usr/share/terminfo
endif
	$(ROMFSINST) -R /bin/ncurses6-config
