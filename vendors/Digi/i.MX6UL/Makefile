#
# Makefile -- Build instructions for AcceleratedConcepts/6310-CX
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	log,c,90,4 \
	flash,c,90,6 \
	image,c,90,8 \
	image1,c,90,10 \
	config,c,90,12 \
	configblock,b,31,6 \
	opt,c,90,14 \
	optblock,b,31,7

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /bin/led.sh
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Accelerated Concepts i.MX6UL" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx6ul-ccimx6ulsbc.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x80800000 -e 0x80800000 -n "Linux-4.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x83800000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx6ul-ccimx6ulsbc.dtb $(IMAGEDIR)/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx6ul-ccimx6ulsbc.dtb /tftpboot/

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
