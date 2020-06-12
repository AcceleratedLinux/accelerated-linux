E2FSPROGS_VERSION = 1.45.5
URL = http://downloads.sourceforge.net/e2fsprogs/e2fsprogs-$(E2FSPROGS_VERSION).tar.gz

CONFOPTS += --disable-defrag
CONFOPTS += --disable-tls

ifdef CONFIG_USER_UTIL_LINUX_LIBBLKID
CONFOPTS += --disable-libblkid
endif
ifdef CONFIG_USER_UTIL_LINUX_LIBUUID
CONFOPTS += --disable-libuuid
endif

AUTOMAKE_ROMFS = e2fsprogs_romfs

include $(ROOTDIR)/tools/automake.inc

e2fsprogs_romfs:
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_CHATTR build/e2fsprogs*install/bin/chattr /bin/chattr
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_LSATTR build/e2fsprogs*install/bin/lsattr /bin/lsattr
ifndef CONFIG_USER_UTIL_LINUX_LIBUUID
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_UUIDGEN build/e2fsprogs*install/bin/uuidgen /bin/uuidgen
endif
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_BADBLOCKS build/e2fsprogs*install/sbin/badblocks /sbin/badblocks
ifndef CONFIG_USER_UTIL_LINUX_LIBBLKID
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_BLKID build/e2fsprogs*install/sbin/blkid /sbin/blkid
endif
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_DEBUGFS build/e2fsprogs*install/sbin/debugfs /sbin/debugfs
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_DUMPE2FS build/e2fsprogs*install/sbin/dumpe2fs /sbin/dumpe2fs
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2FREEFRAG build/e2fsprogs*install/sbin/e2freefrag /sbin/e2freefrag
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2FSCK build/e2fsprogs*install/sbin/e2fsck /sbin/e2fsck
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2IMAGE build/e2fsprogs*install/sbin/e2image /sbin/e2image
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2LABEL build/e2fsprogs*install/sbin/e2label /sbin/e2label
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2UNDO build/e2fsprogs*install/sbin/e2undo /sbin/e2undo
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E4CRYPT build/e2fsprogs*install/sbin/e4crypt /sbin/e4crypt
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_FILEFRAG build/e2fsprogs*install/sbin/filefrag /sbin/filefrag
ifndef CONFIG_USER_UTIL_LINUX_LIBBLKID
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_FINDFS build/e2fsprogs*install/sbin/findfs /sbin/findfs
endif
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_FSCK build/e2fsprogs*install/sbin/fsck /sbin/fsck
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_LOGSAVE build/e2fsprogs*install/sbin/logsave /sbin/logsave
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_MKLOST_FOUND build/e2fsprogs*install/sbin/mklost+found /sbin/mklost+found
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_RESIZE2FS build/e2fsprogs*install/sbin/resize2fs /sbin/resize2fs
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_TUNE2FS build/e2fsprogs*install/sbin/tune2fs /sbin/tune2fs
ifndef CONFIG_USER_UTIL_LINUX_LIBUUID
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_UUIDD build/e2fsprogs*install/sbin/uuidd /sbin/uuidd
endif
	#
	# Hard links for all of the specific fsck.* tools
	#
	$(ROMFSINST) build/e2fsprogs*install/sbin/fsck.ext2 /sbin/fsck.ext2
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_FSCK_EXT3 -l /sbin/fsck.ext2 /sbin/fsck.ext3
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_FSCK_EXT4 -l /sbin/fsck.ext2 /sbin/fsck.ext4
	if [ "$(CONFIG_USER_E2FSPROGS_FSCK_EXT2)" = "" ] ; then \
		rm -f $(ROMFSDIR)/sbin/fsck.ext2 ; \
	fi
	#
	# Hard links for all of the specific mkfs.* tools
	#
	$(ROMFSINST) build/e2fsprogs*install/sbin/mke2fs /sbin/mke2fs
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_MKFS_EXT2 -l /sbin/mke2fs /sbin/mkfs.ext2
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_MKFS_EXT3 -l /sbin/mke2fs /sbin/mkfs.ext3
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_MKFS_EXT4 -l /sbin/mke2fs /sbin/mkfs.ext4
	if [ "$(CONFIG_USER_E2FSPROGS_MKE2FS)" = "" ] ; then \
		rm -f $(ROMFSDIR)/sbin/mke2fs ; \
	fi

