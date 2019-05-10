
URL       = git://anongit.freedesktop.org/libmbim/libmbim
VERSION   = 61b3f09e417e4f6131a763094960a7d45bb4fc56
METHOD    = git
CONFIGURE = autogen.sh

CONFOPTS   = --disable-gtk-doc-html
CONFOPTS   += --enable-more-warnings=no
ifdef CONFIG_LIB_GLIB_STATIC
CONFVARS   = LIBS="-lgmodule-2.0 -lglib-2.0 -lffi -lz -lpthread -ldl"
CONFOPTS   += --enable-static --disable-shared
endif
#CONFOPTS  += --with-tests
MAKEVARS   = 
#LIBTOOL="sh -x $(shell pwd)/build/libmbim/libtool"

ifeq ($(shell which gdbus-codegen),)
# cheat a little on older systems to get the tools we need to build
$(shell mkdir -p build/bin; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.35.3-install/bin/gdbus-codegen build/bin/.; ln -sf $(ROOTDIR)/lib/glib/build/glib-2.35.3-install/bin/glib-mkenums build/bin/.)
MAKEVARS += PATH=$(shell pwd)/build/bin:$$PATH UNINSTALLED_GLIB_SRCDIR=$(ROOTDIR)/lib/glib/build/glib-2.35.3
endif

libmbim_ROMFS = libmbim_romfs

include $(ROOTDIR)/tools/automake.inc

libmbim_romfs:

	
