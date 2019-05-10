#
# Makefile -- Build instructions for Digi/IX14
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin

SIGNING_ALG = ecdsa
SIGNING_EXTRA =

DTB       = imx6ul-ix14.dtb

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,207,18

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	log,c,90,4 \
	flash,c,90,6 \
	image,c,90,8 \
	image1,c,90,10 \
	config,c,90,12 \
	configblock,b,31,6 \
	opt,c,90,14 \
	optblock,b,31,7

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -p 555 /etc/clear-bootcount.sh
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -e CONFIG_PROP_BLUETOOTH -d -p 555 bluez/bluetooth /bin/bluetooth
	$(ROMFSINST) -e CONFIG_PROP_BLUETOOTH -d -p 555 bluez/bluetooth-action /etc/action.d/bluetooth
	$(ROMFSINST) -e CONFIG_PROP_BLUETOOTH -d -p 555 bluez/bluetooth-update /etc/runt.d/bluetooth
	$(ROMFSINST) -e CONFIG_PROP_BLUETOOTH -d -p 555 bluez/main.conf /etc/bluetooth/main.conf
	$(ROMFSINST) -d -p 555 /etc/action.d/led
	$(ROMFSINST) -p 555 /bin/cellular_power.sh
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	echo "Digi IX14" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -d -p 644 wlan/modprobe-qualcomm.conf /etc/modprobe.d/modprobe-qualcomm.conf
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -d -p 755 wlan/qualcomm.sh /etc/qualcomm.sh
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -d -p 644 wlan/bdwlan30_US.bin /lib/firmware/bdwlan30_US.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -s bdwlan30_US.bin /lib/firmware/bdwlan30.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -p 644 wlan/LICENCE.atheros_firmware /lib/firmware/LICENCE.atheros_firmware
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -p 644 wlan/otp30.bin /lib/firmware/otp30.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -p 644 wlan/qwlan30.bin /lib/firmware/qwlan30.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -p 644 wlan/utf30.bin /lib/firmware/utf30.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -s bdwlan30_US.bin /lib/firmware/utfbd30.bin
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -d -p 644 $(ROOTDIR)/modules/qcacld/firmware_bin/WCNSS_cfg.dat /lib/firmware/wlan/cfg.dat
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -p 644 $(ROOTDIR)/modules/qcacld/firmware_bin/WCNSS_qcom_cfg.ini /lib/firmware/wlan/qcom_cfg.ini
	$(ROMFSINST) -e CONFIG_QCA_CLD_WLAN -s /etc/config/wlan/wlan_mac.bin /lib/firmware/wlan/wlan_mac.bin

romfs.post:: romfs.cleanup

uimage.bin:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/$(DTB) >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x80800000 -e 0x80800000 -n "Linux-4.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x83800000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/$(DTB) $(IMAGEDIR)/

image: image.configs image.dir image.arm.zimage image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
