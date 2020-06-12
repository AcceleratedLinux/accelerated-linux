VERSION = 2.1.27
URL = https://github.com/cyrusimap/cyrus-sasl/releases/download/cyrus-sasl-$(VERSION)/cyrus-sasl-$(VERSION).tar.gz

LIBTOOL = "`pwd`/build/$($(PKG)_BUILDDIR)/libtool"

CONFOPTS += --without-saslauthd
CONFOPTS += --with-plugindir=/lib/sasl2
CONFOPTS += --disable-spnego

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --with-configdir=/etc/config
endif

include $(ROOTDIR)/tools/automake.inc
