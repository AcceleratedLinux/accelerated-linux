#
# Makefile -- Build instructions for the RaspberryPi-3
#

THIS_DIR  = $(ROOTDIR)/vendors/RaspberryPi/RaspberryPi-3

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
DTB       = $(IMAGEDIR)/bcm2837-rpi-3-a-plus.dtb \
	    $(IMAGEDIR)/bcm2837-rpi-3-b.dtb \
	    $(IMAGEDIR)/bcm2837-rpi-3-b-plus.dtb

DIRS =

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = bin \
	     boot \
	     dev \
	     etc etc/config etc/default \
	     home home/httpd home/httpd/cgi-bin \
	     lib \
	     mnt \
	     opt \
	     proc \
	     sbin \
	     sys \
	     tmp \
	     usr usr/bin usr/sbin usr/var \
	     var

DEVICES_ = null,c,1,3	zero,c,1,5	console,c,5,1	ttyS1,c,4,65

all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean:
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done

romfs: romfs.default
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	for i in $(DEVICES_); do \
		touch $(ROMFSDIR)/dev/@$$i; \
	done
	$(ROMFSINST) ../../Generic/romfs /
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/run/inetd.conf /etc/inetd.conf
	$(ROMFSINST) -s /var/run/pam.conf /etc/pam.conf
	$(ROMFSINST) -s /var/run/inittab /etc/inittab
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d /etc/inittab.d/console
	$(ROMFSINST) -p 555 digi_update_firmware /bin/netflash
	$(ROMFSINST) /etc/default/76-custom.rules
	$(ROMFSINST) -d -s /opt/76-custom.rules /libexec/udev/rules.d/76-custom.rules
	$(ROMFSINST) -d -s /opt/rtl8188eufw.bin /lib/firmware/rtlwifi/rtl8188eufw.bin
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	echo "Raspberry Pi 3" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup
	$(ROMFSINST) ../../AcceleratedConcepts/romfs/etc/default /etc/default

image: mksquashfs
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR) ; \
	$$mksquashfs . $(ROMFSIMG) -all-root -noappend
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/broadcom/bcm2837-rpi-3-a-plus.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/broadcom/bcm2837-rpi-3-b.dtb $(IMAGEDIR)/
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/broadcom/bcm2837-rpi-3-b-plus.dtb $(IMAGEDIR)/
	mkimage -A arm -O linux -T script -C none -n "boot.scr" -d $(THIS_DIR)/boot_scr_first.txt $(IMAGEDIR)/boot.scr
	mkimage -A arm -O linux -T script -C none -n "boot.scr" -d $(THIS_DIR)/boot_scr_norm.txt $(IMAGEDIR)/norm.scr
	[ -e $(THIS_DIR)/config.txt ] && cp $(THIS_DIR)/config.txt $(IMAGEDIR)/ ; \
	$(THIS_DIR)/mk_firmware_image.sh $(IMAGEDIR)/sdcard.img image 3

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
