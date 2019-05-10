
PKG        = ModemManager
URL        = git://anongit.freedesktop.org/ModemManager/ModemManager
METHOD     = git
VERSION    = 98fbc7a8b39aa84c905a7ce3c7b412ee04566f26
#BUILDDIR  = ModemManager-1.2.0
#URL       = http://cgit.freedesktop.org/ModemManager/ModemManager/snapshot/ModemManager-1.2.0.tar.gz
CONFIGURE  = autogen.sh

CFLAGS    += -Wno-error

CONFOPTS   = --disable-gtk-doc-html
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

# the tool versions need to match the library, so don't use the host system tools
$(shell mkdir -p build/bin; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.57.1-install/bin/gdbus-codegen build/bin/.; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.57.1-install/bin/glib-mkenums build/bin/.)
MAKEVARS += PATH=$(shell pwd)/build/bin:$$PATH UNINSTALLED_GLIB_SRCDIR=$(ROOTDIR)/lib/glib/build/glib-2.57.1

ModemManager_ROMFS = mm_romfs

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
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_VIA \
			/lib/ModemManager/libmm-plugin-via.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_HSO \
			/lib/ModemManager/libmm-plugin-hso.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_IRIDIUM \
			/lib/ModemManager/libmm-plugin-iridium.so
	$(ROMFSINST) -R -E CONFIG_USER_MODEMMANAGER_PLUGIN_SIERRA \
			/lib/ModemManager/libmm-plugin-sierra.so
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
endif

