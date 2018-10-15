#
#	Accelecon X86_64
#

ZIMAGE    = $(IMAGEDIR)/bzImage
ROMFSIMG  = $(IMAGEDIR)/initrd
ROMFSIMGZ = $(IMAGEDIR)/initrd.gz

ISOFILE   = $(IMAGEDIR)/$(CONFIG_PRODUCT).iso
ISODIR    = $(ROOTDIR)/iso

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)

# FLASH_DEVICES =

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.version
	mkdir -p $(ROMFSDIR)/tmp $(ROMFSDIR)/mnt
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) -p 755 /bin/ledcmd
	$(ROMFSINST) -p 755 /etc/default/start
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	# echo "ttyS0:unknown:/sbin/getty 115200 -" >> $(ROMFSDIR)/etc/inittab
	# echo "tty1:unknown:/sbin/getty 9600 -" >> $(ROMFSDIR)/etc/inittab
	# echo "tty2:unknown:/sbin/getty 9600 -" >> $(ROMFSDIR)/etc/inittab

romfs.post:: romfs.cleanup

image: image.configs image.dir image.x86.zimage image.squashfs
	gzip -9 -f $(ROMFSIMG)
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
include ../vendor.mak
