#
# This Makefile fragment incorporates all the declarations and rules
# common to products for this vendor.
#
# It is included by <product>/Makefile 
#

.PHONY: image.clean image.tag image.copy image.dir image.linuz image.arm.zimage image.cramfs
.PHONY: image.sh.mot image.sh.abs image.flash image.configs
.PHONY: romfs.dirs romfs.symlinks romfs.default romfs.recover romfs.rc romfs.version

# Note: These must all be used only in romfs.post::
.PHONY: romfs.no-ixp400-modules romfs.ixp425-microcode romfs.ixp425-boot romfs.nooom

# Stop dd from being so noisy
DD=dd 2>/dev/null

# Override this if necessary
FLASH_DEVICES ?= \
	boot,c,90,0 \
	ethmac,c,90,0 \
	bootarg,c,90,0 \
	config,c,90,2 \
	image,c,90,4 \
	all,c,90,6

SGKEY ?= $(HOME)/keys/sgkey.pem

COMMON_ROMFS_DIRS =
ifdef CONFIG_SYSFS
COMMON_ROMFS_DIRS += sys
endif
ifdef CONFIG_PROC_FS
COMMON_ROMFS_DIRS += proc
endif
ifdef CONFIG_USER_UDEV
COMMON_ROMFS_DIRS += lib/udev/devices/pts lib/udev/devices/flash
else
COMMON_ROMFS_DIRS += dev dev/pts dev/flash
endif

# You probably want to add this to ROMFS_DIRS
DEFAULT_ROMFS_DIRS := $(COMMON_ROMFS_DIRS) \
	bin sbin etc/config etc/init.d lib/modules var \
	home/httpd/cgi-bin usr/bin usr/sbin

FACTORY_ROMFS_DIRS := $(COMMON_ROMFS_DIRS) \
	bin etc lib mnt usr var

# Generate list of processes to kill during a netflash upgrade
NETFLASH_KILL_LIST_FILE ?= etc/netflash_kill_list.txt

# Processes that are killed on all platforms
NETFLASH_KILL_LIST_y ?= 
NETFLASH_KILL_LIST_$(CONFIG_PROP_AUTHD_AUTHD)			+= authd
NETFLASH_KILL_LIST_$(CONFIG_USER_ZEBRA_BGPD_BGPD)		+= bgpd
NETFLASH_KILL_LIST_$(CONFIG_USER_CLAMAV_CLAMAV)			+= clamav
NETFLASH_KILL_LIST_$(CONFIG_USER_CLAMAV_CLAMD)			+= clamd
NETFLASH_KILL_LIST_$(CONFIG_USER_CLAMAV_CLAMSMTP)		+= clamsmtpd
NETFLASH_KILL_LIST_$(CONFIG_USER_CRON_CRON)			+= cron
NETFLASH_KILL_LIST_$(CONFIG_USER_DHCPD_DHCPD)			+= dhcpd
NETFLASH_KILL_LIST_$(CONFIG_USER_DHCP_ISC_SERVER_DHCPD)		+= dhcpd
NETFLASH_KILL_LIST_$(CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY)	+= dhcrelay
NETFLASH_KILL_LIST_$(CONFIG_USER_DNSMASQ_DNSMASQ)		+= dnsmasq
NETFLASH_KILL_LIST_$(CONFIG_USER_DNSMASQ2_DNSMASQ2)		+= dnsmasq
NETFLASH_KILL_LIST_$(CONFIG_USER_FLATFSD_FLATFSD)		+= flatfsd
NETFLASH_KILL_LIST_$(CONFIG_USER_FROX_FROX)			+= frox
NETFLASH_KILL_LIST_$(CONFIG_USER_SSH_SSHKEYGEN)			+= gen-keys
NETFLASH_KILL_LIST_$(CONFIG_PROP_HTTPSCERTGEN_HTTPSCERTGEN)	+= https-certgen
NETFLASH_KILL_LIST_$(CONFIG_USER_IDB_IDB)			+= idb
NETFLASH_KILL_LIST_$(CONFIG_USER_BUSYBOX_BUSYBOX)		+= klogd
NETFLASH_KILL_LIST_$(CONFIG_PROP_NFLOGD_NFLOGD)			+= nflogd
NETFLASH_KILL_LIST_$(CONFIG_USER_NTPD_NTPD)			+= ntpd
NETFLASH_KILL_LIST_$(CONFIG_PROP_AUTHD_AUTHD)			+= proxy80
NETFLASH_KILL_LIST_$(CONFIG_USER_ZEBRA_RIPD_RIPD)		+= ripd
NETFLASH_KILL_LIST_$(CONFIG_USER_SIPROXD_SIPROXD)		+= siproxd
NETFLASH_KILL_LIST_$(CONFIG_PROP_AUTHD_AUTHD)			+= sgadnsd
NETFLASH_KILL_LIST_$(CONFIG_USER_SNMPD_SNMPD)			+= snmpd
NETFLASH_KILL_LIST_$(CONFIG_USER_NETSNMP_SNMPD)			+= snmpd
NETFLASH_KILL_LIST_$(CONFIG_USER_SNORT_SNORT)			+= snort
NETFLASH_KILL_LIST_$(CONFIG_USER_SNORT_SNORT)			+= snort-inline
NETFLASH_KILL_LIST_$(CONFIG_USER_SQUID_SQUID)			+= squid
NETFLASH_KILL_LIST_$(CONFIG_USER_SSH_SSHKEYGEN)			+= ssh-keygen
NETFLASH_KILL_LIST_$(CONFIG_PROP_STATSD_STATSD)			+= statsd
NETFLASH_KILL_LIST_$(CONFIG_USER_BUSYBOX_BUSYBOX)		+= syslogd
NETFLASH_KILL_LIST_$(CONFIG_USER_LINUXIGD_LINUXIGD)		+= upnpd
NETFLASH_KILL_LIST_$(CONFIG_USER_ZEBRA_ZEBRA_ZEBRA)		+= zebra


