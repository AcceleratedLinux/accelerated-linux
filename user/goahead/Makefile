.EXPORT_ALL_VARIABLES:

ifeq ("$(CONFIG_USER_MATRIXSSL)", "y")
DIRS = ../matrixssl-1.8.3 src cgi-src
else
DIRS = src cgi-src 
endif

ROOT_DIRECTORY = /etc_ro/web

all:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

romfs:
	$(ROMFSINST) $(ROOT_DIRECTORY)
ifeq ("$(CONFIG_USER_PPPOECD)", "y")
	mv $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan_simple.asp $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan.asp
else
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan_simple.asp
endif
ifeq ("$(CONFIG_RT2860V2_AP_MESH)", "")
ifeq ("$(CONFIG_RT2860V2_STA_MESH)", "")
	rm $(ROMFSDIR)$(ROOT_DIRECTORY)/wireless/mesh.asp
endif
endif
ifeq ("$(CONFIG_RT2860V2_AP_WDS)", "")
	rm $(ROMFSDIR)$(ROOT_DIRECTORY)/wireless/wds.asp
endif
ifeq ("$(CONFIG_RT2860V2_AP_APCLI)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/wireless/apcli.asp
endif
ifeq ("$(CONFIG_RT2860V2_RT3XXX_ANTENNA_DIVERSITY)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/wireless/ant_diversity.asp
endif
ifeq ("$(CONFIG_RT2860V2_STA)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/station
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/wps/wps_sta.asp
endif

ifeq ("$(CONFIG_USB)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb
else
ifeq ("$(CONFIG_USER_STORAGE)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/STORAGE*
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/SAMBA*
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/FTP*
endif
ifeq ("$(CONFIG_USER_PROFTPD)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/FTP*
endif
ifeq ("$(CONFIG_USER_SAMBA)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/SAMBA*
endif
ifeq ("$(CONFIG_USER_USHARE)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/USHARE*
endif
ifeq ("$(CONFIG_USER_UVC_STREAM)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/UVC*
endif
ifeq ("$(CONFIG_USER_P910ND)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/P910ND*
endif
ifeq ("$(CONFIG_USER_MTDAAPD)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/usb/iTunes*
endif
endif

ifeq ("$(CONFIG_RALINKAPP_MPLAYER)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/media
endif

ifeq ("$(CONFIG_USER_WIZARD)", "")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/adm/wizard.asp
endif

ifneq ("$(CONFIG_RTDEV_MII)", "") 
	$(ROMFSINST) inic $(ROOT_DIRECTORY)
else
ifneq ("$(CONFIG_RTDEV_USB)", "") 
	$(ROMFSINST) inic $(ROOT_DIRECTORY)
else
ifneq ("$(CONFIG_RTDEV_PCI)", "") 
	$(ROMFSINST) inic $(ROOT_DIRECTORY)
else
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/wps/wps_inic.asp
endif
endif
endif

ifneq ("$(CONFIG_RT2561_AP)", "")
	$(ROMFSINST) legacy $(ROOT_DIRECTORY)
endif
ifneq ("$(CONFIG_USER_GOAHEAD_LANG_EN)", "y")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/lang/en
endif
ifneq ("$(CONFIG_USER_GOAHEAD_LANG_ZHTW)", "y")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/lang/zhtw
endif
ifneq ("$(CONFIG_USER_GOAHEAD_LANG_ZHCN)", "y")
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/lang/zhcn
endif
	for i in $(DIRS) ; do make -C $$i $@ ; done

# syslog will always be on
#ifeq ("$(CONFIG_LOGREAD)", "")
#	rm $(ROMFSDIR)$(ROOT_DIRECTORY)/adm/syslog.asp
#else
#ifeq ("$(CONFIG_KLOGD)", "")
#	rm $(ROMFSDIR)$(ROOT_DIRECTORY)/adm/syslog.asp
#else
#endif
#endif

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

