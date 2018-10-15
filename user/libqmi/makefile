
METHOD    = git
VERSION   = a3cadbe15688ba5660ba9332f0c67c1c2b26f86c
URL       = git://anongit.freedesktop.org/libqmi

CONFIGURE = autogen.sh
CONFOPTS = --without-docs --without-tests
CONFOPTS += --enable-more-warnings=no
ifdef CONFIG_LIB_GLIB_STATIC
CONFOPTS += --enable-static --disable-shared
CONFVARS = LIBS="-lgmodule-2.0 -lglib-2.0 -lffi -lz -lpthread -ldl"
endif
MAKEVARS = 
ROMFS    = libqmi_romfs

include $(ROOTDIR)/tools/automake.inc

libqmi_romfs:
ifndef CONFIG_USER_LIBQMI_QMICLI
	$(ROMFSINST) -R /bin/qmicli
endif
ifndef CONFIG_USER_LIBQMI_QMI_NETWORK
	$(ROMFSINST) -R /bin/qmi-network
endif
ifndef CONFIG_USER_LIBQMI_QMI_FIRMWARE_UPDATE
	$(ROMFSINST) -R /bin/qmi-firmware-update
endif
