#
#	Makefile -- Build instructions for McAfee/SG8200
#

ROMFSIMG  = $(IMAGEDIR)/ramdisk
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
IMAGESIZE = 32000000
FLASH     = $(IMAGEDIR)/flash.bin

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS) sda1

DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	boot,c,90,0 \
	tags,c,90,2 \
	log0,c,90,4 \
	log1,c,90,6 \
	kboot,c,90,8 \
	all,c,90,10

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.symlinks romfs.default romfs.rc romfs.version romfs.boot
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) /etc/default/start

image.romfs_clean:
	$(ROMFSINST) -R /share/aclocal
	$(ROMFSINST) -R /share/doc
	$(ROMFSINST) -R /share/gettext
	$(ROMFSINST) -R /share/gtk-doc
	$(ROMFSINST) -R /share/locale
	$(ROMFSINST) -R /share/bash-completion
	$(ROMFSINST) -R /share/gdb
	$(ROMFSINST) -R /share/glib-2.0
	$(ROMFSINST) -R /share/info
	$(ROMFSINST) -R /share/man

image: image.romfs_clean image.configs image.dir image.mips.zimage image.squashfs image.bin image.size image.tag image.copy image.flash

#
# build up a flash.bin
#
vendor_flashbin:
	@$(DD) conv=sync if=$(ROOTDIR)/boot/boot.bin of=$(FLASH) bs=256k 
	@cat $(IMAGE) >> $(FLASH)
	@echo "Factory `basename $(FLASH)` created."
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(FLASH) /tftpboot/.

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