image.clean:
	rm -f mkcramfs mksquashfs mksquashfs7z
	rm -f addr.txt

mkcramfs: $(ROOTDIR)/user/cramfs/mkcramfs.c
	$(HOSTCC) -o $@ $< -lz

.PHONY: mksquashfs
mksquashfs:
	make -C $(ROOTDIR)/user/squashfs/squashfs-tools mksquashfs
	ln -fs $(ROOTDIR)/user/squashfs/squashfs-tools/mksquashfs .

.PHONY: mksquashfs7z
mksquashfs7z:
	make -C $(ROOTDIR)/user/squashfs/squashfs-tools mksquashfs7z
	ln -fs $(ROOTDIR)/user/squashfs/squashfs-tools/mksquashfs7z .

# Tags an image with vendor,product,version and adds the checksum
image.tag:
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE)
ifdef CONFIG_USER_NETFLASH_CRYPTO
	if [ -f $(SGKEY) ] ; then \
		$(ROOTDIR)/prop/cryptimage/cryptimage -k $(SGKEY) -f $(IMAGE) ; \
		printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE) ; \
	fi
endif
ifdef CONFIG_USER_NETFLASH_SHA256
	cat $(IMAGE) | $(ROOTDIR)/user/netflash/sha256sum -b >> $(IMAGE)
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE)
endif
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)

image.size.zimage:
	@if [ `cat $(ZIMAGE) | wc -c` -gt $(ZIMAGESIZE) ]; then \
		echo "Error: $(ZIMAGE) size is greater than $(ZIMAGESIZE)"; \
		exit 1; \
	fi

image.size:
	@if [ `cat $(IMAGE) | wc -c` -gt $(IMAGESIZE) ]; then \
		echo "Error: $(IMAGE) size is greater than $(IMAGESIZE)"; \
		exit 1; \
	fi

