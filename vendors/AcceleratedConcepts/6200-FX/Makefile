#
#	Makefile -- Build instructions for AcceleratedConcepts/6200-FX
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.squashfs
ZIMAGE    = $(IMAGEDIR)/zImage
ZIMAGEDTB = $(IMAGEDIR)/zImage_dtb
IMAGE     = $(IMAGEDIR)/image.bin
KERNEL    = $(IMAGEDIR)/kernel
UKERNEL   = $(IMAGEDIR)/uImage
IMAGESIZE = 32000000                   # much bigger than our flash, fix later

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS) /usr/var

DEVICES += $(DEVICE_PTY_64)

#
# This will hopefully be the final flash layout. But the prototype boards
# don't have a uboot good enough to save the environment, so we can't make
# this work just yet.
#
FLASH_DEVICES = 

#FLASH_DEVICES = \
#	boot,c,90,0 \
#	bootenv,c,90,2 \
#	rootfs,c,90,4 \
#	kernel,c,90,6 \
#	config,c,90,8 \
#	nvram,c,90,8 \
#	art,c,90,10

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.symlinks romfs.default romfs.rc romfs.skeleton romfs.version
	rm -rf $(ROMFSDIR)/tmp
	mkdir -m 1777 $(ROMFSDIR)/tmp
	rm -rf $(ROMFSDIR)/mnt
	mkdir -m 755 $(ROMFSDIR)/mnt
	$(ROMFSINST) -s /bin/init /sbin/init
	$(ROMFSINST) -s /bin/ifconfig /sbin/ifconfig
	$(ROMFSINST) -s /sbin/syslogd /usr/sbin/syslogd
	$(ROMFSINST) -s /sbin/klogd /usr/sbin/klogd
	$(ROMFSINST) -s /bin/haserl /usr/bin/haserl
	echo "Accelerated Concepts 6200-FX" > $(ROMFSDIR)/etc/issue
	#$(ROMFSINST) -s /etc/config/TZ /etc/TZ
	#$(ROMFSINST) -s /etc/config/passwd /etc/passwd
	#$(ROMFSINST) -s /etc/config/group /etc/group
	#$(ROMFSINST) -s /etc/config/udhcpd.conf /etc/udhcpd.conf
	#> $(ROMFSDIR)/etc/default/start
	$(ROMFSINST) -e CONFIG_USER_BUSYBOX_INIT -A "^ttyS0" -a "ttyS0::respawn:/sbin/getty -L ttyS0 115200 vt100 # GENERIC_SERIAL" /etc/inittab
	echo "/dev/mtd2 0x0000 0x10000 0x10000 1" > $(ROMFSDIR)/etc/fw_env.config

image: image.configs image.dir image.arm.zimage image.squashfs image.bin image.size image.tag image.copy
	rm -rf $(ROMFSDIR)/share
	#cat $(ZIMAGE) $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/at91sam9g25ek.dtb > $(ZIMAGEDTB)
	#mkimage -A arm -O linux -T kernel -C none -a 0x20008000 -e 0x20008000 -n "Linux" -d $(ZIMAGEDTB) $(UKERNEL)
	mkimage -A arm -O linux -T kernel -C none -a 0x20008000 -e 0x20008000 -n "Linux-2.6.39.4 (uClinux-dist)" -d $(ZIMAGE) $(UKERNEL)
	cd $(IMAGEDIR); tar cvf 6200-FX.bin uImage rootfs.squashfs
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(UKERNEL) /tftpboot/.
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/.

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
