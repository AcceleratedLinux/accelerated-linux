#
# Makefile -- Build instructions for Digi/ConnectCore-93
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/cc93.itb
ITBRAMIMG = $(IMAGEDIR)/cc93-initrd.itb
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(ITBIMG)
IMAGESIZE = 128000000 # 128MB

SIGNING_ALG = ecdsa

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/cli etc/config etc/runt.d etc/ssl etc/ssl/firmware home proc sys tmp usr/sbin usr/var var opt bin

DEVICES += $(DEVICE_PTY_64) \
	serial/port1,c,246,0

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,0 \
	image,c,90,4 \
	image1,c,90,6

all:

clean: image.clean
	-rm version.info

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) -p 440 console /etc/factory.console
	$(ROMFSINST) -d /etc/inittab.d/console
	#$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Digi CC93" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/imx93-cc93-dvk.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp cc93.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f cc93.its $(ITBIMG) )
	cp cc93-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f cc93-initrd.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/cc93*.its

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin image.tag image.copy image.size
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGEDIR)/kernel /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGEDIR)/rootfs.bin /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGEDIR)/imx93-cc93-dvk.dtb /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
