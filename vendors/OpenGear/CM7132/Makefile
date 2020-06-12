#
# Makefile -- Build instructions for OpenGear/CM7132
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
	serial/port1,c,4,64 \
	serial/port2,c,4,65 \
	serial/port3,c,4,66 \
	serial/port4,c,4,67 \
	serial/port5,c,4,68 \
	serial/port6,c,4,69 \
	serial/port7,c,4,70 \
	serial/port8,c,4,71 \
	serial/port9,c,4,72 \
	serial/port10,c,4,73 \
	serial/port11,c,4,74 \
	serial/port12,c,4,75 \
	serial/port13,c,4,76 \
	serial/port14,c,4,77 \
	serial/port15,c,4,78 \
	serial/port16,c,4,79 \
	serial/port17,c,4,80 \
	serial/port18,c,4,81 \
	serial/port19,c,4,82 \
	serial/port20,c,4,83 \
	serial/port21,c,4,84 \
	serial/port22,c,4,85 \
	serial/port23,c,4,86 \
	serial/port24,c,4,87 \
	serial/port25,c,4,88 \
	serial/port26,c,4,89 \
	serial/port27,c,4,90 \
	serial/port28,c,4,91 \
	serial/port29,c,4,92 \
	serial/port30,c,4,93 \
	serial/port31,c,4,94 \
	serial/port32,c,4,95 \

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	config,c,90,4 \
	image,c,90,6 \
	all,c,90,8 \
	configblock,b,31,2 

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) ../CM7148/start /etc/default/start
	$(ROMFSINST) -d -p 555 ../CM7148/led /etc/action.d/led
	$(ROMFSINST) ../CM7148/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	echo "OpenGear/CM7132" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/armada-370-cm7100.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x01000000 -e 0x01000000 -n "Linux-5.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x03000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

fskimage.bin: image.configs image.dir image.arm.zimage image.squashfs
	rm -f $(IMAGE)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/armada-370-cm7100.dtb >> $(ZIMAGE)
	cat $(ROMFSIMG) $(ZIMAGE) >> $(IMAGE)


image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.bin image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
