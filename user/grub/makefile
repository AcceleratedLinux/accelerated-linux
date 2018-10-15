GRUB_VERSION = 2.00
URL = ftp://ftp.gnu.org/gnu/grub/grub-$(GRUB_VERSION).tar.gz

CONFOPTS += --exec-prefix="/usr"
CONFOPTS += --datarootdir="/usr/share"
CONFOPTS += --program-prefix=""
CONFOPTS += --disable-werror 
CONFOPTS += --with-libintl-prefix=/foo

grub-$(GRUB_VERSION)_ROMFS = grub_romfs

include $(ROOTDIR)/tools/automake.inc

#
# We need to make sure that the grub modules are not stripped.
# The normal automake install through romfs-inst will strip them.
#
grub_romfs:
	cd build/grub-$(GRUB_VERSION)-install/usr/lib/grub/ ; \
	ARCH=`ls` ; \
	cd $$ARCH ; \
	for i in * ; do \
		$(ROMFSINST) -S /usr/lib/grub/$$ARCH/$$i ; \
	done

