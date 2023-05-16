VERSION = 3.6.16
URL = https://www.gnupg.org/ftp/gcrypt/gnutls/v3.6/gnutls-$(VERSION).tar.xz


CONFOPTS += --enable-openssl-compatibility
CONFOPTS += --with-included-libtasn1 --with-included-unistring --without-p11-kit
CONFOPTS += --enable-local-libopts
CONFOPTS += --disable-cxx
CONFOPTS += --disable-guile
CONFOPTS += --disable-libdane

CFLAGS += -I$(shell pwd)/build/$(gnutls-$(VERSION)_BUILDDIR)/src/libopts
LDFLAGS += -L$(shell pwd)/build/$(gnutls-$(VERSION)_BUILDDIR)/src/libopts

LIBTOOL = $(ROOTDIR)/lib/gnutls/build/gnutls-$(VERSION)/libtool

include $(ROOTDIR)/tools/automake.inc
