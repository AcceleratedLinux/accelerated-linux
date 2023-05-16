#
# Common makefile for all TX64 variants
#

COMMON_DIR = $(ROOTDIR)/vendors/Digi/TX64

ROMFSIMG    = $(IMAGEDIR)/rootfs
ZIMAGE      = $(IMAGEDIR)/vmlinuz
IMAGE       = $(IMAGEDIR)/image.bin
IMAGESIZE 	= 1073741824 # 1GB

SIGNED_GRUB ?= $(COMMON_DIR)/bootx64.efi.signed
BLOADER_IMG = $(SIGNED_GRUB)
BLOADER_IMG_VERSION = $(shell strings $(BLOADER_IMG) 2> /dev/null | grep -oP 'set GRUB_VERSION="\K[^"]+')

SIGNING_ALG = hmac
SIGNING_KEY = $(ROOTDIR)/prop/sign_image/devkeys/tx64/developer_key


VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub efi etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,64 \
	ttyBLE,c,4,65 \
	ttyGPS,c,4,67

FLASH_DEVICES = \
	efi,b,8,1 \
	env1,b,8,2 \
	env2,b,8,3 \
	image,b,8,5 \
	image1,b,8,7 \
	opt,b,8,8

all:

clean: image.clean

romfs.common: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) $(COMMON_DIR)/start /etc/default/start
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	$(ROMFSINST) -p 555 $(COMMON_DIR)/clear-bootcount.sh /bin/clear-bootcount.sh
	$(ROMFSINST) -p 555 $(COMMON_DIR)/mkffs /etc/mkffs
	$(ROMFSINST) -p 555 $(COMMON_DIR)/update-grub /bin/update-grub
	$(ROMFSINST) -p 555 $(COMMON_DIR)/dmi2fwenv /etc/dmi2fwenv
	$(ROMFSINST) -d -p 755 $(COMMON_DIR)/pwrbtn.sh /etc/acpi/events/PWRF/00000080
	# Create an mtab file (symlink to /proc/self/mounts) for e2fsprogs
	$(ROMFSINST) -s /proc/self/mounts /etc/mtab

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	rm -rf $(ROMFSDIR)/boot/grub/x86_64-efi
	# If we have a signed image then install it and lose the grub
	# installation, otherwise setup for building a signed image
	if [ -f "$(SIGNED_GRUB)" ]; then \
		rm -rf $(ROMFSDIR)/boot/grub ; \
		rm -f $(ROMFSDIR)/usr/bin/grub-mkstandalone; \
		rm -f $(ROMFSDIR)/usr/sbin/grub-install; \
		$(ROMFSINST) -S -d $(SIGNED_GRUB) /boot/grub/$(notdir $(SIGNED_GRUB)); \
	else \
		mv $(ROMFSDIR)/usr/lib/grub/x86_64-efi $(ROMFSDIR)/boot/grub/; \
	fi

	# Remove no longer necessary GRUB things
	rm -rf $(ROMFSDIR)/usr/lib/grub

	rm -rf $(ROMFSDIR)/usr/lib/python3.4/test
	rm -rf $(ROMFSDIR)/usr/share/doc
	rm -rf $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.sign-atmel image.tag image.copy image.size
	# Create manufacturing image
	. $(COMMON_DIR)/mkdisk

# Comment it out to stop generating the bootloader update file
ifneq ($(shell [ -f "$(SIGNED_GRUB)" ] && echo OK),)
image: bloader.pack
endif

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
