VERSION = 2.92
URL = https://github.com/transmission/transmission-releases/raw/master/transmission-$(VERSION).tar.xz

CONFOPTS = --datarootdir=/usr/share --mandir=/share

ZLIB_CFLAGS    = -I/include
ZLIB_LIBS      = -lz
OPENSSL_CFLAGS = -I/include
OPENSSL_LIBS   = -lssl
LIBS           = -lcrypto

ifdef CONFIG_USER_TRANSMISSION_DAEMON
	CONFOPTS += --enable-daemon
else
	CONFOPTS += --disable-daemon
endif

ifdef CONFIG_USER_TRANSMISSION_CLI
	CONFOPTS += --enable-cli
else
	CONFOPTS += --disable-cli
endif

ifdef CONFIG_USER_TRANSMISSION_LIGHTWEIGHT
	CONFOPTS += --enable-lightweight
else
	CONFOPTS += --disable-lightweight
endif

include $(ROOTDIR)/tools/automake.inc
