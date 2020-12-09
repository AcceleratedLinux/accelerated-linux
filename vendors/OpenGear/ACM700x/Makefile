#
# Makefile -- Build instructions for OpenGear/ACM700x
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/OpenGear

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	serial/port1,c,4,64 \
	serial/port2,c,4,65 \
	serial/port3,c,4,66 \
	serial/port4,c,4,67

FLASH_DEVICES = \
	ubi,c,90,0 \
	boot,c,90,2 \
	bootenv,c,90,4 \
	factory_image,c,90,6 \
	system-log,c,90,8 \
	mtd-oops,c,90,10 \
	all-spi,c,90,14 \
	config,c,90,16 \
	config_backup,c,90,18 \
	image,c,90,20 \
	image_backup,c,90,22 \
	nvlog,c,90,24

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.version romfs.cryptokey
	$(ROMFSINST) -s /var/dev/sercon /dev/sercon
	$(ROMFSINST) -s /var/dev/modem01 /dev/modem01
	$(ROMFSINST) -s /var/dev/usbmodem01 /dev/usbmodem01
	$(ROMFSINST) -s /var/dev/cellmodem01 /dev/cellmodem01
	$(ROMFSINST) -s /var/dev/cellcommand01 /dev/cellcommand01
	$(ROMFSINST) -s /var/dev/bus /dev/bus
	$(ROMFSINST) -s /var/dev/gps0 /dev/gps0
	$(ROMFSINST) -s /var/tmp /tmp    
	echo "OpenGear/ACM700x" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -A "cons:" -a "cons:unknown:/bin/agetty ttyS0 115200" /etc/default/inittab
	$(ROMFSINST) romfs /
	make -f ../common.mk romfs

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/armada-370-acm7000.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x01000000 -e 0x01000000 -n "Linux-5.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x03000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.bin image.tag image.copy
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
