#
# Makefile for the GoAhead web server reference source base
#  for the uClinux OS
#
# Copyright (c) GoAhead Software, Inc. 1995-2000
#
# $Id: Makefile,v 1.65 2009-12-21 07:46:19 chhung Exp $
#

NAME	= goahead

#CFLAGS += -Werror

# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

# Digest Access switch
# DASW	= -DDIGEST_ACCESS_SUPPORT

# SSL switches
ifeq ("$(CONFIG_USER_GOAHEAD_SSL)", "y")
SSLPATCHFILE = matrix_ssl.o sslSocket.o
MATRIXDIR = $(ROOTDIR)/user/matrixssl-1.8.3
SSLINC = $(MATRIXDIR)
SSLLIB = $(MATRIXDIR)/src/libmatrixsslstatic.a
SSLSW = -DWEBS_SSL_SUPPORT -DMATRIX_SSL -I$(SSLINC)
endif

# If-Modified-Support switches (requires math library, libm.a)
# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# IFMODLIB = /usr/lib/libm.a

# Dependencies
DEPEND_FILES	= asp.o balloc.o base64.o cgi.o default.o  \
				  ejlex.o ejparse.o form.o \
				  h.o handler.o mime.o misc.o page.o \
				  ringq.o rom.o \
				  sock.o sockGen.o $(SSLPATCHFILE) \
				  security.o sym.o uemf.o url.o value.o \
				  md5c.o um.o umui.o websda.o emfdb.o \
				  webrom.o webs.o websuemf.o \
				  internet.o utils.o wireless.o \
				  firewall.o management.o wps.o $(NAME).o
ifneq ("$(CONFIG_RT2860V2_STA)", "")
DEPEND_FILES += station.o
endif
ifneq ("$(CONFIG_RTDEV_MII)", "") 
DEPEND_FILES += inic.o wps_inic.o
CFLAGS += -DRTDEV_SUPPORT
else
ifneq ("$(CONFIG_RTDEV_USB)", "")
DEPEND_FILES += inic.o wps_inic.o
CFLAGS += -DRTDEV_SUPPORT
else
ifneq ("$(CONFIG_RTDEV_PCI)", "")
DEPEND_FILES += inic.o wps_inic.o
CFLAGS += -DRTDEV_SUPPORT
endif
endif
endif
ifneq ("$(CONFIG_RT2561_AP)", "")
DEPEND_FILES += legacy.o
endif
ifeq ("$(CONFIG_USER_GOAHEAD_IPV6)", "y")
CFLAGS += -DWF_USE_IPV6
endif
ifeq ("$(CONFIG_USER_GOAHEAD_HOSTNAME)", "y")
CFLAGS += -DGA_HOSTNAME_SUPPORT
endif
ifeq ("$(CONFIG_USER_WPA_SUPPLICANT)", "y")
ifeq ("$(CONFIG_RT2860V2_STA_WPA_SUPPLICANT)", "y")
CFLAGS += -DWPA_SUPPLICANT_SUPPORT
endif
endif
ifneq ("$(CONFIG_USB)", "")
DEPEND_FILES += usb.o
endif
ifeq ("$(CONFIG_RALINKAPP_MPLAYER)", "y")
DEPEND_FILES += media.o
endif
ifeq ("$(CONFIG_RALINKAPP_SWQOS)", "y")
DEPEND_FILES += qos.o
endif
ifeq ("$(CONFIG_RALINKAPP_WAPI)", "y")
ifeq ("$(CONFIG_RT2860V2_AP_WAPI)", "y")
CFLAGS += -DAP_WAPI_SUPPORT
endif
ifeq ("$(CONFIG_RT2860V2_STA_WAPI)", "y")
CFLAGS += -DSTA_WAPI_SUPPORT
endif
endif

CFLAGS	+= -DWEBS -DUEMF -DOS="LINUX" -DLINUX $(UMSW) $(DASW) $(SSLSW) $(IFMODSW)
CFLAGS  += -Wall -fno-strict-aliasing
#CFLAGS	+= -I$(ROOTDIR)/lib/libnvram -I$(ROOTDIR)/$(LINUXDIR)/drivers/char -I$(ROOTDIR)/$(LINUXDIR)/include
#CFLAGS  += -I$(ROOTDIR)/$(LINUXDIR)/drivers/flash 
OTHERS	= -DB_STATS -DB_FILL -DDEBUG
LDFLAGS	+= $(SSLLIB) $(IFMODLIB)
LDLIBS	+= -lnvram

