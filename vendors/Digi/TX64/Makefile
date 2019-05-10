#
# Makefile -- Build instructions for Digi/TX64
#

ROMFSIMG    = $(IMAGEDIR)/rootfs
ZIMAGE      = $(IMAGEDIR)/vmlinuz
IMAGE       = $(IMAGEDIR)/image.bin
BLIMG_BUILT = $(IMAGEDIR)/bootx64_nonsigned.efi
BLIMG       = $(IMAGEDIR)/bootx64.efi

SIGNING_ALG = hmac
# Add '-e', because the kernel is embedded in the rootfs
SIGNING_EXTRA = -e
# use our dev key by default
SIGNING_KEY = $(ROOTDIR)/prop/sign_image/devkeys/tx64/developer_key


VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub efi etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,64

FLASH_DEVICES = \
	efi,b,8,1 \
	env1,b,8,2 \
	env2,b,8,3 \
	image,b,8,5 \
	image1,b,8,7 \
	opt,b,8,8

GRUB_STANDALONE_MODULES = \
	terminal serial \
	part_gpt part_msdos fat squash4 \
	verify_part gcry_sha256 \
	loadenv echo normal linux reboot sleep test \
	i2c i2c_dw atecc \
	mcu \
	bootcntr watchdog \

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
	$(ROMFSINST) /boot/grub/grub.cfg
	$(ROMFSINST) -p 555 /bin/clear-bootcount.sh
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /etc/dmi2fwenv
	$(ROMFSINST) -d -p 755 pwrbtn.sh /etc/acpi/events/PWRF/00000080

romfs.post:: romfs.cleanup
	# Add version tag to the GRUB
	sed -i -e 's/<GRUB_VERSION>/$(VERSIONPKG)/g' $(ROMFSDIR)/boot/grub/grub.cfg
	# Create a standalone GRUB image
	grub-mkstandalone \
		--directory "$(ROMFSDIR)/usr/lib/grub/x86_64-efi" \
		--format x86_64-efi \
		--modules "$(GRUB_STANDALONE_MODULES)" \
		--install-modules "$(GRUB_STANDALONE_MODULES)" \
		--output "$(ROMFSDIR)/boot/grub/bootx64.efi" \
		"boot/grub/grub.cfg=$(ROMFSDIR)/boot/grub/grub.cfg"
	# Copy non-signed GRUB image into the image directory
	cp "$(ROMFSDIR)/boot/grub/bootx64.efi" "$(BLIMG_BUILT)"
	# Swap it with the pre-built, signed image
	cp bootx64.efi.signed "$(ROMFSDIR)/boot/grub/bootx64.efi"
	# Remove no longer necessary GRUB things
	rm -rf $(ROMFSDIR)/usr/lib/grub
	rm -rf $(ROMFSDIR)/boot/grub/grub.cfg

	rm -rf $(ROMFSDIR)/usr/lib/python3.4/test
	rm -rf $(ROMFSDIR)/usr/share/doc
	rm -rf $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.sign-atmel image.tag image.copy
	# Copy signed GRUB image into the image directory
	cp bootx64.efi.signed "$(BLIMG)"
	# Create manufacturing image
	./mkdisk

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
