VERSION = 2.1.26
URL = ftp://ftp.cyrusimap.org/cyrus-sasl/cyrus-sasl-$(VERSION).tar.gz

LIBTOOL = "`pwd`/build/$($(PKG)_BUILDDIR)/libtool"

CONFOPTS += --without-saslauthd --with-plugindir=/lib/sasl2 --disable-spnego
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --with-configdir=/etc/config
endif

include $(ROOTDIR)/tools/automake.inc
