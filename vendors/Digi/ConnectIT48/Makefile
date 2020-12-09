#
# Makefile -- Build instructions for Digi/ConnectIT48
#

ROMFSIMG  = $(IMAGEDIR)/rootfs
ROMFSIMGZ = $(IMAGEDIR)/rootfs.gz
ZIMAGE    = $(IMAGEDIR)/vmlinuz
IMAGE     = $(IMAGEDIR)/image.bin

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
	serial/port16,c,4,85 \
	serial/port17,c,4,86 \
	serial/port18,c,4,87 \
	serial/port19,c,4,88 \
	serial/port20,c,4,89 \
	serial/port21,c,4,90 \
	serial/port22,c,4,91 \
	serial/port23,c,4,92 \
	serial/port24,c,4,93 \
	serial/port25,c,4,94 \
	serial/port26,c,4,95 \
	serial/port27,c,4,96 \
	serial/port28,c,4,97 \
	serial/port29,c,4,98 \
	serial/port30,c,4,99 \
	serial/port31,c,4,100 \
	serial/port32,c,4,101 \
	serial/port33,c,4,102 \
	serial/port34,c,4,103 \
	serial/port35,c,4,104 \
	serial/port36,c,4,105 \
	serial/port37,c,4,106 \
	serial/port38,c,4,107 \
	serial/port39,c,4,108 \
	serial/port40,c,4,109 \
	serial/port41,c,4,110 \
	serial/port42,c,4,111 \
	serial/port43,c,4,112 \
	serial/port44,c,4,113 \
	serial/port45,c,4,114 \
	serial/port46,c,4,115 \
	serial/port47,c,4,116 \
	serial/port48,c,4,117

FLASH_DEVICES = \
	efi,b,8,17 \
	image,b,8,18 \
	image1,b,8,19 \
	opt,b,8,20

all:

clean: image.clean
	rm -f grub.cfg

romfs: grub.cfg romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
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
	$(ROMFSINST) -d -p 755 pwrbtn.sh /etc/acpi/events/PWRF/00000080
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	echo "Digi Connect IT 48" > $(ROMFSDIR)/etc/issue

grub.cfg: grub.cfg.in Makefile
	sed 's/BOOT_VER[ 	]*=.*$$/BOOT_VER="$(VERSIONPKG)"/g' < $< > $@

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	rm -rf $(ROMFSDIR)/boot/grub/x86_64-efi
	# if we have a signed image then install it and lose the
	# grub installation,  otherwise setup for building a signed image
	if [ -f "$(SIGNED_GRUB)" ]; then \
		rm -rf $(ROMFSDIR)/boot/grub ; \
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

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.sign-atmel image.tag image.copy
	gzip -9 -c -f $(ROMFSIMG) > $(ROMFSIMGZ)
	./mkdisk

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak

