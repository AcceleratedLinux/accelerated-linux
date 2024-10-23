#
# Makefile -- Build instructions for Digi/IX40
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/ix40.itb
ITBRAMIMG = $(IMAGEDIR)/ix40-initrd.itb
UKERNEL   = $(ITBIMG)
IMAGE     = $(IMAGEDIR)/image.bin
IMAGESIZE = 94371840 # 90MB

SIGNING_ALG = ecdsa



VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += bin etc etc/config home proc sys tmp usr/bin usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	serial/port1,c,207,17

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	log,c,90,4 \
	all,c,90,6 \
	image,b,179,1 \
	image1,b,179,2 \
	opt,b,179,3

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -d -p 555 /bin/serial-mode
	$(ROMFSINST) -p 555 /etc/rc
	echo "Digi IX40" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/imx8mp-ix40.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp ix40*.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f ix40.its $(ITBIMG) )
	cp ix40-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f ix40-initrd.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/ix40*.its

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy image.size
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
