VERSION = v2.0.2
URL = git://git.infradead.org/mtd-utils.git
METHOD = git

AUTORECONF = ./autogen.sh
AUTOMAKE_ROMFS = my_romfs

CONFOPTS-$(CONFIG_LIB_LIBLZO) += --without-lzo
CONFOPTS-$(CONFIG_USER_MTD_UTILS_MKFSJFFS2) += --without-jffs
CONFOPTS-$(CONFIG_USER_MTD_UTILS_UBIATTACH) += --without-ubifs
CONFOPTS += $(CONFOPTS-)
CONFOPTS += --without-xattr
CONFOPTS += --disable-tests

include $(ROOTDIR)/tools/automake.inc

#
# We want to be able to build a host based mkfs.jffs2 binary.
# So add an extra compile target to build this. This is really for
# backward compatabity for some of the older targets.
#
HOST_MKFS_JFFS2 = $(CURDIR)/build/mkfs.jffs2
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/mkfs.jffs2.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/compr.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/compr_zlib.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/compr_lzo.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/compr_rtime.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/jffsX-utils/rbtree.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/lib/libcrc32.c
HOST_MKFS_JFFS2_SRC += $(CURDIR)/build/mtd-utils/lib/dictionary.c
HOST_CFLAGS += -I$(CURDIR)/build/mtd-utils/include
HOST_CFLAGS += -I$(CURDIR)/build/mtd-utils/jffsX-utils
HOST_CFLAGS += -DWITHOUT_XATTR
HOST_CFLAGS += -DWITHOUT_LZO 
HOST_CFLAGS += -D_GNU_SOURCE

$(HOST_MKFS_JFFS2):
	echo "Building host binary for mkfs.jffs2 tool"
	gcc $(HOST_CFLAGS) -o $@ $(HOST_MKFS_JFFS2_SRC) -lz

complete_target: $(HOST_MKFS_JFFS2)

MTDBINS = $(CURDIR)/build/mtd-utils-install/sbin

my_romfs:
	mkdir -p $(ROMFSDIR)/sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_ERASE $(MTDBINS)/flash_erase /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_ERASEALL $(MTDBINS)/flash_eraseall /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_LOCK $(MTDBINS)/flash_lock /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UNLOCK $(MTDBINS)/flash_unlock /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FLASH_OTP_INFO $(MTDBINS)/flash_otp_info /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FLASH_OTP_DUMP $(MTDBINS)/flash_otp_dump /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FLASH_OTP_LOCK $(MTDBINS)/flash_otp_lock /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FLASH_OTP_WRITE $(MTDBINS)/flash_otp_write /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FLASHCP $(MTDBINS)/flashcp /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FTL_CHECK $(MTDBINS)/ftl_check /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_FTL_FORMAT $(MTDBINS)/ftl_format /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_JFFS2DUMP $(MTDBINS)/jffs2dump /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_JFFS2READER $(MTDBINS)/jffs2reader /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_MKFSJFFS2 $(MTDBINS)/mkfs.jffs2 /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_MTD_DEBUG $(MTDBINS)/mtd_debug /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_MTDINFO $(MTDBINS)/mtdinfo /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_MTDPART $(MTDBINS)/mtdpart /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_NFTLDUMP $(MTDBINS)/nftldump /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_NFTL_FORMAT $(MTDBINS)/nftl_format /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_NANDDUMP $(MTDBINS)/nanddump /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_NANDTEST $(MTDBINS)/nandtest /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_NANDWRITE $(MTDBINS)/nandwrite /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_DOC_LOADBIOS $(MTDBINS)/doc_loadbios /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_DOCFDISK $(MTDBINS)/docfdisk /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_RFDDUMP $(MTDBINS)/rfddump /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_RFDFORMAT $(MTDBINS)/rfdformat /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_RECV_IMAGE $(MTDBINS)/recv_image /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_SERVE_IMAGE $(MTDBINS)/server_image /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_SUMTOOL $(MTDBINS)/sumtool /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_MKFSUBIFS $(MTDBINS)/mkfs.ubifs /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIATTACH $(MTDBINS)/ubiattach /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIBLOCK $(MTDBINS)/ubiblock /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBICRC32 $(MTDBINS)/ubicrc32 /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIDETACH $(MTDBINS)/ubidetach /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIFORMAT $(MTDBINS)/ubiformat /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIMKVOL $(MTDBINS)/ubimkvol /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBINFO $(MTDBINS)/ubinfo /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBINIZE $(MTDBINS)/ubinize /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIRENAME $(MTDBINS)/ubirename /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIRMVOL $(MTDBINS)/ubirmvol /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIRSVOL $(MTDBINS)/ubirsvol /sbin
	$(ROMFSINST) -e CONFIG_USER_MTD_UTILS_UBIUPDATEVOL $(MTDBINS)/ubiupdatevol /sbin

