#
# Makefile -- Build instructions for AcceleratedConcepts/5401-RM
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin
IMAGESIZE = 7864320

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,65 \
	serial/port2,c,4,66 \
	serial/port3,c,4,67 \
	serial/port4,c,4,68

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	flash,c,90,4 \
	all,c,90,6 \
	image,c,90,8 \
	image1,c,90,10 \
	config,c,90,12 \
	configblock,b,31,6 \
	opt,c,90,14 \
	optblock,b,31,7

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -d /libexec/udev/rules.d/77-mm-pci-blacklist.rules
	$(ROMFSINST) -d /etc/inittab.d/console
	echo "Accelerated Concepts 5401-RM" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/kirkwood-5400-rm.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x2000000 -e 0x2000000 -n "Linux" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x3000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(UKERNEL)  /tftpboot/
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(UKERNEL)  /tftpboot/uImage
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(UROMFSIMG)  /tftpboot/
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(UROMFSIMG)  /tftpboot/uInitrd

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
