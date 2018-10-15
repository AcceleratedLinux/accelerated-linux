#
#	Makefile -- Build instructions for AcceleratedConcepts/NetReach
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
ZROMFSIMG = $(IMAGEDIR)/rootfs.bin.lzma
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
VMLINUX   = $(IMAGEDIR)/vmlinux
IMAGE     = $(IMAGEDIR)/image.bin
KERNEL    = $(IMAGEDIR)/kernel
LZKERNEL  = $(IMAGEDIR)/kernel.lzma
UKERNEL   = $(IMAGEDIR)/ukernel.bin
IMAGESIZE = 32000000                   # much bigger than our flash, fix later

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS) \
	etc_ro

DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	boot,c,90,0 \
        bootenv,c,90,2 \
        config,c,90,4 \
	rootfs,c,90,6 \
	kernel,c,90,8 \
        image,c,90,10

all:

clean: image.clean

romfs.first:
	$(ROMFSINST) -e CONFIG_USER_GOAHEAD_GOAHEAD $(VENDOR_ROMFS_DIR)/romfs-goahead /

romfs: romfs.first romfs_dev romfs.dirs romfs.symlinks romfs.default romfs.rc romfs.version
	rm -rf $(ROMFSDIR)/tmp
	mkdir -m 1777 $(ROMFSDIR)/tmp
	rm -rf $(ROMFSDIR)/mnt
	mkdir -m 755 $(ROMFSDIR)/mnt
	$(ROMFSINST) -s /bin/ifconfig /sbin/ifconfig
	$(ROMFSINST) -s /etc/config/TZ /etc/TZ
	$(ROMFSINST) -s /etc/config/passwd /etc/passwd
	$(ROMFSINST) -s /etc/config/group /etc/group
	$(ROMFSINST) -s /etc/config/udhcpd.conf /etc/udhcpd.conf

./lzma: Makefile
	make -C $(ROOTDIR)/user/lzma clean
	make -C $(ROOTDIR)/user/lzma CC="$(HOSTCC)"
	cp  $(ROOTDIR)/user/lzma/build/C/Util/Lzma/lzma .
	make -C $(ROOTDIR)/user/lzma clean

image: ./lzma image.configs image.dir image.mips.vmlinux image.squashfs image.bin image.size image.tag image.copy
	$(OBJCOPY) -O binary $(VMLINUX) $(KERNEL)
	./lzma e $(KERNEL) $(LZKERNEL)
	[ `stat -c %s $(LZKERNEL)` -lt 1703936 ] || { echo "Kernel too big."; exit 1; }
	#./lzma e $(ROMFSIMG) $(ZROMFSIMG)
	#mkimage -A mips -O linux -T multi -C lzma -a 0x80060000 -e 0x80060000 -n "Accelerated Concepts Linux" -d $(LZKERNEL):$(ZROMFSIMG) $(UKERNEL)
	mkimage -A mips -O linux -T kernel -C lzma -a 0x80060000 -e 0x80060000 -n "Linux" -d $(LZKERNEL) $(UKERNEL)
	mkimage -A mips -O linux -T ramdisk -C none -a 0x80800000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	SIZE=`cat $(ROMFSIMG) | wc -c` ; \
	[ "$$SIZE" -le 6160384 ] || { echo "ROOTFS Too big.";exit 1; }; \
	PAD=`expr 6160384 - $$SIZE` ; \
	dd if=/dev/zero of=$(ROMFSIMG).pad bs=1 count=$$PAD
	cat $(ROMFSIMG) $(ROMFSIMG).pad $(UKERNEL) > $(IMAGE)
	gzip -9 $(IMAGE) ; mv $(IMAGE).gz $(IMAGE)
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE)
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
	rm -f $(ROMFSIMG).pad
	if [ -d /tftpboot ]; then \
		cp $(UKERNEL) /tftpboot ; \
		cp $(UROMFSIMG) /tftpboot ; \
		cp $(IMAGE) /tftpboot ; \
	fi

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
