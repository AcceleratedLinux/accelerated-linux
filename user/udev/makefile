
URL = https://www.kernel.org/pub/linux/utils/kernel/hotplug/udev-182.tar.bz2

CONFOPTS += --disable-introspection
CONFOPTS += --disable-gudev
CONFOPTS += --disable-manpages --disable-gtk-doc-html
CONFOPTS += --enable-fast-install --disable-dependency-tracking
CONFOPTS += --disable-keymap
CONFOPTS += --disable-mtd_probe
CONFOPTS += --with-usb-ids-path=/etc/usb.ids
CONFOPTS += --with-pci-ids-path=/etc/pci.ids
CONFOPTS += --sysconfdir=/etc
# CONFOPTS += --enable-debug

CONFVARS = BLKID_CFLAGS=" " BLKID_LIBS="-lblkid" LIBS=-lrt
MAKEVARS = 

include $(ROOTDIR)/tools/automake.inc

