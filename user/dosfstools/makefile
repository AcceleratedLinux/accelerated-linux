VERSION = 4.2
URL = https://github.com/dosfstools/dosfstools/releases/download/v$(VERSION)/dosfstools-$(VERSION).tar.gz

AUTOMAKE_ROMFS = dosfstools_romfs

include $(ROOTDIR)/tools/automake.inc

dosfstools_romfs:
	$(ROMFSINST) -e CONFIG_USER_DOSFSTOOLS_DOSFSCK build/dosfstools-$(VERSION)/src/fsck.fat /sbin/fsck.fat
	$(ROMFSINST) -e CONFIG_USER_DOSFSTOOLS_MKDOSFS build/dosfstools-$(VERSION)/src/mkfs.fat /sbin/mkfs.fat
	$(ROMFSINST) -e CONFIG_USER_DOSFSTOOLS_DOSFSLABEL build/dosfstools-$(VERSION)/src/fatlabel /sbin/fatlabel
