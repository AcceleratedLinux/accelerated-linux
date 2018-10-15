
VERSION = 18
URL = https://www.kernel.org/pub/linux/utils/kernel/kmod/kmod-$(VERSION).tar.gz

CONFOPTS += --disable-dependency-tracking
CONFOPTS += --enable-fast-install
CONFOPTS += --disable-manpages
CONFOPTS += --disable-gtk-doc
CONFOPTS += --disable-gtk-doc-html
CONFOPTS += --disable-gtk-doc-pdf
# CONFOPTS += --enable-debug

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_KMOD_LIBKMOD \
		build/kmod-$(VERSION)-install/lib/libkmod.so /lib/libkmod.so
	$(ROMFSINST) -e CONFIG_USER_KMOD_TOOLS \
		build/kmod-$(VERSION)-install/bin/kmod /bin/kmod
	for i in modprobe modinfo lsmod insmod depmod rmmod ; do \
		$(ROMFSINST) -e CONFIG_USER_KMOD_TOOLS -s /bin/kmod /sbin/$$i || exit 1; \
	done

