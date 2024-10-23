#
# Makefile -- Build instructions for Digi/TX40
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel.bin
ZKERNEL   = $(IMAGEDIR)/kernel.gz
ifneq ($(CONFIG_DEFAULTS_DIGI_TX40_EVT),)
ITB_NAME           = tx40-evt
ITBRAMIMAGE_NAME   = tx40-evt-initrd
DTBNAME            = fsl-ls1028a-tx40-evt.dtb
PBLIMG    = $(IMAGEDIR)/bl2_flexspi_nor_rcw_tx40_1500_evt.pbl
else
ITB_NAME           = tx40
ITBRAMIMAGE_NAME   = tx40-initrd
DTBNAME            = fsl-ls1028a-tx40.dtb
PBLIMG    = $(IMAGEDIR)/bl2_flexspi_nor_rcw_tx40_1300_dvt.pbl
endif
ITBIMG    = $(IMAGEDIR)/$(ITB_NAME).itb
ITBRAMIMG = $(IMAGEDIR)/$(ITBRAMIMAGE_NAME).itb
UKERNEL   = $(ITBIMG)
IMAGE     = $(IMAGEDIR)/image.bin
IMAGESIZE = 104857600 # 100MB
UBOOTIMG  = $(IMAGEDIR)/fip.bin
UBOOTIMG_OFFSET = 1048576 # 1MB
SPINORIMG = $(IMAGEDIR)/spi_nor.bin
BLOADER_IMG = $(UBOOTIMG)
BLOADER_IMG_VERSION = $(shell strings $(BLOADER_IMG) | grep -oE "[0-9]{2}\.[0-9]{1,2}\.[0-9]{1,4}\.[0-9]{1,4}(-.*|$$)")

SIGNING_ALG = ecdsa



PRODUCTION_BUILD=0
ifeq ($(BUILD_TYPE),candidate)
PRODUCTION_BUILD=1
endif

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/cli etc/config etc/runt.d etc/ssl etc/ssl/firmware home proc sys tmp usr/sbin usr/var var opt bin

DEVICES += $(DEVICE_PTY_64) \
	ttyGPS,c,4,65

FLASH_DEVICES = \
	pbl,c,90,0 \
	boot,c,90,2 \
	bootenv,c,90,4 \
	image,b,179,1 \
	image1,b,179,2 \
	opt,b,179,3 \
	config,b,179,4

all:

clean: image.clean
	-rm version.info

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /etc/mkffs.fixup
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -p 555 /etc/emc
	$(ROMFSINST) -p 555 /etc/factory_wlan_test
	$(ROMFSINST) -d -p 555 /etc/reset_bootcounter.sh
	echo "Digi TX40" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -d -p 644 acpi/acpi.map /etc/acpi.map
	$(ROMFSINST) -d -p 644 acpi/acpid.conf /etc/acpid.conf
	$(ROMFSINST) -d -p 755 acpi/ignition /etc/acpi/ignition
	$(ROMFSINST) -d -p 755 acpi/vin_low /etc/acpi/vin_low
	$(ROMFSINST) -d -p 755 acpi/vin_critical /etc/acpi/vin_critical
	$(ROMFSINST) -d -p 755 set_temp_pwr_off_delay /bin/set_temp_pwr_off_delay
	$(ROMFSINST) -d -p 644 /libexec/udev/rules.d/65-tx40-serial.rules
	$(ROMFSINST) -p 555 /etc/bootupdate_platform_check.sh

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/freescale/$(DTBNAME) $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	gzip -c $(KERNEL) > $(ZKERNEL)
	cp $(ITB_NAME).its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f $(ITB_NAME).its $(ITBIMG) )
	cp $(ITBRAMIMAGE_NAME).its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f $(ITBRAMIMAGE_NAME).its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/tx40*.its

nor_flash.pack:
	@if [ -f $(PBLIMG) ] ; then \
		echo "Creating SPI NOR flash image for manufacturing..." ; \
		dd if=/dev/zero bs=512 count=$$(($(UBOOTIMG_OFFSET) / 512)) | tr "\000" "\377" >$(SPINORIMG) ; \
		dd if=$(PBLIMG) of=$(SPINORIMG) conv=notrunc seek=0 ; \
		dd if=$(UBOOTIMG) of=$(SPINORIMG) conv=notrunc bs=512 seek=$$(($(UBOOTIMG_OFFSET) / 512)) ; \
		cd $(dir $(SPINORIMG)); md5sum $(notdir $(SPINORIMG)) > $(SPINORIMG).md5 ; \
		tar zcf $(SPINORIMG).tar.gz -C $(dir $(SPINORIMG)) $(notdir $(SPINORIMG)) $(notdir $(SPINORIMG).md5) ; \
		rm -f $(SPINORIMG) $(SPINORIMG).md5 ; \
	fi

image: image.configs image.dir image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy nor_flash.pack
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBIMG) /tftpboot/
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ITBRAMIMG) /tftpboot/

# Comment it out to stop generating the bootloader update file
image: bloader.pack

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
