#
# Makefile -- Build instructions for Compex/WPQ618
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
KERNEL    = $(IMAGEDIR)/kernel
IMAGE     = $(IMAGEDIR)/image.bin
ITBRAMIMG = $(IMAGEDIR)/wpq618.itb
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
	usr usr/sbin usr/var \
	var

DEVICES = \
	mem,c,1,1	kmem,c,1,2	null,c,1,3 \
	zero,c,1,5	random,c,1,8	urandom,c,1,9 \
	tty,c,5,0	console,c,5,1

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
	echo "Compex/WPQ618" > $(ROMFSDIR)/etc/issue
	echo "wpq618" > $(ROMFSDIR)/etc/default/hostname

./mksquashfs:
	CC=$(HOSTCC) CFLAGS=$(HOSTCFLAGS) EXTRA_CFLAGS= make -C $(ROOTDIR)/user/squashfs-new/squashfs-tools mksquashfs
	ln -fs $(ROOTDIR)/user/squashfs-new/squashfs-tools/mksquashfs .

image: ./mksquashfs
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR) ; \
	$$mksquashfs . $(ROMFSIMG) -all-root -noappend
	# FIXME: end of rootfs being corrupted at load time?
	dd if=/dev/zero bs=1024 count=1024 >> $(ROMFSIMG)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/dts/qcom/ipq6018-cp01-c1.dtb $(IMAGEDIR)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm64/boot/Image $(KERNEL)
	cp wpq618.its $(IMAGEDIR)
	( cd $(IMAGEDIR) ; mkimage -f wpq618.its $(ITBRAMIMG) )
	rm -f $(IMAGEDIR)/wpq618.its
	-cp $(ITBRAMIMG) /tftpboot/

