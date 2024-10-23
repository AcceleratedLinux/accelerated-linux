URL      = https://gitlab.freedesktop.org/mobile-broadband/libqmi.git
METHOD   = git
VERSION  = 1.34.0

CONFOPTS += -Dbash_completion=false
CONFOPTS += -Dqrtr=false
CONFOPTS += -Dintrospection=false
CONFOPTS += -Dman=false
CONFOPTS += -Dudevdir=/lib/udev

ROMFS    = libqmi_romfs

ifneq ($(CONFIG_DEFAULTS_DIGI_TX64)$(CONFIG_DEFAULTS_DIGI_TX64_PR)$(CONFIG_DEFAULTS_DIGI_TX64_RAIL_SINGLE_CELLULAR_PR),)
PATCH_SERIES = series-TX64
endif

include $(ROOTDIR)/tools/meson.inc

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
