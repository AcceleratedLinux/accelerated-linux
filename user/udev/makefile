VERSION = 3.2.11

URL = https://github.com/eudev-project/eudev/archive/refs/tags/v$(VERSION).tar.gz

BUILDDIR = eudev-$(VERSION)

CONFOPTS += --enable-fast-install
CONFOPTS += --disable-introspection
CONFOPTS += --disable-manpages
CONFOPTS += --disable-gtk-doc-html
CONFOPTS += --disable-dependency-tracking
CONFOPTS += --disable-selinux 
CONFOPTS += --disable-hwdb
CONFOPTS += --prefix=/
CONFOPTS += --sysconfdir=/etc
# these set to be compatible with original udev-182
CONFOPTS += --sbindir=/libexec/udev
CONFOPTS += --with-rootlibexecdir=/libexec/udev
CONFOPTS += --with-firmware-path=/lib/firmware

CONFVARS = BLKID_CFLAGS=" "
CONFVARS += BLKID_LIBS="-lblkid"
CONFVARS += LIBS=-lrt

MAKEVARS =

AUTORECONF = autoreconf -f -i -s

v$(VERSION)_ROMFS = sanitize_rules

include $(ROOTDIR)/tools/automake.inc

sanitize_rules:
	# Adjust rule set as required.
	# We don't want to use persistent network names.
	rm -f $(ROMFSDIR)/libexec/udev/rules.d/80-net-name-slot.rules

