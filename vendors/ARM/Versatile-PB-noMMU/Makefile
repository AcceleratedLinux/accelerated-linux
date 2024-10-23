#
# Makefile -- Build instructions for ARM/Versatile-PB (with MMU disabled)
#

ROMFSIMG  = $(IMAGEDIR)/rootfs.bin
UROMFSIMG = $(IMAGEDIR)/urootfs.bin
ZIMAGE    = $(IMAGEDIR)/zImage
UIMAGE    = $(IMAGEDIR)/uImage
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
	usr usr/sbin usr/var \
	var

all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean:
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done

romfs:
	[ -d $(ROMFSDIR) ] || rm -f $(ROMFSDIR)
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) ../../Generic/romfs /
	$(ROMFSINST) ../../Generic/httpd /home/httpd
	$(ROMFSINST) ../../Generic/big/inittab /etc/inittab
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /bin/init /init
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	echo "ARM/Versatile-PB" > $(ROMFSDIR)/etc/issue
	echo "versatile" > $(ROMFSDIR)/etc/default/hostname

image:
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)