image.copy:
	@set -e; for i in $(IMAGE) $(KERNELZ) $(IMAGEDIR)/sh.mot $(IMAGEDIR)/sh.abs; do \
		[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] && continue; \
		[ -f $$i ] || continue; \
		echo cp $$i /tftpboot; \
		cp $$i /tftpboot; \
	done
	@[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || ( echo cp $(IMAGE) /tftpboot/$(CONFIG_PRODUCT).bin; cp $(IMAGE) /tftpboot/$(CONFIG_PRODUCT).bin )

image.dir:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	rm -rf $(ROMFSDIR)/man[1-9]
	rm -rf $(ROMFSDIR)/usr/man
	rm -rf $(ROMFSDIR)/share

# Create ZIMAGE as vmlinux -> objcopy -> $(ZIMAGE)
image.linuz:
	$(CROSS)objcopy -O binary $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGEDIR)/linux.bin
	gzip -c -9 < $(IMAGEDIR)/linux.bin >$(ZIMAGE)

# Create ZIMAGE as vmlinux -> objcopy (include bss) -> $(ZIMAGE)
image.bsslinuz:
	$(CROSS)objcopy -O binary --set-section-flags .bss=load,contents $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGEDIR)/linux.bin
	gzip -c -9 < $(IMAGEDIR)/linux.bin >$(ZIMAGE)

# Create ZIMAGE as arm/arm/boot/zImage
image.arm.zimage:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(ZIMAGE)

image.mips.zimage:
	gzip -c -9 < $(ROOTDIR)/$(LINUXDIR)/arch/mips/boot/vmlinux.bin >$(ZIMAGE)

image.i386.zimage:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/i386/boot/bzImage $(ZIMAGE)

image.mips.vmlinux:
	cp $(ROOTDIR)/$(LINUXDIR)/vmlinux $(VMLINUX)

# Create a 16MB file for testing
image.16mb:
	dd if=/dev/zero of=$(ROMFSDIR)/16MB bs=1000000 count=16

image.16mb.rm:
	rm -f $(ROMFSDIR)/16MB

image.cramfs: mkcramfs
	./mkcramfs -z -r $(ROMFSDIR) $(ROMFSIMG)

image.squashfs: mksquashfs
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR); \
	$$mksquashfs . $(ROMFSIMG) -all-root -noappend $(SQUASH_ENDIAN)

image.squashfs7z: mksquashfs7z
	rm -f $(ROMFSIMG); mksquashfs7z=`pwd`/mksquashfs7z; cd $(ROMFSDIR); \
	$$mksquashfs7z . $(ROMFSIMG) -all-root -noappend $(SQUASH_ENDIAN)

image.romfs:
	rm -f $(ROMFSIMG)
	genromfs -f $(ROMFSIMG) -d $(ROMFSDIR)

# Create (possibly) mbr + cramfs + zimage/linuz
image.bin:
	cat $(MBRIMG) $(ROMFSIMG) $(SHIM) $(ZIMAGE) >$(IMAGE)

addr.txt: $(ROOTDIR)/$(LINUXDIR)/vmlinux
	$(CROSS)nm $(ROOTDIR)/$(LINUXDIR)/vmlinux | \
		grep " __bss_start$$" | \
		cut -d' ' -f1 | xargs printf "0x%s\n" >$@
	@echo ROMFS@`cat $@`

image.sh.mot: addr.txt
	@ADDR=`cat addr.txt`; \
        $(CROSS)objcopy --add-section=.romfs=$(ROMFSIMG) \
          --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
          --set-section-flags=.romfs=alloc,load,data   \
		  -O srec \
          $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGEDIR)/sh.mot

image.sh.abs: addr.txt
	ADDR=`cat addr.txt`; \
        $(CROSS)objcopy --add-section=.romfs=$(ROMFSIMG) \
          --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
          --set-section-flags=.romfs=alloc,load,data   \
          $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGEDIR)/sh.abs

image.flash:
	[ ! -f $(ROOTDIR)/boot/boot.bin ] || $(MAKE) vendor_flashbin

image.configs:
	@rm -rf configs
	@mkdir -p configs
	cp $(ROOTDIR)/.config configs/config.device
	cp $(ROOTDIR)/config/.config configs/config.vendor-$(patsubst linux-%,%,$(CONFIG_LINUXDIR)) 
	cp $(ROOTDIR)/$(CONFIG_LINUXDIR)/.config configs/config.$(CONFIG_LINUXDIR)
	-cp $(ROOTDIR)/$(CONFIG_LIBCDIR)/.config configs/config.$(CONFIG_LIBCDIR)
	tar czf $(IMAGEDIR)/configs.tar.gz configs
	@rm -rf configs
	
