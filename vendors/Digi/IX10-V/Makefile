#
# Makefile -- Build instructions for Digi/IX10
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin
IMAGESIZE = 47185920 # 45MB

SIGNING_ALG = ecdsa



VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,207,17

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
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -d -p 555 /bin/serial-mode
	echo "Digi IX10-V" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/nxp/imx/imx6ull-ix10.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x80800000 -e 0x80800000 -n "Linux-5.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x83800000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/nxp/imx/imx6ull-ix10.dtb $(IMAGEDIR)/

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy image.size

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
