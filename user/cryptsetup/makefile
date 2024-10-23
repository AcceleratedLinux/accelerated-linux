VERSION = 2.6.1

URL = https://www.kernel.org/pub/linux/utils/cryptsetup/v$(basename $(VERSION))/cryptsetup-$(VERSION).tar.xz

CONFOPTS = --enable-keyring --enable-cryptsetup
CONFOPTS += --disable-verifysetup
CONFOPTS += --disable-cryptsetup-reencrypt
CONFOPTS += --disable-integritysetup
CONFOPTS += --enable-kernel_crypto --enable-blkid
CONFOPTS += --enable-libargon2 --disable-internal-argon2
CONFOPTS += --enable-luks-adjust-xts-keysize --with-crypto_backend=openssl
CONFOPTS += --enable-static --enable-largefile --disable-static-cryptsetup
CONFOPTS += --with-luks2-lock-path=/var/run/cryptsetup
CONFOPTS += --with-default-luks-format=LUKS2
CONFOPTS += --disable-ssh-token
CONFOPTS += --disable-asciidoc

# Need this, otherwise it complains about missing 'libintl_gettext'
#CONFVARS += LIBS=-lintl

CFLAGS += -D_LARGEFILE64_SOURCE
LIBTOOL = "`pwd`/build/$($(PKG)_BUILDDIR)/libtool"

AUTOMAKE_PRESTAGE = cryptsetup_prestage

include $(ROOTDIR)/tools/automake.inc

# Remove locale and man files
cryptsetup_prestage:
	rm -Rf build/cryptsetup-$(VERSION)-install/share

