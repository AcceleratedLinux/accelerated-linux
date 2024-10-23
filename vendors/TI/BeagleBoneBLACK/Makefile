#
# Makefile -- Build instructions for the BeagleBone-BLACK
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
IMAGE     = $(IMAGEDIR)/image.bin
UKERNEL   = $(IMAGEDIR)/ukernel.bin

DIRS = 

ROMFS_DIRS = \
	bin \
	dev \
	etc etc/config etc/default \
	home home/httpd home/httpd/cgi-bin \
	lib \
	mnt \
	proc \
	sbin \
	sys \
	tmp \
	usr usr/bin usr/sbin usr/var \
	var

DEVICES = null,c,1,3	zero,c,1,5	console,c,5,1

all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean:
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	for i in $(DEVICES); do \
		touch $(ROMFSDIR)/dev/@$$i; \
	done
	$(ROMFSINST) ../../Generic/romfs /
	$(ROMFSINST) ../../Generic/httpd /home/httpd
	$(ROMFSINST) ../../Generic/big/inittab /etc/inittab
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/default/start
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	echo "BeagleBoneBLACK" > $(ROMFSDIR)/etc/issue

./mksquashfs:
	CC=$(HOSTCC) CFLAGS=$(HOSTCFLAGS) EXTRA_CFLAGS= make -C $(ROOTDIR)/user/squashfs-new/squashfs-tools mksquashfs
	ln -fs $(ROOTDIR)/user/squashfs-new/squashfs-tools/mksquashfs .

image: ./mksquashfs
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR) ; \
	$$mksquashfs . $(ROMFSIMG) -all-root -noappend
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)
	cat $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/dts/ti/omap/am335x-boneblack.dtb >> $(ZIMAGE)
	mkimage -A arm -O linux -T kernel -C none -a 0x81000000 -e 0x81000000 -n "Linux-4.x" -d $(ZIMAGE) $(UKERNEL)
	mkimage -A arm -O linux -T ramdisk -C none -a 0x88080000 -n "ramdisk" -d $(ROMFSIMG) $(UROMFSIMG)
	if [ -d /tftpboot ]; then \
		cp $(UKERNEL) /tftpboot ; \
		cp $(UROMFSIMG) /tftpboot ; \
	fi

