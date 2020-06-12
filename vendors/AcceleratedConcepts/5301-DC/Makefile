#
#	Makefile -- Build instructions for AcceleratedConcepts/5301-DC
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
KERNEL    = $(IMAGEDIR)/kernel
UKERNEL   = $(IMAGEDIR)/ukernel.bin
# IMAGESIZE = 7864320
IMAGESIZE = 16000000

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var

DEVICES += $(DEVICE_PTY_64) \
	slic0,c,25,0 \
	slic1,c,25,1

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	config,c,90,4 \
	configblock,b,31,2 \
	image,c,90,6 \
	image1,c,90,8 \
	all,c,90,10

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.default romfs.version
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /etc/config/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) -d /etc/inittab.d/console
	echo "Accelerated Concepts 5301-DC" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/imx25-5300-dc.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x80060000 -e 0x80060000 -n "Linux" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x80800000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

factory.bin:
	> $(IMAGEDIR)/factory.bin
	$(ROOTDIR)/bin/pad-ff.sh 192 $(IMAGEDIR)/boot.bin >> $(IMAGEDIR)/factory.bin
	$(ROOTDIR)/bin/pad-ff.sh 64 bootenv.bin >> $(IMAGEDIR)/factory.bin
	$(ROOTDIR)/bin/pad-ff.sh 768 /dev/null >> $(IMAGEDIR)/factory.bin
	$(ROOTDIR)/bin/pad-ff.sh 7680 $(IMAGEDIR)/image.bin >> $(IMAGEDIR)/factory.bin
	$(ROOTDIR)/bin/pad-ff.sh 7680 $(IMAGEDIR)/image.bin >> $(IMAGEDIR)/factory.bin
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || ( echo cp $(IMAGEDIR)/factory.bin /tftpboot/.; cp $(IMAGEDIR)/factory.bin /tftpboot/. )
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || ( echo cp $(IMAGEDIR)/factory.bin /tftpboot/$(CONFIG_PRODUCT)-factory.bin; cp $(IMAGEDIR)/factory.bin /tftpboot/$(CONFIG_PRODUCT)-factory.bin )

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy factory.bin image.size

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