romfs.dirs:
	mkdir -p $(ROMFSDIR)
	@for i in $(ROMFS_DIRS); do \
		mkdir -p $(ROMFSDIR)/$$i; \
	done

romfs.symlinks:
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /var/tmp/log /dev/log
	[ -d $(ROMFSDIR)/sbin ] || $(ROMFSINST) -s bin /sbin

# Override this if necessary
VENDOR_ROMFS_DIR ?= ..

romfs.default:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs /
	chmod 755 $(ROMFSDIR)/etc/default/dhcpcd-change
	chmod 755 $(ROMFSDIR)/etc/default/ip-*
ifeq ($(CONFIG_LIBCDIR),glibc)
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/nsswitch.conf /etc/nsswitch.conf
endif
ifdef CONFIG_USER_NETFLASH_NETFLASH
	rm -f $(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE)
	for p in $(sort $(NETFLASH_KILL_LIST_y)) ; do echo $$p >> $(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE); done
endif

romfs.recover:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs.recover /

romfs.factory:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs/etc/services /etc/services

# This is the old way. Just install the static rc file
romfs.rc.static:
	if [ -f rc-$(CONFIG_LANGUAGE) ]; then \
		$(ROMFSINST) /etc/rc-$(CONFIG_LANGUAGE) /etc/rc; \
	else \
		$(ROMFSINST) /etc/rc; \
	fi
	[ ! -f filesystems ] || $(ROMFSINST) /etc/filesystems

# This is the new way. Generate it dynamically.
romfs.rc:
	if [ -f $(ROOTDIR)/prop/configdb/rcgen ]; then \
		[ ! -f rc-$(CONFIG_LANGUAGE) ] || ( echo "*** Error: Static rc-$(CONFIG_LANGUAGE) file exists, but trying to use dynamic rc file"; exit 1 ) ; \
		[ ! -f rc ] || echo "*** Warning: Static rc file exists, but using dynamic rc file" ; \
		tclsh $(ROOTDIR)/prop/configdb/rcgen $(ROOTDIR) >$(ROMFSDIR)/etc/rc ; \
	else \
		$(ROMFSINST) /etc/rc; \
	fi
	[ ! -f filesystems ] || $(ROMFSINST) /etc/filesystems

romfs.inittab:
	[ ! -f inittab ] || echo "*** Warning: Static inittab file exists, but using dynamic inittab file"
	$(ROMFSINST) -e CONFIG_USER_INETD_INETD -a "inet:unknown:/bin/inetd" /etc/inittab

romfs.no-ixp400-modules:
	rm -f $(ROMFSDIR)/lib/modules/*/kernel/ixp425/ixp400-*/ixp400_*.o

romfs.ixp425-microcode:
	$(ROMFSINST) -e CONFIG_IXP400_LIB_2_0 -d $(ROOTDIR)/modules/ixp425/ixp400-2.0/IxNpeMicrocode.dat /etc/IxNpeMicrocode.dat
	$(ROMFSINST) -e CONFIG_IXP400_LIB_2_1 -d $(ROOTDIR)/modules/ixp425/ixp400-2.1/IxNpeMicrocode.dat /etc/IxNpeMicrocode.dat
	$(ROMFSINST) -e CONFIG_IXP400_LIB_2_4 -d $(ROOTDIR)/modules/ixp425/ixp400-2.4/IxNpeMicrocode.dat /etc/IxNpeMicrocode.dat

romfs.ixp425-boot:
ifneq ($(strip $(BOOTLOADER)),)
	$(ROMFSINST) -d $(BOOTLOADERBIOS) /boot/biosplus.bin
	$(ROMFSINST) -d $(BOOTLOADER) /boot/bootplus.bin
else 
	-$(ROMFSINST) -d $(ROOTDIR)/boot/ixp425/bios.bin /boot/biosplus.bin
	-$(ROMFSINST) -d $(ROOTDIR)/boot/ixp425/boot.bin /boot/bootplus.bin
endif

romfs.boot:
# Skip this whole target for host builds
ifndef HOSTBUILD
ifneq ($(strip $(BOOTLOADER)),)
	$(ROMFSINST) -d $(BOOTLOADER) /boot/boot.bin
