URL      = https://github.com/AcceleratedLinux/ModemManager.git
METHOD   = git
VERSION  = d5c23993eb1c0ffb516fefe377ddf46a4f62fe95
CFLAGS    += -Wno-error

CONFOPTS += -Dudev=true
CONFOPTS += -Dudevdir=/libexec/udev
CONFOPTS += -Dtests=false
CONFOPTS += -Dsystemdsystemunitdir=false
CONFOPTS += -Dsystemd_suspend_resume=false
CONFOPTS += -Dpowerd_suspend_resume=false
CONFOPTS += -Dsystemd_journal=false
CONFOPTS += -Dpolkit=no
CONFOPTS += -Dat_command_via_dbus=true
CONFOPTS += -Dbuiltin_plugins=true

ifdef CONFIG_USER_LIBMBIM
CONFOPTS += -Dmbim=true
else
CONFOPTS += -Dmbim=false
endif

CONFOPTS += -Dqmi=true
CONFOPTS += -Dqrtr=false
CONFOPTS += -Dintrospection=false
CONFOPTS += -Dvapi=false
CONFOPTS += -Dman=false
CONFOPTS += -Dgtk_doc=false
CONFOPTS += -Dbash_completion=false

ifndef CONFIG_USER_MODEMMANAGER_ALLPLUGINS
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_GENERIC
		CONFOPTS += -Dplugin_generic=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_ALTAIRLTE
		CONFOPTS += -Dplugin_altair_lte=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_ANYDATA
		CONFOPTS += -Dplugin_anydata=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_BROADMOBI
		CONFOPTS += -Dplugin_broadmobi=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_CINTERION
		CONFOPTS += -Dplugin_cinterion=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_DELL
		CONFOPTS += -Dplugin_dell=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_DLINK
		CONFOPTS += -Dplugin_dlink=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_FIBOCOM
		CONFOPTS += -Dplugin_fibocom=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_FOXCONN
		CONFOPTS += -Dplugin_foxconn=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_GOSUNCN
		CONFOPTS += -Dplugin_gosuncn=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_HAIER
		CONFOPTS += -Dplugin_haier=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_HUAWEI
		CONFOPTS += -Dplugin_huawei=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_INTEL
		CONFOPTS += -Dplugin_intel=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_IRIDIUM
		CONFOPTS += -Dplugin_iridium=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_LINKTOP
		CONFOPTS += -Dplugin_linktop=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_LONGCHEER
		CONFOPTS += -Dplugin_longcheer=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_MBM
		CONFOPTS += -Dplugin_mbm=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_MOTOROLA
		CONFOPTS += -Dplugin_motorola=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_MTK
		CONFOPTS += -Dplugin_mtk=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_NOKIA
		CONFOPTS += -Dplugin_nokia=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_NOKIA_ICERA
		CONFOPTS += -Dplugin_nokia_icera=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_NOVATEL
		CONFOPTS += -Dplugin_novatel=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_NOVATEL_LTE
		CONFOPTS += -Dplugin_novatel_lte=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_OPTION
		CONFOPTS += -Dplugin_option=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_OPTIONHSO
		CONFOPTS += -Dplugin_option_hso=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_PANTECH
		CONFOPTS += -Dplugin_pantech=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_QCOM_SOC
		CONFOPTS += -Dplugin_qcom_soc=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_QUECTEL
		CONFOPTS += -Dplugin_quectel=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_SAMSUNG
		CONFOPTS += -Dplugin_samsung=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_SIERRALEGACY
		CONFOPTS += -Dplugin_sierra_legacy=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_SIERRA
		CONFOPTS += -Dplugin_sierra=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_SIMTECH
		CONFOPTS += -Dplugin_simtech=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_TELIT
		CONFOPTS += -Dplugin_telit=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_THURAYA
		CONFOPTS += -Dplugin_thuraya=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_TPLINK
		CONFOPTS += -Dplugin_tplink=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_UBLOX
		CONFOPTS += -Dplugin_ublox=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_VIA
		CONFOPTS += -Dplugin_via=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_WAVECOM
		CONFOPTS += -Dplugin_wavecom=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_X22X
		CONFOPTS += -Dplugin_x22x=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_ZTE
		CONFOPTS += -Dplugin_zte=disabled
	endif
	ifndef CONFIG_USER_MODEMMANAGER_PLUGIN_UNITAC
		CONFOPTS += -Dplugin_unitac=disabled
	endif
endif

ROMFS = mm_romfs

include $(ROOTDIR)/tools/meson.inc

mm_romfs:
	rm -f $(ROMFSDIR)/lib/ModemManager/*.la
	rm -f $(ROMFSDIR)/lib/ModemManager/*.a
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d build/ModemManager-install/etc/dbus-1/system.d/org.freedesktop.ModemManager1.conf\
			/etc/default/dbus-1/system.d/org.freedesktop.ModemManager1.conf
else
	$(ROMFSINST) -d build/ModemManager-install/etc/dbus-1/system.d/org.freedesktop.ModemManager1.conf\
			/etc/dbus-1/system.d/org.freedesktop.ModemManager1.conf
endif

ifdef CONFIG_PROP_OGCS
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d build/ModemManager-install/share/dbus-1/system-services/org.freedesktop.ModemManager1.service\
			/etc/default/dbus-1/system.d/org.freedesktop.ModemManager1.service
else
	$(ROMFSINST) -d build/ModemManager-install/share/dbus-1/system-services/org.freedesktop.ModemManager1.service\
			/etc/dbus-1/system.d/org.freedesktop.ModemManager1.service
endif
endif
