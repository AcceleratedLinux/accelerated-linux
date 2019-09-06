URL = https://github.com/landley/toybox.git
METHOD = git
VERSION = 0.8.1

CFLAGS = -Wall
LDFLAGS += -lcrypt

AUTORECONF = make defconfig

toybox_INSTALLVARS = PREFIX=$(ROOTDIR)/user/toybox/build/toybox-install
INSTALL = install

include $(ROOTDIR)/tools/automake.inc