else 
	-$(ROMFSINST) -d $(ROOTDIR)/boot/boot.bin /boot/boot.bin
endif
endif

romfs.version:
	echo "$(VERSIONSTR) -- " $(BUILD_START_STRING) > $(ROMFSDIR)/etc/version
	echo "$(HW_VENDOR)/$(HW_PRODUCT)" > $(ROMFSDIR)/etc/hwdetails

romfs.cryptokey:
ifdef CONFIG_USER_NETFLASH_CRYPTO
	if [ -f $(SGKEY) ] ; then \
		openssl rsa -in $(SGKEY) -pubout > $(ROMFSDIR)/etc/publickey.pem ; \
	fi
endif

romfs.nooom:
	[ ! -x $(ROMFSDIR)/bin/no_oom ] || ( ( cd $(ROMFSDIR) && mkdir -p .no_oom ) && for i in `echo ${CONFIG_USER_NOOOM_BINARIES}` ; do [ -x $(ROMFSDIR)/$$i ] && [ x`readlink $(ROMFSDIR)/$$i` != x/bin/no_oom ] && mv $(ROMFSDIR)/$$i $(ROMFSDIR)/.no_oom/`basename $$i` && ln -s /bin/no_oom $(ROMFSDIR)/$$i || "NOTICE: $$i not present in romfs" ; done )

romfs.cleanup:
	$(ROMFSINST) -R /share
	$(ROMFSINST) -R /usr/share/man

