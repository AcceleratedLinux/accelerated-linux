#
# Makefile -- Build instructions for Digi/TX64
#

ROMFSIMG    = $(IMAGEDIR)/rootfs
ZIMAGE      = $(IMAGEDIR)/vmlinuz
IMAGE       = $(IMAGEDIR)/image.bin

SIGNED_GRUB = bootx64.efi.signed
BLOADER_IMG = $(SIGNED_GRUB)
BLOADER_IMG_VERSION = $(shell strings $(BLOADER_IMG) | grep -oP 'set GRUB_VERSION="\K[^"]+')

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

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 sim /bin/sim1
	$(ROMFSINST) -p 555 -s /bin/sim1 /bin/sim2
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	echo "Digi TX64" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	$(ROMFSINST) -p 555 /bin/clear-bootcount.sh
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /bin/update-grub
	$(ROMFSINST) -p 555 /etc/dmi2fwenv
	$(ROMFSINST) -d -p 755 pwrbtn.sh /etc/acpi/events/PWRF/00000080
	# Add version tag to the GRUB
	sed -e 's/<GRUB_VERSION>/$(VERSIONPKG)/g' <grub.cfg.in >$(ROMFSDIR)/boot/grub/grub.cfg
	# Create an mtab file (symlink to /proc/self/mounts) for e2fsprogs
	$(ROMFSINST) -s /proc/self/mounts /etc/mtab

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	rm -rf $(ROMFSDIR)/boot/grub/x86_64-efi
	# If we have a signed image then install it and lose the grub
	# installation, otherwise setup for building a signed image
	if [ -f "$(SIGNED_GRUB)" ]; then \
		rm -rf $(ROMFSDIR)/boot/grub ; \
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

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.sign-atmel image.tag image.copy
	# Create manufacturing image
	./mkdisk

# Comment it out to stop generating the bootloader update file
image: bloader.pack

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
