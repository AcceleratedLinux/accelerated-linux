#
# Makefile -- Build instructions for AcceleratedConcepts/porter
#

ROMFSIMG  = $(IMAGEDIR)/rootfs
ZIMAGE    = $(IMAGEDIR)/vmlinuz
IMAGE     = $(IMAGEDIR)/image.bin
DISKRAW   = $(IMAGEDIR)/disk.raw
DISKQCOW2 = $(IMAGEDIR)/disk.qcow2

ROMFS_DIRS = $(DEFAULT_ROMFS_DIRS)
ROMFS_DIRS += boot/grub etc etc/config etc/inittab.d home mnt proc sys tmp usr/var var opt

# Minimal set of GRUB modules. This saves megabytes of squashfs.
# Ideally, we would have a separate rootfs that is only used to setup the disk
# and not include these in the normal firmware image.
GRUB_FILES   = moddep.lst diskboot.img boot.img kernel.img lzma_decompress.img
GRUB_MODULES = linux normal vbe video video_fb boot relocator mmap \
               terminfo terminal serial extcmd gettext \
               biosdisk fshelp gcry_crc \
               part_msdos squash4 ext2 gzio crypto xzio lzopio \
               loopback configfile test loadenv

DEVICES += $(DEVICE_PTY_64) \
	ledman,c,126,0 \

FLASH_DEVICES = \
	disk,b,8,1

all:

clean: image.clean

romfs: romfs_dev romfs.dirs romfs.default romfs.rc romfs.version romfs.cryptokey
	$(ROMFSINST) -s lib /lib64
	$(ROMFSINST) /etc/default/start
	$(ROMFSINST) /etc/fw_env.config
	$(ROMFSINST) -s /var/run/console /etc/inittab.d/console
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /var/run/syslog.conf -e CONFIG_USER_SYSKLOGD /etc/syslog.conf
	$(ROMFSINST) -p 555 /bin/sim
	$(ROMFSINST) -p 555 /bin/btnmon
	$(ROMFSINST) -p 555 /etc/grub-install
	$(ROMFSINST) /boot/grub/grub.cfg
	$(ROMFSINST) /boot/grub/grub_embedded.cfg
	$(ROMFSINST) -p 555 /etc/mkffs
	# Directories to mount/swap images for booting from a loopback device
	mkdir -p $(ROMFSDIR)/boot/initrd
	mkdir -p $(ROMFSDIR)/mnt/disk
	mkdir -p $(ROMFSDIR)/mnt/image
	mkdir -p $(ROMFSDIR)/.oldroot # Required for libvirtd/lxc mounting of read-only fs
	$(ROMFSINST) -p 555 /bin/init-loop
	$(ROMFSINST) -s /bin/init /bin/init-lte
	$(ROMFSINST) $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage /boot/vmlinuz
	echo "Accelerated Concepts Porter" > $(ROMFSDIR)/etc/issue

romfs.post:: romfs.cleanup
	# We munge the grub installation to allow for read-only root
	mkdir -p $(ROMFSDIR)/boot/grub/i386-pc
	mv $(foreach f,$(GRUB_FILES),$(ROMFSDIR)/usr/lib/grub/i386-pc/$(f)) $(ROMFSDIR)/boot/grub/i386-pc/
	mv $(foreach f,$(GRUB_MODULES),$(ROMFSDIR)/usr/lib/grub/i386-pc/$(f).mod*) $(ROMFSDIR)/boot/grub/i386-pc/
	rm -rf $(ROMFSDIR)/usr/lib/grub
	rm -rf $(ROMFSDIR)/usr/lib/python3.4/test
	rm -rf $(ROMFSDIR)/usr/share/doc
	rm -rf $(ROMFSDIR)/usr/include
	$(ROMFSINST) -R /usr/share/locale

image.x86.bin:
	cp $(ROMFSIMG) $(IMAGE)

disk.raw:
	./mkdisk ${IMAGE} ${ZIMAGE} ${DISKRAW} 120

disk.qcow2: disk.raw
	qemu-img convert -f raw -O qcow2 ${DISKRAW} ${DISKQCOW2}

image: image.configs image.dir image.x86.zimage image.squashfs image.x86.bin image.tag image.copy disk.qcow2

include $(ROOTDIR)/vendors/config/config.dev
include $(ROOTDIR)/vendors/AcceleratedConcepts/vendor.mak
