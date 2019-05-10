#
# Makefile -- Build instructions for Digi/WR11
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
	all,c,90,8 \
	image,c,90,10 \
	image1,c,90,12 \
	config,c,90,14 \
	configblock,b,31,7 \
	opt,c,90,16 \
	optblock,b,31,8

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /bin/led.sh
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /bin/init /init
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Digi WR11" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx28-wr11.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x40800000 -e 0x40800000 -n "Linux-4.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x42000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx28-wr11.dtb /tftpboot/

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
