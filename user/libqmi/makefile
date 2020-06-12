
METHOD    = git
VERSION   = 9d75bb2fad2329601bfb0be14294d6256df8bbd7 # 1.25.4+ master 2020-05-13
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

ifdef CONFIG_DEFAULTS_DIGI_TX64
PATCH_SERIES = series-TX64
endif

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
