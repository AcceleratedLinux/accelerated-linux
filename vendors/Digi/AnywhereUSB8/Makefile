#
# Makefile -- Build instructions for Digi/AnywhereUSB8
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ROMFSSIG  = $(IMAGEDIR)/rootfs.bin.sig
ROMFSSIZE = $(IMAGEDIR)/rootfs.bin.size
KERNEL    = $(IMAGEDIR)/kernel.bin
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/anywhereusb8.itb
ITBSIG    = $(IMAGEDIR)/anywhereusb8.itb.sig

# Accelerated style update binary, not yet supported
IMAGE     = $(IMAGEDIR)/image.bin

#UKERNEL   = $(IMAGEDIR)/ukernel.bin
#UROMFSIMG = $(IMAGEDIR)/urootfs.bin

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config etc/runt.d home proc sys tmp usr/var var opt bin

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,65 \

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	image,c,90,10 \
	image1,c,90,12 \
	opt,b,179,5

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -p 555 /etc/runt.d/startup.product
	$(ROMFSINST) -p 755 /bin/fw_printenv
	$(ROMFSINST) -s /bin/fw_printenv /bin/fw_setenv
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	echo "Digi AnywhereUSB8" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/anywhereusb8.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp anywhereusb8.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f anywhereusb8.its $(ITBIMG) )
	cp anywhereusb8-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f anywhereusb8-initrd.its anywhereusb8-initrd.itb )
	rm -f $(IMAGEDIR)/anywhereusb8*.its

image: image.configs image.dir image.squashfs uimage.bin image.tag image.copy
	if [ -x $(ROOTDIR)/prop/sign_image/sign_image.py ] ; then \
		$(ROOTDIR)/prop/sign_image/sign_image.py $(ITBIMG) foo awusb ; \
		$(ROOTDIR)/prop/sign_image/sign_image.py $(ROMFSIMG) foo awusb ; \
		stat --printf="%s\n" $(ROMFSIMG) > $(ROMFSSIZE) ; \
		[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBSIG) /tftpboot/kernel.sig ; \
		[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSSIG) /tftpboot/rootfs.sig ; \
		[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSSIZE) /tftpboot/rootfs.size ; \
	fi
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/kernel
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/rootfs

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
