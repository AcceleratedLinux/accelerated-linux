URL = https://www.freedesktop.org/software/libqmi/libqmi-1.26.4.tar.xz
DOWNLOADHASH = c0adcb84ba452e3e13edeb3f94f7b003ea927b354bc187629e05efb333ce270b

CONFOPTS = --without-docs --without-tests
CONFOPTS += --enable-more-warnings=no
ifdef CONFIG_LIB_GLIB_STATIC
CONFOPTS += --enable-static --disable-shared
CONFVARS = LIBS="-lgmodule-2.0 -lglib-2.0 -lffi -lz -lpthread -ldl"
endif
MAKEVARS = 
ROMFS    = libqmi_romfs

AUTORECONF = autoreconf -i

ifneq ($(CONFIG_DEFAULTS_DIGI_TX64)$(CONFIG_DEFAULTS_DIGI_TX64_PR),)
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
