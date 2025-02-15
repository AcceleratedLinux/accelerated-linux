#
# Makefile -- Build instructions for Digi/ConnectIT16
#

ROMFSIMG  = $(IMAGEDIR)/rootfs
ROMFSIMGZ = $(IMAGEDIR)/rootfs.gz
ZIMAGE    = $(IMAGEDIR)/vmlinuz
IMAGE     = $(IMAGEDIR)/image.bin
IMAGESIZE = 1073741824 # 1GB

SIGNING_ALG   = ecdsa

SIGNED_GRUB   = bootx64.efi.signed

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub efi etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/console,c,4,64 \
	serial/port1,c,4,70 \
	serial/port2,c,4,71 \
	serial/port3,c,4,72 \
	serial/port4,c,4,73 \
	serial/port5,c,4,74 \
	serial/port6,c,4,75 \
	serial/port7,c,4,76 \
	serial/port8,c,4,77 \
	serial/port9,c,4,78 \
	serial/port10,c,4,79 \
	serial/port11,c,4,80 \
	serial/port12,c,4,81 \
	serial/port13,c,4,82 \
	serial/port14,c,4,83 \
	serial/port15,c,4,84 \
	serial/port16,c,4,85

FLASH_DEVICES = \
	efi,b,8,17 \
	image,b,8,18 \
	image1,b,8,19 \
	opt,b,8,20

all:

clean: image.clean
	rm -f grub.cfg
	rm -f atecc508.aml

atecc508.aml: atecc508.asl
	@echo "Generating ATECC508 SSDT overlay..."
	/usr/bin/iasl -p $@ $<

romfs: grub.cfg romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey atecc508.aml
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /bin/update-grub
	$(ROMFSINST) -p 555 /etc/dmi2fwenv
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) -p 555 /bin/setup-new-device.sh
	$(ROMFSINST) -p 555 /bin/clear-bootcount.sh
	$(ROMFSINST) /boot/grub/grub.cfg
	$(ROMFSINST) -d -p 755 pwrbtn.sh /etc/acpi/PWRF/00000080
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-sdcard_poll_rate.rules
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-sdcard_automount.rules
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-usb1_automount.rules
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-usb2_automount.rules
	echo "Digi Connect IT 16" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -d /etc/ssdt/atecc508.aml

grub.cfg: grub.cfg.in Makefile
	sed 's/BOOT_VER[ 	]*=.*$$/BOOT_VER="$(VERSIONPKG)"/g' < $< > $@

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	rm -rf $(ROMFSDIR)/boot/grub/x86_64-efi
	# if we have a signed image then install it and lose the
	# grub installation,  otherwise setup for building a signed image
	if [ -f "$(SIGNED_GRUB)" ]; then \
		rm -rf $(ROMFSDIR)/boot/grub ; \
		rm -f $(ROMFSDIR)/usr/bin/grub-mkstandalone; \
		rm -f $(ROMFSDIR)/usr/sbin/grub-install; \
		$(ROMFSINST) -S -d /boot/grub/$(SIGNED_GRUB); \
	else \
		mv $(ROMFSDIR)/usr/lib/grub/x86_64-efi $(ROMFSDIR)/boot/grub/;\
	fi
	$(ROMFSINST) -R $(ROMFSDIR)/usr/lib/grub
	$(ROMFSINST) -R $(ROMFSDIR)/usr/lib/python3.4/test
	$(ROMFSINST) -R $(ROMFSDIR)/usr/share/doc
	$(ROMFSINST) -R $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale
	mkdir -p $(ROMFSDIR)/usr/share/locale

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.sign-atmel image.tag image.copy image.size
	gzip -9 -c -f $(ROMFSIMG) > $(ROMFSIMGZ)
	./mkdisk

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak

