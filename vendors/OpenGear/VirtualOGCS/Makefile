#
# Makefile -- Build instructions for Digi/VirtualDAL
#

ROMFSIMG  = $(IMAGEDIR)/rootfs
ROMFSIMGZ = $(IMAGEDIR)/rootfs.gz
ZIMAGE    = $(IMAGEDIR)/vmlinuz
IMAGE     = $(IMAGEDIR)/image.bin

VENDOR_ROMFS_DIR = $(ROOTDIR)/vendors/OpenGear

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \
    sercon,c,5,1 \
	serial/vt1,c,4,1 \
	serial/port1,c,4,64

FLASH_DEVICES = 

all:

clean: image.clean
	rm -f grub.cfg
	rm -f mksquashfs

romfs: grub.cfg romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -p 555 /etc/mkffs
	$(ROMFSINST) -p 555 /bin/update-grub
	$(ROMFSINST) -d /usr/lib/grub/grub.cfg
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	echo "OpenGear Virtual OGCS" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -A "cons:" -a "cons:unknown:/bin/agetty ttyS0 115200" /etc/inittab
	make -f ../common.mk romfs

grub.cfg: grub.cfg.in Makefile
	sed 's/BOOT_VER[ 	]*=.*$$/BOOT_VER="$(VERSIONPKG)"/g' < $< > $@

romfs.post:: romfs.cleanup
	$(ROMFSINST) -R $(ROMFSDIR)/usr/share/doc
	$(ROMFSINST) -R $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale
	mkdir -p $(ROMFSDIR)/usr/share/locale
	make -f ../common.mk romfs.post

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.tag image.copy
	gzip -9 -c -f $(ROMFSIMG) > $(ROMFSIMGZ)
	./mkdisk
	cp VirtualOGCS.vmx $(IMAGEDIR)/VirtualOGCS-$(VERSIONPKG)/.
	cp VirtualOGCS.vmdk $(IMAGEDIR)/VirtualOGCS-$(VERSIONPKG)/.
	cp run-qemu.sh $(IMAGEDIR)/VirtualOGCS-$(VERSIONPKG)/.
	cp run-vmplayer.sh $(IMAGEDIR)/VirtualOGCS-$(VERSIONPKG)/.
	rm -f $(IMAGEDIR)/VM.tar*
	tar cvCf $(IMAGEDIR) $(IMAGEDIR)/VM.tar VirtualOGCS-$(VERSIONPKG)
	gzip -9 $(IMAGEDIR)/VM.tar

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak

