#
# Makefile -- Build instructions for AcceleratedConcepts/9400-UA
#

ROMFSIMG  = $(IMAGEDIR)/rootfs
ROMFSIMGZ = $(IMAGEDIR)/rootfs.gz
ZIMAGE    = $(IMAGEDIR)/vmlinuz
IMAGE     = $(IMAGEDIR)/image.bin
ISOFILE   = $(IMAGEDIR)/$(CONFIG_PRODUCT).iso
ISODIR    = $(IMAGEDIR)/iso

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \

FLASH_DEVICES = \
	image,b,8,1 \
	image1,b,8,2 \
	opt,b,8,3 \
	boot,c,90,4 \
	bootall,c,90,0 \
	bootenv,c,90,2

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -s /var/run/fw_env.config /etc/fw_env.config
	$(ROMFSINST) fw_env.config /etc/fw_env_hw.config
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) /boot/grub/grub.cfg
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) -p 555 /bin/setup-new-device.sh
	$(ROMFSINST) -p 555 /etc/grub-install
	$(ROMFSINST) -p 555 /etc/switch.sh
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	echo "Accelerated Concepts 9400-UA" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	rm -rf $(ROMFSDIR)/boot/grub/i386-pc
	mv $(ROMFSDIR)/usr/lib/grub/i386-pc $(ROMFSDIR)/boot/grub/
	rmdir $(ROMFSDIR)/usr/lib/grub
	rm -rf $(ROMFSDIR)/usr/lib/python3.4/test
	rm -rf $(ROMFSDIR)/usr/share/doc
	rm -rf $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.tag image.copy
	gzip -9 -c -f $(ROMFSIMG) > $(ROMFSIMGZ)
	mkdir -p $(ISODIR)/boot/isolinux
	cp isolinux.bin $(ISODIR)/boot/isolinux/
	cp ldlinux.c32 $(ISODIR)/boot/isolinux/
	cp boot.msg $(ISODIR)/boot/isolinux/
	cp isolinux.cfg $(ISODIR)/boot/isolinux/
	cp $(ZIMAGE) $(ISODIR)/boot/isolinux/
	cp $(ROMFSIMGZ) $(ISODIR)/boot/isolinux/initrd.gz
	mkisofs -o $(ISOFILE) -b boot/isolinux/isolinux.bin \
		-c boot/isolinux/boot.cat -no-emul-boot \
		-boot-load-size 4 -boot-info-table $(ISODIR)
	isohybrid $(ISOFILE)

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
