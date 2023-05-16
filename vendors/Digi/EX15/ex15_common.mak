#
# Common makefile for all EX15 variants
#

THIS_DIR = $(ROOTDIR)/vendors/Digi/EX15

ROMFSIMG    = $(IMAGEDIR)/rootfs.bin
UROMFSIMG   = $(IMAGEDIR)/urootfs.bin
ZIMAGE      = $(IMAGEDIR)/zImage
IMAGE       = $(IMAGEDIR)/image.bin
VMLINUX     = $(IMAGEDIR)/vmlinux
KERNEL      = $(IMAGEDIR)/kernel
LZKERNEL    = $(IMAGEDIR)/kernel.lzma
UKERNEL     = $(IMAGEDIR)/ukernel.bin
DTB         = $(ROOTDIR)/linux/arch/mips/boot/dts/ralink/MT7621.dtb
BLOADER_IMG = $(IMAGEDIR)/u-boot.nand
BLOADER_IMG_VERSION   = $(shell strings $(BLOADER_IMG) | grep -oE "[0-9]{2}\.[0-9]{1,2}\.[0-9]{1,4}\.[0-9]{1,4}(-.*|$$)")
BLOADER_UPDATE_SCRIPT = $(THIS_DIR)/bootloader_update.sh

SIGNING_ALG = ecdsa
SIGNING_KEY = $(ROOTDIR)/prop/sign_image/devkeys/ex15/ex15_dev.key
IMAGESIZE = 33554432 # 32MB

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
	serial/port1,c,4,65 \

FLASH_DEVICES = \
	boot,c,90,0 \
	bootenv,c,90,2 \
	log,c,90,4 \
	flash,c,90,6 \
	all,c,90,8 \
	image,c,90,10 \
	image1,c,90,12 \
	config,c,90,14 \
	configblock,b,31,7 \
	opt,c,90,16 \
	optblock,b,31,8

all:

clean: image.clean
	rm -f mksquashfs lzma

romfs.common: romfs_dev romfs.dirs romfs.default romfs.version romfs.cryptokey
	$(ROMFSINST) $(THIS_DIR)/start /etc/default/start
	$(ROMFSINST) -p 555 $(THIS_DIR)/mkffs /etc/mkffs
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/led /etc/action.d/led
	$(ROMFSINST) $(THIS_DIR)/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d $(THIS_DIR)/console /etc/inittab.d/console
	$(ROMFSINST) -p 555 $(THIS_DIR)/rc /etc/rc
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/reset_bootcounter.sh /etc/reset_bootcounter.sh
	$(ROMFSINST) -d $(THIS_DIR)/blacklist-hwcrypto.conf /etc/modprobe.d/blacklist-hwcrypto.conf

romfs.post:: romfs.cleanup

lzma: Makefile
	make -C $(ROOTDIR)/user/lzma clean
	make -C $(ROOTDIR)/user/lzma CC="$(HOSTCC)"
	cp  $(ROOTDIR)/user/lzma/build/C/Util/Lzma/lzma .
	make -C $(ROOTDIR)/user/lzma clean

uimage.bin: lzma
	$(OBJCOPY) -O binary $(VMLINUX) $(KERNEL)
	./lzma e $(KERNEL) $(LZKERNEL)
	mkimage -A mips -O linux -T kernel -C lzma -a 0x80001000 -e 0x80001000 -n "Linux" -d $(LZKERNEL) $(UKERNEL)
	mkimage -A mips -O linux -T ramdisk -C none -a 0x88000000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/

bloader.overwrite:
	@if [ "$(BLOADER_OVERWRITE_IMG)" ]; then \
		echo "!!! Overwriting bootloader image with '$(BLOADER_OVERWRITE_IMG)'"; \
		wget --quiet -O $(BLOADER_IMG) "$(BLOADER_OVERWRITE_IMG)" || exit 1; \
	fi

image: bloader.overwrite image.configs image.dir image.mips.vmlinux image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy image.size

# Comment it out to stop generating the bootloader update file
image: bloader.pack

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
