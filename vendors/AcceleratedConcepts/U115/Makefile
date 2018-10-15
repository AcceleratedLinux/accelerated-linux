#
# Makefile -- Build instructions for AcceleratedConcepts/U115
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin
DTB       = $(IMAGEDIR)/armada-380-u115.dtb

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config etc/inittab.d home mnt opt proc sys tmp usr/var var

MODEMFWDIR = /opt/sierra

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,65

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
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Accelerated Concepts U115" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/armada-380-u115.dtb $(DTB)
	cat $(DTB) >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x01000000 -e 0x01000000 -n "Linux-3.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x3000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.tag image.copy bundle

.PHONY: bundle
bundle: image.tag
	@for f in $(MODEMFWDIR)/MC*.tar.gz ; do \
		[ $$f = "$(MODEMFWDIR)/MC*.tar.gz" ] && return 0; \
		d=$(IMAGEDIR)/`basename $${f}`_bundle; \
		rm -rf $$d; \
		mkdir $$d; \
		tar -C $$d -xf $$f; \
		[ -s $$f.md5 ] && { \
			cd $$d; \
			md5sum -c $$f.md5 || return 1; \
			cd $(ROOTDIR); \
		}; \
		cp $(IMAGE) $$d/$(CONFIG_PRODUCT)-$(VERSIONPKG)-`date "-d$(BUILD_START_STRING)" +%Y%m%d`-`basename $(IMAGE)`; \
		tar -C $$d -caf $(IMAGEDIR)/`basename $${f}` `echo $$d/* | sed "s|$$d/||g"`; \
		rm -rf $$d; \
	done

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
