#
# Intel/MinnowBoard -- build image for the Intel based MinnowBoard
#

ZIMAGE    = $(IMAGEDIR)/bzImage
ROMFSIMG  = $(IMAGEDIR)/initrd
ROMFSIMGZ = $(IMAGEDIR)/initrd.gz
ISOFILE   = $(IMAGEDIR)/$(CONFIG_PRODUCT).iso
ISODIR    = $(ROOTDIR)/iso

DIRS = 

ROMFS_DIRS = \
	bin \
	dev \
	etc \
	home home/httpd home/httpd/cgi-bin \
	lib \
	mnt \
	proc \
	sys \
	tmp \
	usr usr/bin usr/lib usr/var \
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
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) ../../Generic/romfs /
	$(ROMFSINST) ../../Generic/httpd /home/httpd
	$(ROMFSINST) ../../Generic/big/inittab /etc/inittab
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) /etc/motd
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	echo "MinnowBoard" > $(ROMFSDIR)/etc/issue
	echo "minnow" > $(ROMFSDIR)/etc/hostname

romfs.post::
	$(ROMFSINST) -R /share
	$(ROMFSINST) -R /bin/iconv
	$(ROMFSINST) -R /lib/pkgconfig
	$(ROMFSINST) -R /lib/gio
	$(ROMFSINST) -R /lib/dbus-1.0
	$(ROMFSINST) -R /var/run

./mksquashfs:
	CC=$(HOSTCC) CFLAGS=$(HOSTCFLAGS) EXTRA_CFLAGS= make -C $(ROOTDIR)/user/squashfs-new/squashfs-tools mksquashfs
	ln -fs $(ROOTDIR)/user/squashfs-new/squashfs-tools/mksquashfs .

image: ./mksquashfs
	cp $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage $(ZIMAGE)
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR) ; \
	$$mksquashfs . $(ROMFSIMG) -all-root -noappend
	gzip -9 -f $(ROMFSIMG)
	mkdir -p $(ISODIR)/boot/isolinux
	cp isolinux.bin $(ISODIR)/boot/isolinux/
	cp ldlinux.c32 $(ISODIR)/boot/isolinux/
	cp boot.msg $(ISODIR)/boot/isolinux/
	cp isolinux.cfg $(ISODIR)/boot/isolinux/
	cp $(ZIMAGE) $(ISODIR)/boot/isolinux/
	cp $(ROMFSIMGZ) $(ISODIR)/boot/isolinux/initrd.gz
	mkisofs -o $(ISOFILE) -b boot/isolinux/isolinux.bin \
		-c boot/isolinux/boot.cat -no-emul-boot \
		-boot-load-size 4 -boot-info-table $(ISODIR)
	isohybrid $(ISOFILE)

#include $(ROOTDIR)/vendors/config/config.dev
