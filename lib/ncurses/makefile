
URL = http://ftp.gnu.org/pub/gnu/ncurses/ncurses-5.9.tar.gz
CONFOPTS  = --without-ada
CONFOPTS += --without-cxx
CONFOPTS += --without-progs
CONFOPTS += --without-manpages
CONFOPTS += --with-shared
CONFOPTS += --without-profile
CONFOPTS += --without-debug
CONFOPTS += --without-gpm
CONFOPTS += --without-sysmouse
ifdef CONFIG_USER_FLATFSD_FLATFSD
CONFOPTS += --with-terminfo-dirs=/etc/config/terminfo:/etc/terminfo
else
CONFOPTS += --with-terminfo-dirs=/etc/terminfo
endif
CONFOPTS += --enable-overwrite
CONFOPTS += --disable-big-core
CONFOPTS += --datadir=/usr/share

include $(ROOTDIR)/tools/automake.inc

ifdef CONFIG_LIB_NCURSES_MINIMAL
romfs:
	mkdir -p $(ROMFSDIR)/usr/share/terminfo.new/a
	mkdir -p $(ROMFSDIR)/usr/share/terminfo.new/v
	mv $(ROMFSDIR)/usr/share/terminfo/v/vt100 $(ROMFSDIR)/usr/share/terminfo.new/v/
	mv $(ROMFSDIR)/usr/share/terminfo/a/ansi $(ROMFSDIR)/usr/share/terminfo.new/a/
	rm -rf $(ROMFSDIR)/usr/share/terminfo
	mv $(ROMFSDIR)/usr/share/terminfo.new $(ROMFSDIR)/usr/share/terminfo
endif
