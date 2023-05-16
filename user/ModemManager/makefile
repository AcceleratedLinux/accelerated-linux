URL                               = https://www.freedesktop.org/software/ModemManager/ModemManager-1.14.0.tar.xz
SHASUM_ModemManager-1.14.0.tar.xz = 2af06f619e716eeb75577f74a3e2c444d84d011d

CFLAGS    += -Wno-error

CONFOPTS   = --disable-gtk-doc-html --with-at-command-via-dbus
ifdef CONFIG_LIB_GLIB_STATIC
CONFVARS   = LIBS="-lgmodule-2.0 -lglib-2.0 -lffi -lz -lpthread -ldl"
CONFOPTS   += --enable-static --disable-shared
endif
#CONFOPTS  += --with-tests
CONFOPTS   += --with-udev-base-dir=/libexec/udev
ifdef CONFIG_USER_LIBMBIM
CONFVARS   += MBIM_CFLAGS="-I$(STAGEDIR)/include/libmbim-glib/."
CONFVARS   += MBIM_LIBS="-lmbim-glib"
else
CONFOPTS   += --without-mbim
endif
MAKEVARS   = 

AUTORECONF = NOCONFIGURE=true ./autogen.sh

# the tool versions need to match the library, so don't use the host system tools
$(shell mkdir -p build/bin; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.57.1-install/bin/gdbus-codegen build/bin/.; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.57.1-install/bin/glib-mkenums build/bin/.)
MAKEVARS += PATH=$(shell pwd)/build/bin:$$PATH UNINSTALLED_GLIB_SRCDIR=$(ROOTDIR)/lib/glib/build/glib-2.57.1

ROMFS = mm_romfs

include $(ROOTDIR)/tools/automake.inc

mm_romfs:
	rm -f $(ROMFSDIR)/lib/ModemManager/*.la
	rm -f $(ROMFSDIR)/lib/ModemManager/*.a
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d build/ModemManager*/data/org.freedesktop.ModemManager1.conf\
			/etc/default/dbus-1/system.d/org.freedesktop.ModemManager1.conf
else
	$(ROMFSINST) -d build/ModemManager*/data/org.freedesktop.ModemManager1.conf\
			/etc/dbus-1/system.d/org.freedesktop.ModemManager1.conf
endif

ifdef CONFIG_PROP_OGCS
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d build/ModemManager*/data/org.freedesktop.ModemManager1.service\
			/etc/default/dbus-1/system.d/org.freedesktop.ModemManager1.service
else
	$(ROMFSINST) -d build/ModemManager*/data/org.freedesktop.ModemManager1.service\
			/etc/dbus-1/system.d/org.freedesktop.ModemManager1.service
endif
endif

ifndef CONFIG_USER_MODEMMANAGER_ALLPLUGINS
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_ANYDATA \
			/lib/ModemManager/libmm-plugin-anydata.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_MOTOROLA \
			/lib/ModemManager/libmm-plugin-motorola.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_GENERIC \
			/lib/ModemManager/libmm-plugin-generic.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_SAMSUNG \
			/lib/ModemManager/libmm-plugin-samsung.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_MBM \
			/lib/ModemManager/libmm-plugin-mbm.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_ZTE \
			/lib/ModemManager/libmm-plugin-zte.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_PANTECH \
			/lib/ModemManager/libmm-plugin-pantech.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_NOKIA_ICERA \
			/lib/ModemManager/libmm-plugin-nokia-icera.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_NOVATEL_LTE \
			/lib/ModemManager/libmm-plugin-novatel-lte.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_WAVECOM \
			/lib/ModemManager/libmm-plugin-wavecom.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_NOVATEL \
			/lib/ModemManager/libmm-plugin-novatel.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_GOBI \
			/lib/ModemManager/libmm-plugin-gobi.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_CINTERION \
			/lib/ModemManager/libmm-plugin-cinterion.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_LONGCHEER \
			/lib/ModemManager/libmm-plugin-longcheer.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_OPTION \
			/lib/ModemManager/libmm-plugin-option.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_OPTIONHSO \
			/lib/ModemManager/libmm-plugin-option-hso.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_VIA \
			/lib/ModemManager/libmm-plugin-via.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_HSO \
			/lib/ModemManager/libmm-plugin-hso.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_IRIDIUM \
			/lib/ModemManager/libmm-plugin-iridium.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_SIERRA \
			/lib/ModemManager/libmm-plugin-sierra.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_SIERRALEGACY \
			/lib/ModemManager/libmm-plugin-sierra-legacy.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_NOKIA \
			/lib/ModemManager/libmm-plugin-nokia.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_HUAWEI \
			/lib/ModemManager/libmm-plugin-huawei.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_X22X \
			/lib/ModemManager/libmm-plugin-x22x.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_SIMTECH \
			/lib/ModemManager/libmm-plugin-simtech.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_LINKTOP \
			/lib/ModemManager/libmm-plugin-linktop.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_ALTAIRLTE \
			/lib/ModemManager/libmm-plugin-altair-lte.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_BROADMOBI \
			/lib/ModemManager/libmm-plugin-broadmobi.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_DELL \
			/lib/ModemManager/libmm-plugin-dell.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_DLINK \
			/lib/ModemManager/libmm-plugin-dlink.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_ERICSSONMBM \
			/lib/ModemManager/libmm-plugin-ericsson-mbm.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_FIBOCOM \
			/lib/ModemManager/libmm-plugin-fibocom.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_FOXCONN \
			/lib/ModemManager/libmm-plugin-foxconn.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_HAIER \
			/lib/ModemManager/libmm-plugin-haier.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_MTK \
			/lib/ModemManager/libmm-plugin-mtk.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_QUECTEL \
			/lib/ModemManager/libmm-plugin-quectel.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_TELIT \
			/lib/ModemManager/libmm-plugin-telit.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_THURAYA \
			/lib/ModemManager/libmm-plugin-thuraya.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_TPLINK \
			/lib/ModemManager/libmm-plugin-tplink.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_UBLOX \
			/lib/ModemManager/libmm-plugin-ublox.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_UNITAC \
			/lib/ModemManager/libmm-plugin-unitac.so
endif