#CONF_H	= $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h
#UCONF_H	= $(ROOTDIR)/config/autoconf.h
#BUSYCONF_H = $(ROOTDIR)/user/busybox/include/autoconf.h

all:	$(NAME)

#
#	Primary link
#
$(NAME): clean_inet $(DEPEND_FILES)
	$(CC) -o $@ $(DEPEND_FILES) $(LDFLAGS) $(EXTRALIBS) $(LDLIBS)
	$(STRIP) --remove-section=.note --remove-section=.comment $@

romfs:
	$(ROMFSINST) /bin/$(NAME)
ifeq ("$(CONFIG_USER_GOAHEAD_SSL)", "y")
	$(ROMFSINST) /etc_ro/serverkey.pem
	$(ROMFSINST) /etc_ro/servercert.pem
endif

clean:
	rm -f $(NAME) *.o

clean_inet:
	rm -f internet.o

#
#	Dependencies
#
asp.o:  webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

balloc.o: balloc.c uemf.h

base64.o:  base64.c webs.h wsIntrn.h  ej.h ejIntrn.h uemf.h

cgi.o:  webs.h wsIntrn.h uemf.h

default.o:  default.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h $(CONF_H)

ejlex.o:  ejlex.c ej.h ejIntrn.h uemf.h

ejparse.o:  ejparse.c ej.h ejIntrn.h uemf.h

emfdb.o:  emfdb.h wsIntrn.h uemf.h

firewall.o: firewall.c webs.h firewall.h

form.o:  form.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

goahead.o: goahead.c uemf.h wsIntrn.h internet.h utils.h wireless.h firewall.h management.h qos.h $(CONF_H) $(UCONF_H) $(BUSYBOXCONF_H)

h.o:  h.c uemf.h

handler.o:  handler.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

inic.o: inic.c inic.h internet.h utils.h webs.h 

legacy.o: legacy.c legacy.h internet.h utils.h webs.h 

internet.o: internet.c internet.h utils.h webs.h  $(CONF_H) $(UCONF_H) $(BUSYBOXCONF_H)

management.o: management.c management.h webs.h $(CONF_H) $(UCONF_H) 

matrix_ssl.o: matrix_ssl.c wsIntrn.h webs.h websSSL.h sslSocket.h

md5c.o:  md5.h wsIntrn.h uemf.h

media.o: media.c media.h webs.h

mime.o:  mime.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

misc.o:  misc.c uemf.h

page.o:  page.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

ringq.o:  ringq.c uemf.h

rom.o:  rom.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

security.o:  security.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h $(CONF_H)

sock.o:  sock.c uemf.h

sockGen.o:  sockGen.c uemf.h $(CONF_H)

sslSocket.o: sslSocket.c sslSocket.h

station.o: station.c station.h oid.h stapriv.h webs.h $(CONF_H) $(UCONF_H)

usb.o: usb.c usb.h webs.h $(UCONF_H)

sym.o:  sym.c uemf.h

uemf.o:  uemf.c uemf.h

um.o:  webs.h wsIntrn.h um.h uemf.h

umui.o:  webs.h wsIntrn.h um.h uemf.h

url.o:  url.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

utils.o: utils.c utils.h webs.h $(CONF_H) $(UCONF_H)

value.o:  value.c uemf.h

webrom.o:  webrom.c webs.h wsIntrn.h uemf.h

webs.o:  webs.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h $(CONF_H)

websda.o:  webs.h wsIntrn.h websda.h uemf.h

websuemf.o:  websuemf.c webs.h wsIntrn.h ej.h ejIntrn.h uemf.h

websSSL.o:  websSSL.c websSSL.h wsIntrn.h ej.h ejIntrn.h uemf.h

wireless.o: wireless.c wireless.h internet.h utils.h webs.h $(CONF_H) $(UCONF_H)

wps.o: wps.c wps.h utils.h webs.h internet.h wireless.h station.h oid.h $(CONF_H)

wps_inic.o: wps_inic.c wps.h utils.h webs.h internet.h oid.h $(CONF_H)

qos.o: qos.h utils.h $(CONF_H)
