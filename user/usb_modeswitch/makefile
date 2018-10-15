
LDFLAGS      += -lusb -lpthread
MAKEVARS     =  CC="$(CC) $(CFLAGS) -I."
MAKEVARS     += JIM_CONFIGURE_OPTS="--disable-lineedit --with-out-jim-ext=\"stdlib posix load signal syslog\" --prefix="
CONFVARS     =  $(MAKEVARS)

##########################################################

usb-modeswitch-1.2.4_URL         = \
	http://www.draisberghof.de/usb_modeswitch/usb-modeswitch-1.2.4.tar.bz2
usb-modeswitch-1.2.4_MAKEVARS  = $(MAKEVARS) UDEVDIR=`pwd`/build/usb-modeswitch-1.2.4-install/libexec/udev
usb-modeswitch-1.2.4_INSTALL     = install-static

usb-modeswitch-data-20120815_URL = \
	http://www.draisberghof.de/usb_modeswitch/usb-modeswitch-data-20120815.tar.bz2
usb-modeswitch-data-20120815_MAKEVARS  = $(MAKEVARS) RULESDIR=`pwd`/build/usb-modeswitch-data-20120815-install/libexec/udev/rules.d

##########################################################

usb-modeswitch-2.3.0_URL         = \
	http://www.draisberghof.de/usb_modeswitch/usb-modeswitch-2.3.0.tar.bz2
usb-modeswitch-2.3.0_MAKEVARS  = $(MAKEVARS) UDEVDIR=`pwd`/build/usb-modeswitch-2.3.0-install/libexec/udev
usb-modeswitch-2.3.0_INSTALL     = install-static

usb-modeswitch-data-20160112_URL = \
	http://www.draisberghof.de/usb_modeswitch/usb-modeswitch-data-20160112.tar.bz2
usb-modeswitch-data-20160112_MAKEVARS  = $(MAKEVARS) RULESDIR=`pwd`/build/usb-modeswitch-data-20160112-install/libexec/udev/rules.d

##########################################################

# AUTOMAKE_y   += usb-modeswitch-1.2.4 usb-modeswitch-data-20120815
AUTOMAKE_y   += usb-modeswitch-2.3.0 usb-modeswitch-data-20160112

include $(ROOTDIR)/tools/automake.inc

