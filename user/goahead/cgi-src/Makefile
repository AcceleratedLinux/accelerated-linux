#
# Makefile for the GoAhead web server reference source base
#  for the uClinux OS
#
# Copyright (c) GoAhead Software, Inc. 1995-2000
#
# $Id: Makefile,v 1.29 2009-12-14 03:45:48 winfred Exp $
#

#CFLAGS += -Werror

CONF	= $(ROOTDIR)/$(LINUXDIR)
ZLIB_H  = $(ROOTDIR)/lib/zlib
CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h

#
#  As you can see, RRDTool(traffic graphic) is obsoleted.
#
UPLOAD_CGI = upload.cgi 
UPLOAD_BOOTLOADER_CGI = upload_bootloader.cgi
UPLOAD_SETTINGS = upload_settings.cgi
USB_UPGRADE_CGI = usb_upgrade.cgi
UPLOAD_CA_CLIENT_CERT_CGI = upload_caclcert.cgi
UPLOAD_KEY_CERT_CGI = upload_keycert.cgi
UPLOAD_WAPI_AS_CERT_CGI = upload_wapi_as_cert.cgi
UPLOAD_WAPI_USER_CERT_CGI = upload_wapi_user_cert.cgi

#RRDTOOL_CGIWARPPER = rrdtool_cgiwrapper.cgi
#CPU_MEM_RRD_UPDATER	= cpu_mem_updater
#NETWORK_RRD_UPDATER	= network_updater

ALL_EXE = $(UPLOAD_CGI) $(UPLOAD_BOOTLOADER_CGI) $(UPLOAD_SETTINGS) $(RRDTOOL_CGIWARPPER) $(CPU_MEM_RRD_UPDATER) $(NETWORK_RRD_UPDATER)
ifeq ("$(CONFIG_USB_STORAGE)", "y")
ifeq ("$(CONFIG_USER_STORAGE)", "y")
ALL_EXE += $(USB_UPGRADE_CGI)
endif
endif
ifeq ("$(CONFIG_USER_WPA_SUPPLICANT)", "y")
ifeq ("$(CONFIG_RT2860V2_STA_WPA_SUPPLICANT)", "y")
ALL_EXE += $(UPLOAD_CA_CLIENT_CERT_CGI)
ALL_EXE += $(UPLOAD_KEY_CERT_CGI)
endif
endif
ifeq ("$(CONFIG_RALINKAPP_WAPI)", "y")
ifeq ("$(CONFIG_RT2860V2_AP_WAPI)", "y")
ALL_EXE += $(UPLOAD_WAPI_AS_CERT_CGI)
ALL_EXE += $(UPLOAD_WAPI_USER_CERT_CGI)
endif
endif

all:	$(ALL_EXE)

$(UPLOAD_CGI): upload.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o upload.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_BOOTLOADER_CGI): upload_bootloader.cgi.o
	$(CC) $(CFLAGS) upload_bootloader.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_SETTINGS): upload_settings.cgi.o
	$(CC) $(CFLAGS) upload_settings.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(USB_UPGRADE_CGI): usb_upgrade.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o usb_upgrade.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_CA_CLIENT_CERT_CGI): upload_caclcert.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o upload_caclcert.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_KEY_CERT_CGI): upload_keycert.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o upload_keycert.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_WAPI_AS_CERT_CGI): upload_wapi_as_cert.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o upload_wapi_as_cert.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

$(UPLOAD_WAPI_USER_CERT_CGI): upload_wapi_user_cert.cgi.o crc32.o
	$(CC) $(CFLAGS) crc32.o upload_wapi_user_cert.cgi.o -o $@
	$(STRIP) --remove-section=.note --remove-section=.comment $@

#$(RRDTOOL_CGIWARPPER): rrdtool_cgiwrapper.o
#	$(CC) -o $@	rrdtool_cgiwrapper.o
#	$(STRIP) --remove-section=.note --remove-section=.comment $@
#
#$(CPU_MEM_RRD_UPDATER): cpu_mem_updater.o
#	$(CC) -o $@	cpu_mem_updater.o
#	$(STRIP) --remove-section=.note --remove-section=.comment $@
#
#$(NETWORK_RRD_UPDATER): network_updater.o
#	$(CC) -o $@	network_updater.o
#	$(STRIP) --remove-section=.note --remove-section=.comment $@
	
romfs:
	$(ROMFSINST) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_CGI)
	$(ROMFSINST) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_BOOTLOADER_CGI)
	$(ROMFSINST) reboot.sh $(ROOT_DIRECTORY)/cgi-bin/reboot.sh
