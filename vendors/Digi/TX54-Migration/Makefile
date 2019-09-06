#
# Makefile -- Build instructions for Digi/TX54
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
VMLINUX   = $(IMAGEDIR)/vmlinux
KERNEL    = $(IMAGEDIR)/kernel
LZKERNEL  = $(IMAGEDIR)/kernel.lzma
UKERNEL   = $(IMAGEDIR)/ukernel.bin
XOS_ROOTFS_DIR = $(ROOTDIR)/xos_rootfs

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/AcceleratedConcepts
ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += etc etc/config home proc sys tmp usr/var var opt

ifndef CONFIG_MIGRATION_FIRMWARE_IMAGE
$(error "Please configure CONFIG_MIGRATION_FIRMWARE_IMAGE")
endif

ifndef CONFIG_MIGRATION_BOOTLOADER_IMAGE
$(error "Please configure CONFIG_MIGRATION_BOOTLOADER_IMAGE")
endif

all:

clean: image.clean
	rm -f mksquashfs lzma
	rm -rf $(XOS_ROOTFS_DIR)

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) /etc/config/start
	$(ROMFSINST) -d -p 555 /etc/init_gpios
	$(ROMFSINST) -d -p 555 /etc/reset_bootcounter.sh
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) -s /bin/init /init
	echo "Digi TX54 Migration" > $(ROMFSDIR)/etc/issue

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
	#[ "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot/

xos-rootfs.squashfs: mksquashfs
	echo "Constructing XOS rootfs in $(XOS_ROOTFS_DIR)"

	mkdir -p $(XOS_ROOTFS_DIR)
	rm -rf $(XOS_ROOTFS_DIR)/*

	mkdir $(XOS_ROOTFS_DIR)/firmware
	cp $(CONFIG_MIGRATION_FIRMWARE_IMAGE) $(XOS_ROOTFS_DIR)/firmware/firmware.bin; \
	cp $(CONFIG_MIGRATION_BOOTLOADER_IMAGE) $(XOS_ROOTFS_DIR)/firmware/bootloader.bin; \

	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(XOS_ROOTFS_DIR) ; \
		$$mksquashfs . $(ROMFSIMG) -all-root -noappend $(SQUASH_BCJ) $(SQUASH_ENDIAN)

image.xos-sign:
	$(ROOTDIR)/prop/sign_image/sign_image.py $(UKERNEL) foo tx54
	$(ROOTDIR)/prop/sign_image/sign_image.py $(ROMFSIMG) foo tx54
	$(ROOTDIR)/prop/sign_image/gen_kernel.py $(IMAGEDIR) tx54

image.xos-fw:
	./mk_xos_fw.sh "wr54" "$(VERSIONPKG)" "$$(cat $(ROOTDIR)/romfs/etc/version)" "${ROOTDIR}/prop/sign_image/development.pem" "$(ROOTDIR)/images/tx54_migration.bin" "$(ROOTDIR)"

image: image.mips.vmlinux uimage.bin xos-rootfs.squashfs image.xos-sign image.xos-fw

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
