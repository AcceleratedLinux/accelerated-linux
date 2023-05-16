#
# Common makefile for all LR54 variants
#

THIS_DIR = $(ROOTDIR)/vendors/Digi/LR54

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
VMLINUX   = $(IMAGEDIR)/vmlinux
KERNEL    = $(IMAGEDIR)/kernel
LZKERNEL  = $(IMAGEDIR)/kernel.lzma
UKERNEL   = $(IMAGEDIR)/ukernel.bin
IMAGESIZE 	= 73646080 # 70.2MB
BLOADER_IMG = $(IMAGEDIR)/u-boot.nand

SIGNING_ALG = hmac
SIGNING_KEY = $(ROOTDIR)/prop/sign_image/devkeys/lr54/developer_key

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	all,c,90,0 \
	boot,c,90,2 \
	bootenv,c,90,4 \
	bootenv1,c,90,6 \
	flash,c,90,8 \
	opt,c,90,10 \
	optblock,b,31,5 \
	csid,c,90,12 \
	odm,c,90,14 \
	image,c,90,16 \
	image1,c,90,18

all:

clean: image.clean
	rm -f mksquashfs lzma

romfs.common: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) $(THIS_DIR)/start /etc/default/start
	$(ROMFSINST) -p 555 $(THIS_DIR)/mkffs /etc/mkffs
	#@ Variants have separate LED script
	$(ROMFSINST) -d -p 555 led /etc/action.d/led
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/init_gpios /etc/init_gpios
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/reset_bootcounter.sh /etc/reset_bootcounter.sh
	$(ROMFSINST) $(THIS_DIR)/fw_env.config /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -d $(THIS_DIR)/console /etc/inittab.d/console
	$(ROMFSINST) -d -p 555 $(THIS_DIR)/fwenv_fixup.sh /sbin/fwenv_fixup.sh
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
	mkimage -A mips -O linux -T kernel -C lzma -a 0x81001000 -n "Linux" -d $(LZKERNEL) $(UKERNEL)
	[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/

bloader.overwrite:
	@if [ "$(BLOADER_OVERWRITE_IMG)" ]; then \
		echo "!!! Overwriting bootloader image with '$(BLOADER_OVERWRITE_IMG)'"; \
		wget --quiet -O $(BLOADER_IMG) "$(BLOADER_OVERWRITE_IMG)" || exit 1; \
	fi

image: bloader.overwrite image.configs image.dir image.mips.vmlinux image.squashfs uimage.bin image.ukernel.bin image.sign-atmel image.tag image.copy image.size

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
