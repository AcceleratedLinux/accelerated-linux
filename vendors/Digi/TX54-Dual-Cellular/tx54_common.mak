#
# Common makefile for all TX54 variants
#

THIS_DIR = $(ROOTDIR)/vendors/Digi/TX54-Dual-Cellular

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
VMLINUX   = $(IMAGEDIR)/vmlinux
KERNEL    = $(IMAGEDIR)/kernel
LZKERNEL  = $(IMAGEDIR)/kernel.lzma
UKERNEL   = $(IMAGEDIR)/ukernel.bin
HWNAME    = $(shell echo $(CONFIG_PRODUCT) | tr '[:upper:]' '[:lower:]' | tr '-' '_')
MFGIMG    = $(IMAGEDIR)/$(HWNAME)_mfg.tar.gz

SIGNING_ALG = ecdsa
SIGNING_EXTRA =

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	ttyBLE,c,4,64 \
	ttyGPS,c,4,66

FLASH_DEVICES = \
	all,c,90,0 \
	boot,c,90,2 \
	bootenv,c,90,4 \
	bootenv1,c,90,6 \
	flash,c,90,8 \
	userfs,c,90,10 \
	csid,c,90,12 \
	odm,c,90,14 \
	image,c,90,16 \
	image1,c,90,18 \
	opt,c,90,20 \
	optblock,b,31,10


all:

clean.common: image.clean
	rm -f mksquashfs lzma

romfs.common: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) $(THIS_DIR)/start /etc/default/start
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/led /etc/action.d/led
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/reset_bootcounter.sh /etc/reset_bootcounter.sh
	$(ROMFSINST) $(THIS_DIR)/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d $(THIS_DIR)/console /etc/inittab.d/console
	$(ROMFSINST) -d -p 755 $(THIS_DIR)/pwrbtn.sh /etc/acpi/events/PWRF/00000080
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/fwenv_fixup.sh /sbin/fwenv_fixup.sh

romfs.post:: romfs.cleanup

lzma: Makefile
	make -C $(ROOTDIR)/user/lzma clean
	make -C $(ROOTDIR)/user/lzma CC="$(HOSTCC)"
	cp  $(ROOTDIR)/user/lzma/build/C/Util/Lzma/lzma .
	make -C $(ROOTDIR)/user/lzma clean

uimage.bin: lzma
	$(OBJCOPY) -O binary $(VMLINUX) $(KERNEL)
	./lzma e $(KERNEL) $(LZKERNEL)
	#mkimage -A mips -O linux -T kernel -C lzma -a 0x80001000 -e 0x80001000 -n "Linux" -d $(LZKERNEL) $(UKERNEL)
	mkimage -A mips -O linux -T kernel -C lzma -a 0x81001000 -n "Linux" -d $(LZKERNEL) $(UKERNEL)
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/

image: image.configs image.dir image.mips.vmlinux image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy
	@# Create manufacturing image only if U-Boot is built
	@if [ -f "$(IMAGEDIR)/u-boot.nand" ]; then \
		echo "Creating manufacturing package..."; \
		echo "PRODUCT=$(HWNAME)" > $(IMAGEDIR)/version; \
		echo "PRODUCT_VERSION=$(VERSIONPKG)" >> $(IMAGEDIR)/version; \
		tar --transform='flags=r;s|u-boot.nand|uboot.img|' -zcvf $(MFGIMG) -C $(IMAGEDIR) image.bin u-boot.nand version; \
	else \
		echo "!!! Warning: manufacturing package is not generated in open-source builds !!!"; \
	fi

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
