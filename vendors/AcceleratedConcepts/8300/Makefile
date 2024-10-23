#
# Makefile -- Build instructions for AcceleratedConcepts/8300
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var
DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	log,c,90,4 \
	flash,c,90,6 \
	all,c,90,8 \
	image,c,90,10 \
	config,c,90,12 \
	recover,c,90,14 \
	recover_images,c,90,16 \
	extra,c,90,18

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.default romfs.version romfs.boot romfs.cryptokey
	mkdir -p $(ROMFSDIR)/tmp $(ROMFSDIR)/mnt
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 755 /etc/default/start
	$(ROMFSINST) -p 555 /etc/switch.sh
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /etc/config/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -d -p 555 /etc/action.d/led

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/marvell/armada-370-ac8300.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x01000000 -e 0x01000000 -n "Linux" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x03000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
