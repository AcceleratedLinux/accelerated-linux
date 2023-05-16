#
# Makefile -- Build instructions for Digi/EX50
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/ex50.itb
ITBRAMIMG = $(IMAGEDIR)/ex50-initrd.itb
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(ITBIMG)
IMAGESIZE = 67108864 # 64MB

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
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -p 440 console /etc/factory.console
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /etc/power-led.sh
	$(ROMFSINST) -p 555 /etc/update-boot.sh
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Digi EX50" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -d /etc/modprobe.d/blacklist-spi-nor.conf
	$(ROMFSINST) -p 555 /etc/wifi_mac_calc.sh

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/qcom/ex50.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp ex50.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f ex50.its $(ITBIMG) )
	cp ex50-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f ex50-initrd.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/ex50*.its

boot.bin:
	if [ -d $(ROOTDIR)/prop ] ; then \
		ROOTDIR=$(ROOTDIR) ; \
		$(ROOTDIR)/prop/qca-sectools/mk-nosign-blobs.sh ; \
		$(ROOTDIR)/prop/qca-sectools/mk-boot-images.sh ; \
	fi

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin boot.bin image.sign-atmel image.tag image.copy image.size
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
