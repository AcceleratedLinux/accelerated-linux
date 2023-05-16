
ZIMAGE    = $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage
DTB	  = $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/kirkwood-im7200.dtb

include $(ROOTDIR)/vendors/OpenGear/opengear.mk

romfs::
	$(ROMFSINST) -A "cons:" -a "cons:unknown:/bin/agetty ttyS0 115200" \
		/etc/default/inittab
