
URL = http://ftp.gnu.org/pub/gnu/ncurses/ncurses-5.9.tar.gz
CONFOPTS  = --without-ada
CONFOPTS += --without-cxx
CONFOPTS += --without-progs
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

