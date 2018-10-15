VERSION = lxc-3.0.2
URL = https://github.com/lxc/lxc.git
METHOD = git

AUTORECONF = ./autogen.sh

CONFOPTS := --datarootdir=/etc \
	--localstatedir=/opt \
	--runstatedir=/var/run

CONFOPTS += --disable-doc \
	--disable-api-docs \
	--disable-examples \
	--disable-bash

include $(ROOTDIR)/tools/automake.inc
