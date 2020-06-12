VERSION = 6.9
URL = https://github.com/nhorman/rng-tools/archive/v$(VERSION).tar.gz

BUILDDIR = rng-tools-$(VERSION)
AUTORECONF = autoreconf -i
AUTOMAKE_ROMFS = my_romfs

CONFOPTS = --without-nistbeacon --without-pkcs11 --program-prefix=
ifndef CONFIG_LIB_LIBGCRYPT
CONFOPTS += --without-libgcrypt
endif

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/v$(VERSION)-install/sbin/rngd /sbin/rngd
