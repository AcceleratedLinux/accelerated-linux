#
# Makefile -- Build instructions for the Digi/IoT-Gateway-Small
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin
BOOT_SCR  = $(IMAGEDIR)/boot.scr
DTB       = digi-iot-gateway.dtb

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt opt/ext
THIS_DIR = $(ROOTDIR)/vendors/Digi/IoT-Gateway-Small

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \

FLASH_DEVICES = \
	bootenv=/dev/ubi0_0 \
	image=/dev/ubi0_2 \
	image1=/dev/ubi0_3 \
	config=/dev/ubi1_0 \
	opt=/dev/ubi1_1 \

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
	$(ROMFSINST) -e CONFIG_PROP_XBEE -p 644 /etc/xbee.conf
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-usb1_automount.rules
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/76-usb2_automount.rules
	$(ROMFSINST) -d -p 644 /etc/watchdog.d/watchdog.conf

	echo "Digi IoT Gateway" > $(ROMFSDIR)/etc/issue

romfs.override_files:
	$(ROMFSINST) -p 755 /bin/xbee

romfs.post:: romfs.override_files romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/st/$(DTB) >> $(ZIMAGE)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/st/$(DTB) $(IMAGEDIR)/$(DTB)
	mkimage -A arm -O linux -T kernel -C none -a 0xc2000000 -e 0xc2000000 -n "Linux-4.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0xc4000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/st/$(DTB) /tftpboot/

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