romfs.post:: romfs.nooom
CONFIG_USER_BUSYBOX=y
# CONFIG_USER_BUSYBOX_FEDORA_COMPAT is not set
CONFIG_USER_BUSYBOX_LONG_OPTS=y
# CONFIG_USER_BUSYBOX_LFS is not set
# CONFIG_USER_BUSYBOX_PAM is not set
CONFIG_USER_BUSYBOX_BUSYBOX=y
# CONFIG_USER_BUSYBOX_FEATURE_SHOW_SCRIPT is not set
# CONFIG_USER_BUSYBOX_FEATURE_INSTALLER is not set
CONFIG_USER_BUSYBOX_INSTALL_NO_USR=y
# CONFIG_USER_BUSYBOX_SELINUX is not set
# CONFIG_USER_BUSYBOX_FEATURE_CLEAN_UP is not set
CONFIG_USER_BUSYBOX_PLATFORM_LINUX=y
# CONFIG_USER_BUSYBOX_USE_PORTABLE_CODE is not set
# CONFIG_USER_BUSYBOX_STACK_OPTIMIZATION_386 is not set
# CONFIG_USER_BUSYBOX_DEBUG is not set
# CONFIG_USER_BUSYBOX_DEBUG_SANITIZE is not set
# CONFIG_USER_BUSYBOX_UNIT_TEST is not set
# CONFIG_USER_BUSYBOX_WERROR is not set
CONFIG_USER_BUSYBOX_NO_DEBUG_LIB=y
# CONFIG_USER_BUSYBOX_DMALLOC is not set
# CONFIG_USER_BUSYBOX_EFENCE is not set
# CONFIG_USER_BUSYBOX_FEATURE_USE_BSS_TAIL is not set
# CONFIG_USER_BUSYBOX_FLOAT_DURATION is not set
CONFIG_USER_BUSYBOX_FEATURE_BUFFERS_USE_MALLOC=y
# CONFIG_USER_BUSYBOX_FEATURE_BUFFERS_GO_ON_STACK is not set
# CONFIG_USER_BUSYBOX_FEATURE_BUFFERS_GO_IN_BSS is not set
# CONFIG_USER_BUSYBOX_FEATURE_ETC_SERVICES is not set
# CONFIG_USER_BUSYBOX_LOCALE_SUPPORT is not set
# CONFIG_USER_BUSYBOX_UNICODE_SUPPORT is not set
# CONFIG_USER_BUSYBOX_FEATURE_USE_SENDFILE is not set
# CONFIG_USER_BUSYBOX_UNCOMPRESS is not set
# CONFIG_USER_BUSYBOX_GUNZIP is not set
# CONFIG_USER_BUSYBOX_ZCAT is not set
# CONFIG_USER_BUSYBOX_BZCAT is not set
# CONFIG_USER_BUSYBOX_UNLZMA is not set
# CONFIG_USER_BUSYBOX_LZCAT is not set
# CONFIG_USER_BUSYBOX_LZMA is not set
# CONFIG_USER_BUSYBOX_UNXZ is not set
# CONFIG_USER_BUSYBOX_XZCAT is not set
# CONFIG_USER_BUSYBOX_XZ is not set
# CONFIG_USER_BUSYBOX_UNLZOP is not set
# CONFIG_USER_BUSYBOX_LZOPCAT is not set
# CONFIG_USER_BUSYBOX_RPM2CPIO is not set
# CONFIG_USER_BUSYBOX_DATE is not set
# CONFIG_USER_BUSYBOX_UNIX2DOS is not set
# CONFIG_USER_BUSYBOX_UNEXPAND is not set
# CONFIG_USER_BUSYBOX_FACTOR is not set
# CONFIG_USER_BUSYBOX_HOSTID is not set
# CONFIG_USER_BUSYBOX_ID is not set
# CONFIG_USER_BUSYBOX_GROUPS is not set
# CONFIG_USER_BUSYBOX_LINK is not set
# CONFIG_USER_BUSYBOX_SHA1SUM is not set
# CONFIG_USER_BUSYBOX_SHA256SUM is not set
# CONFIG_USER_BUSYBOX_SHA512SUM is not set
# CONFIG_USER_BUSYBOX_SHA3SUM is not set
# CONFIG_USER_BUSYBOX_MKTEMP is not set
# CONFIG_USER_BUSYBOX_NL is not set
# CONFIG_USER_BUSYBOX_NPROC is not set
# CONFIG_USER_BUSYBOX_PASTE is not set
# CONFIG_USER_BUSYBOX_SHRED is not set
# CONFIG_USER_BUSYBOX_SHUF is not set
# CONFIG_USER_BUSYBOX_FSYNC is not set
# CONFIG_USER_BUSYBOX_TEST is not set
# CONFIG_USER_BUSYBOX_TEST1 is not set
# CONFIG_USER_BUSYBOX_TEST2 is not set
# CONFIG_USER_BUSYBOX_TIMEOUT is not set
# CONFIG_USER_BUSYBOX_TOUCH is not set
# CONFIG_USER_BUSYBOX_TR is not set
# CONFIG_USER_BUSYBOX_TRUNCATE is not set
# CONFIG_USER_BUSYBOX_BB_ARCH is not set
# CONFIG_USER_BUSYBOX_UNLINK is not set
# CONFIG_USER_BUSYBOX_BASE64 is not set
# CONFIG_USER_BUSYBOX_FEATURE_VERBOSE is not set
# CONFIG_USER_BUSYBOX_FGCONSOLE is not set
# CONFIG_USER_BUSYBOX_SETFONT is not set
# CONFIG_USER_BUSYBOX_MINIPS is not set
# CONFIG_USER_BUSYBOX_NUKE is not set
# CONFIG_USER_BUSYBOX_RESUME is not set
# CONFIG_USER_BUSYBOX_RUN_INIT is not set
# CONFIG_USER_BUSYBOX_PATCH is not set
# CONFIG_USER_BUSYBOX_VI is not set
# CONFIG_USER_BUSYBOX_EGREP is not set
# CONFIG_USER_BUSYBOX_FGREP is not set
# CONFIG_USER_BUSYBOX_POWEROFF is not set
# CONFIG_USER_BUSYBOX_REBOOT is not set
# CONFIG_USER_BUSYBOX_LINUXRC is not set
# CONFIG_USER_BUSYBOX_ADD_SHELL is not set
# CONFIG_USER_BUSYBOX_REMOVE_SHELL is not set
# CONFIG_USER_BUSYBOX_ADDUSER is not set
# CONFIG_USER_BUSYBOX_CHPASSWD is not set
# CONFIG_USER_BUSYBOX_CRYPTPW is not set
# CONFIG_USER_BUSYBOX_MKPASSWD is not set
# CONFIG_USER_BUSYBOX_DEPMOD is not set
CONFIG_USER_BUSYBOX_INSMOD=y
CONFIG_USER_BUSYBOX_LSMOD=y
CONFIG_USER_BUSYBOX_MODINFO=y
CONFIG_USER_BUSYBOX_MODPROBE=y
CONFIG_USER_BUSYBOX_RMMOD=y
CONFIG_USER_BUSYBOX_FEATURE_CMDLINE_MODULE_OPTIONS=y
CONFIG_USER_BUSYBOX_FEATURE_MODPROBE_SMALL_CHECK_ALREADY_LOADED=y
# CONFIG_USER_BUSYBOX_BLKDISCARD is not set
# CONFIG_USER_BUSYBOX_BLOCKDEV is not set
# CONFIG_USER_BUSYBOX_CAL is not set
# CONFIG_USER_BUSYBOX_CHRT is not set
# CONFIG_USER_BUSYBOX_EJECT is not set
# CONFIG_USER_BUSYBOX_FALLOCATE is not set
# CONFIG_USER_BUSYBOX_FATATTR is not set
# CONFIG_USER_BUSYBOX_FDFLUSH is not set
# CONFIG_USER_BUSYBOX_FSFREEZE is not set
# CONFIG_USER_BUSYBOX_FSTRIM is not set
# CONFIG_USER_BUSYBOX_HD is not set
# CONFIG_USER_BUSYBOX_XXD is not set
# CONFIG_USER_BUSYBOX_IONICE is not set
# CONFIG_USER_BUSYBOX_MDEV is not set
# CONFIG_USER_BUSYBOX_MESG is not set
# CONFIG_USER_BUSYBOX_MKE2FS is not set
# CONFIG_USER_BUSYBOX_MKFS_EXT2 is not set
# CONFIG_USER_BUSYBOX_MKFS_MINIX is not set
# CONFIG_USER_BUSYBOX_MKFS_REISER is not set
# CONFIG_USER_BUSYBOX_MKDOSFS is not set
# CONFIG_USER_BUSYBOX_MKFS_VFAT is not set
# CONFIG_USER_BUSYBOX_FEATURE_MOUNT_OTHERTAB is not set
# CONFIG_USER_BUSYBOX_MOUNTPOINT is not set
# CONFIG_USER_BUSYBOX_NSENTER is not set
# CONFIG_USER_BUSYBOX_RENICE is not set
# CONFIG_USER_BUSYBOX_REV is not set
# CONFIG_USER_BUSYBOX_LINUX32 is not set
# CONFIG_USER_BUSYBOX_LINUX64 is not set
# CONFIG_USER_BUSYBOX_SETPRIV is not set
# CONFIG_USER_BUSYBOX_SETSID is not set
# CONFIG_USER_BUSYBOX_SWAPON is not set
# CONFIG_USER_BUSYBOX_SWAPOFF is not set
# CONFIG_USER_BUSYBOX_TASKSET is not set
# CONFIG_USER_BUSYBOX_UEVENT is not set
# CONFIG_USER_BUSYBOX_UNSHARE is not set
# CONFIG_USER_BUSYBOX_BC is not set
# CONFIG_USER_BUSYBOX_DC is not set
# CONFIG_USER_BUSYBOX_CONSPY is not set
# CONFIG_USER_BUSYBOX_FLASH_ERASEALL is not set
# CONFIG_USER_BUSYBOX_FLASHCP is not set
# CONFIG_USER_BUSYBOX_HEXEDIT is not set
# CONFIG_USER_BUSYBOX_I2CGET is not set
# CONFIG_USER_BUSYBOX_I2CSET is not set
# CONFIG_USER_BUSYBOX_I2CDUMP is not set
# CONFIG_USER_BUSYBOX_I2CDETECT is not set
# CONFIG_USER_BUSYBOX_I2CTRANSFER is not set
# CONFIG_USER_BUSYBOX_INOTIFYD is not set
# CONFIG_USER_BUSYBOX_LESS is not set
# CONFIG_USER_BUSYBOX_LSSCSI is not set
# CONFIG_USER_BUSYBOX_NANDWRITE is not set
# CONFIG_USER_BUSYBOX_NANDDUMP is not set
# CONFIG_USER_BUSYBOX_PARTPROBE is not set
# CONFIG_USER_BUSYBOX_SETFATTR is not set
# CONFIG_USER_BUSYBOX_SETSERIAL is not set
# CONFIG_USER_BUSYBOX_TS is not set
# CONFIG_USER_BUSYBOX_UBIATTACH is not set
# CONFIG_USER_BUSYBOX_UBIDETACH is not set
# CONFIG_USER_BUSYBOX_UBIMKVOL is not set
# CONFIG_USER_BUSYBOX_UBIRMVOL is not set
# CONFIG_USER_BUSYBOX_UBIRSVOL is not set
# CONFIG_USER_BUSYBOX_UBIUPDATEVOL is not set
# CONFIG_USER_BUSYBOX_UBIRENAME is not set
# CONFIG_USER_BUSYBOX_DNSDOMAINNAME is not set
# CONFIG_USER_BUSYBOX_IFUP is not set
# CONFIG_USER_BUSYBOX_IFDOWN is not set
# CONFIG_USER_BUSYBOX_IPADDR is not set
# CONFIG_USER_BUSYBOX_IPLINK is not set
# CONFIG_USER_BUSYBOX_IPROUTE is not set
# CONFIG_USER_BUSYBOX_IPTUNNEL is not set
# CONFIG_USER_BUSYBOX_IPRULE is not set
# CONFIG_USER_BUSYBOX_IPNEIGH is not set
# CONFIG_USER_BUSYBOX_FAKEIDENTD is not set
# CONFIG_USER_BUSYBOX_NAMEIF is not set
# CONFIG_USER_BUSYBOX_NBDCLIENT is not set
# CONFIG_USER_BUSYBOX_NC is not set
# CONFIG_USER_BUSYBOX_NETCAT is not set
# CONFIG_USER_BUSYBOX_PING is not set
# CONFIG_USER_BUSYBOX_SSL_CLIENT is not set
# CONFIG_USER_BUSYBOX_TC is not set
# CONFIG_USER_BUSYBOX_UDPSVD is not set
# CONFIG_USER_BUSYBOX_WHOIS is not set
# CONFIG_USER_BUSYBOX_UDHCPD is not set
# CONFIG_USER_BUSYBOX_DUMPLEASES is not set
# CONFIG_USER_BUSYBOX_DHCPRELAY is not set
# CONFIG_USER_BUSYBOX_UDHCPC is not set
# CONFIG_USER_BUSYBOX_FREE is not set
# CONFIG_USER_BUSYBOX_FUSER is not set
# CONFIG_USER_BUSYBOX_KILL is not set
# CONFIG_USER_BUSYBOX_KILLALL is not set
# CONFIG_USER_BUSYBOX_KILLALL5 is not set
# CONFIG_USER_BUSYBOX_PGREP is not set
# CONFIG_USER_BUSYBOX_PKILL is not set
# CONFIG_USER_BUSYBOX_PIDOF is not set
# CONFIG_USER_BUSYBOX_PS is not set
# CONFIG_USER_BUSYBOX_BB_SYSCTL is not set
# CONFIG_USER_BUSYBOX_RUNSV is not set
# CONFIG_USER_BUSYBOX_RUNSVDIR is not set
# CONFIG_USER_BUSYBOX_SV is not set
# CONFIG_USER_BUSYBOX_SVC is not set
# CONFIG_USER_BUSYBOX_SVOK is not set
# CONFIG_USER_BUSYBOX_SVLOGD is not set
# CONFIG_USER_BUSYBOX_SH_IS_ASH is not set
# CONFIG_USER_BUSYBOX_SH_IS_HUSH is not set
CONFIG_USER_BUSYBOX_SH_IS_NONE=y
# CONFIG_USER_BUSYBOX_BASH_IS_ASH is not set
# CONFIG_USER_BUSYBOX_BASH_IS_HUSH is not set
CONFIG_USER_BUSYBOX_BASH_IS_NONE=y
# CONFIG_USER_BUSYBOX_LOGREAD is not set
# CONFIG_USER_BUSYBOX_SYSLOGD is not set
