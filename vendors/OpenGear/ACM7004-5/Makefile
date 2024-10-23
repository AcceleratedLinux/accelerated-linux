
ZIMAGE    = $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage
DTB	  = $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/marvell/armada-370-acm7004-5.dtb

include $(ROOTDIR)/vendors/OpenGear/opengear.mk

romfs::
	$(ROMFSINST) -A "cons:" -a "cons:unknown:/bin/agetty ttyS0 115200" \
		/etc/default/inittab
