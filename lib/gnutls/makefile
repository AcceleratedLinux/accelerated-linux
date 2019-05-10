VERSION = 3.5.19
URL = https://www.gnupg.org/ftp/gcrypt/gnutls/v3.5/gnutls-$(VERSION).tar.xz


CONFOPTS += --enable-openssl-compatibility
CONFOPTS += --with-included-libtasn1 --with-included-unistring --without-p11-kit
CONFOPTS += --enable-local-libopts
CONFOPTS += --disable-cxx

CFLAGS += -I$(shell pwd)/build/$(gnutls-$(VERSION)_BUILDDIR)/src/libopts
LDFLAGS += -L$(shell pwd)/build/$(gnutls-$(VERSION)_BUILDDIR)/src/libopts

include $(ROOTDIR)/tools/automake.inc
