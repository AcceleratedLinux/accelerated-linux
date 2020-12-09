#
# Makefile -- Build instructions for Digi/ConnectEZ-2
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel.bin
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/connectez2.itb
ITBRAMIMG = $(IMAGEDIR)/connectez2-initrd.itb
UKERNEL   = $(ITBIMG)
IMAGE     = $(IMAGEDIR)/image.bin

SIGNING_ALG = ecdsa

PRODUCTION_BUILD=0
ifeq ($(BUILD_TYPE),candidate)
PRODUCTION_BUILD=1
endif

FIRMWARE_CA_PEM = $(ROOTDIR)/prop/sign_image/production/awusb/firmware_ca.pem
DEV_FIRMWARE_CA_PEM = $(ROOTDIR)/prop/sign_image/devkeys/awusb/firmware_ca.pem
ifneq ($(PRODUCTION_BUILD),1)
$(info Developer Build $(BUILD_TYPE))
SIGN_IMAGE_SCRIPT = $(ROOTDIR)/prop/sign_image/sign_image.py
SIGNER_CERT_PEM = $(ROOTDIR)/prop/sign_image/devkeys/awusb/development.pem
SIGNER_OPTS =
else
$(info PRODUCTION BUILD)
SIGN_IMAGE_SCRIPT = $(ROOTDIR)/prop/sign_image/sign_awusb.py
SIGNER_CERT_PEM = $(ROOTDIR)/prop/sign_image/production/awusb/signer_cert.pem
SIGNER_OPTS = -p
endif

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/cli etc/config etc/runt.d etc/ssl etc/ssl/firmware home proc sys tmp usr/sbin usr/var var opt bin

DEVICES += $(DEVICE_PTY_64) \
	serial/port1,c,4,66 \
	serial/port2,c,4,67

FLASH_DEVICES = \
	boot,c,90,2 \
	bootenv,c,90,6 \
	image,b,179,1 \
	image1,b,179,2 \
	opt,b,179,3

all:

clean: image.clean
	-rm version.info

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) ../ConnectEZ1/start /etc/default/start
	$(ROMFSINST) -d -p 555 ../ConnectEZ1/led /etc/action.d/led
	$(ROMFSINST) ../ConnectEZ1/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -p 555 ../ConnectEZ1/mkffs /etc/mkffs
	[ ! -d $(ROOTDIR)/prop ] || $(ROMFSINST) -p 400 $(FIRMWARE_CA_PEM) /etc/ssl/firmware/firmware_ca.pem
	[ ! -d $(ROOTDIR)/prop ] || $(ROMFSINST) -p 400 $(DEV_FIRMWARE_CA_PEM) /etc/ssl/firmware/dev_firmware_ca.pem
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	echo "Digi ConnectEZ-2" > $(ROMFSDIR)/etc/issue
	echo "NETWORK_PRODUCT=connectez" > version.info
	echo "PRODUCT_VERSION="$$(sed -e 's?^[^ ]* [^ ]* ??' -e 's?-.*$$??' < $(ROMFSDIR)/etc/version) >> version.info
	echo "PRODUCT_BUILDSTRING="$$(sed -e 's?^[^ ]* [^ ]* ??' -e 's?-.*$$??' < $(ROMFSDIR)/etc/version) connectez -- $(BUILD_START_STRING) >> version.info
	echo "SUPPORTED_PRODUCT_IDS=0xd00a" >> version.info
	echo "MIN_RUNNING_RELEASE=" >> version.info
	$(ROMFSINST) -p 444 /etc/version.info

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/connectez2.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp connectez2.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f connectez2.its $(ITBIMG) )
	cp connectez2-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f connectez2-initrd.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/connectez2*.its

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