#	$(ROMFSINST) $(RRDTOOL_CGIWARPPER) $(ROOT_DIRECTORY)/cgi-bin/$(RRDTOOL_CGIWARPPER)
	$(ROMFSINST) ExportSettings.sh $(ROOT_DIRECTORY)/cgi-bin/ExportSettings.sh
	$(ROMFSINST) $(UPLOAD_SETTINGS)  $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_SETTINGS)
	$(ROMFSINST) history.sh  $(ROOT_DIRECTORY)/cgi-bin/history.sh	
	-$(ROMFSINST) $(ROOTDIR)/History  $(ROOT_DIRECTORY)/cgi-bin/History
ifeq ("$(CONFIG_USB_STORAGE)", "y")
ifeq ("$(CONFIG_USER_STORAGE)", "y")
	$(ROMFSINST) $(USB_UPGRADE_CGI) $(ROOT_DIRECTORY)/cgi-bin/$(USB_UPGRADE_CGI)
endif
endif
ifeq ("$(CONFIG_USER_WPA_SUPPLICANT)", "y")
ifeq ("$(CONFIG_RT2860V2_STA_WPA_SUPPLICANT)", "y")
	$(ROMFSINST) $(UPLOAD_CA_CLIENT_CERT_CGI) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_CA_CLIENT_CERT_CGI)
	$(ROMFSINST) $(UPLOAD_KEY_CERT_CGI) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_KEY_CERT_CGI)
endif
endif
ifeq ("$(CONFIG_RALINKAPP_WAPI)", "y")
ifeq ("$(CONFIG_RT2860V2_AP_WAPI)", "y")
	$(ROMFSINST) $(UPLOAD_WAPI_AS_CERT_CGI) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_WAPI_AS_CERT_CGI)
	$(ROMFSINST) $(UPLOAD_WAPI_USER_CERT_CGI) $(ROOT_DIRECTORY)/cgi-bin/$(UPLOAD_WAPI_USER_CERT_CGI)
endif
endif


#they are not cgi but cgi-related.
#	$(ROMFSINST) $(CPU_MEM_RRD_UPDATER) $(ROOT_DIRECTORY)/cgi-bin/$(CPU_MEM_RRD_UPDATER)
#	$(ROMFSINST) $(NETWORK_RRD_UPDATER) $(ROOT_DIRECTORY)/cgi-bin/$(NETWORK_RRD_UPDATER)
	
clean:
	#rm -f $(ALL_EXE) $(ARCH) $(USB_UPGRADE_CGI) $(UPLOAD_CA_CLIENT_CERT_CGI) $(UPLOAD_KEY_CERT_CGI) $(UPLOAD_WAPI_AS_CERT_CGI) $(UPLOAD_WAPI_USER_CERT_CGI) *.o
	rm -f $(ALL_EXE) $(ARCH) *.o

#
#	Dependencies
#
upload.cgi.o: upload.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_FIRMWARE_SUPPORT -I$(CONF) -c upload.cgi.c -o $@

crc32.o: crc32.c $(CONF_H)
	$(CC) $(CFLAGS) -DUSE_HOSTCC -I$(CONF) -I$(ZLIB_H) -c crc32.c -o $@

upload_bootloader.cgi.o: upload.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_BOOTLOADER_SUPPORT -I$(CONF) -c upload.cgi.c -o $@

upload_settings.cgi.o: upload_settings.cgi.c
	$(CC) $(CFLAGS) -c upload_settings.cgi.c -o $@

usb_upgrade.cgi.o: usb_upgrade.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -I$(CONF) -c usb_upgrade.cgi.c -o $@

upload_caclcert.cgi.o: upload_cert.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_CACL_CERT_SUPPORT -I$(CONF) -c upload_cert.cgi.c -o $@

upload_keycert.cgi.o: upload_cert.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_KEY_CERT_SUPPORT -I$(CONF) -c upload_cert.cgi.c -o $@

upload_wapi_as_cert.cgi.o: upload_cert.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_WAPI_AS_CERT_SUPPORT -I$(CONF) -c upload_cert.cgi.c -o $@

upload_wapi_user_cert.cgi.o: upload_cert.cgi.c $(CONF_H)
	$(CC) $(CFLAGS) -DUPLOAD_WAPI_USER_CERT_SUPPORT -I$(CONF) -c upload_cert.cgi.c -o $@

#rrdtool_cgiwrapper.o: rrdtool_cgiwrapper.c
#	$(CC) -c rrdtool_cgiwrapper.c -o $@
#
#cpu_mem_updater.o: cpu_mem_updater.c
#	$(CC) -c cpu_mem_updater.c -o $@
#
#network_updater.o: network_updater.c
#	$(CC) -c network_updater.c -o $@

