#
# Makefile -- Build instructions for Digi/ConnectEZ-32
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel.bin
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ITBIMG    = $(IMAGEDIR)/connectez32.itb
ITBRAMIMG = $(IMAGEDIR)/connectez32-initrd.itb
UKERNEL   = $(ITBIMG)
IMAGE     = $(IMAGEDIR)/image.bin
IMAGESIZE = 94371840 # 90MB

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

DEVICES += serial/console,c,4,64 \
	serial/port1,c,4,65 \
	serial/port2,c,4,66 \
	serial/port3,c,4,67 \
	serial/port4,c,4,68 \
	serial/port5,c,4,69 \
	serial/port6,c,4,70 \
	serial/port7,c,4,71 \
	serial/port8,c,4,72 \
	serial/port9,c,4,73 \
	serial/port10,c,4,74 \
	serial/port11,c,4,75 \
	serial/port12,c,4,76 \
	serial/port13,c,4,77 \
	serial/port14,c,4,78 \
	serial/port15,c,4,79 \
	serial/port16,c,4,80 \
	serial/port17,c,4,81 \
	serial/port18,c,4,82 \
	serial/port19,c,4,83 \
	serial/port20,c,4,84 \
	serial/port21,c,4,85 \
	serial/port22,c,4,86 \
	serial/port23,c,4,87 \
	serial/port24,c,4,88 \
	serial/port25,c,4,89 \
	serial/port26,c,4,90 \
	serial/port27,c,4,91 \
	serial/port28,c,4,92 \
	serial/port29,c,4,93 \
	serial/port30,c,4,94 \
	serial/port31,c,4,95 \
	serial/port32,c,4,96

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
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) ../ConnectEZ1/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -p 555 /etc/mkffs
	[ ! -d $(ROOTDIR)/prop ] || $(ROMFSINST) -p 400 $(FIRMWARE_CA_PEM) /etc/ssl/firmware/firmware_ca.pem
	[ ! -d $(ROOTDIR)/prop ] || $(ROMFSINST) -p 400 $(DEV_FIRMWARE_CA_PEM) /etc/ssl/firmware/dev_firmware_ca.pem
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/factory.console
	$(ROMFSINST) -d -p 644 ../ConnectEZ8W/76-sdcard_poll_rate.rules /libexec/udev/rules.d/76-sdcard_poll_rate.rules
	$(ROMFSINST) -d -p 644 ../ConnectEZ8W/76-cez_automount.rules /libexec/udev/rules.d/76-cez_automount.rules
	$(ROMFSINST) -d -p 555 /bin/serial-mode
	echo "Digi ConnectEZ-32" > $(ROMFSDIR)/etc/issue
	echo "NETWORK_PRODUCT=connectez" > version.info
	echo "PRODUCT_VERSION="$$(sed -e 's?^[^ ]* [^ ]* ??' -e 's?-.*$$??' < $(ROMFSDIR)/etc/version) >> version.info
	echo "PRODUCT_BUILDSTRING="$$(sed -e 's?^[^ ]* [^ ]* ??' -e 's?-.*$$??' < $(ROMFSDIR)/etc/version) connectez -- $(BUILD_START_STRING) >> version.info
	echo "MIN_RUNNING_RELEASE=" >> version.info
	$(ROMFSINST) -p 444 /etc/version.info

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/connectez32.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp connectez32.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f connectez32.its $(ITBIMG) )
	cp connectez32-initrd.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f connectez32-initrd.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/connectez32*.its

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy image.size
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
