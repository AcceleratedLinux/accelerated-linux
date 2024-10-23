#
# This Makefile fragment incorporates all the declarations and rules
# common to products for this vendor.
#
# It is included by <product>/Makefile 
#

.PHONY: image.clean image.tag image.copy image.dir image.linuz image.arm.zimage image.cramfs
.PHONY: image.sh.mot image.sh.abs image.flash image.configs
.PHONY: romfs.dirs romfs.symlinks romfs.default romfs.recover romfs.rc romfs.version
.PHONY: image.sign-atmel

# Note: These must all be used only in romfs.post::
.PHONY: romfs.nooom

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

ACL_LICENSE = Digi
ACL_URL     = http://www.digi.com/
ACL_PKG     = Digi Accelerated Linux
export ACL_PKG ACL_URL ACL_LICENSE

ACKEY ?= $(HOME)/keys/ackey.pem
ACKEYV3 ?= $(HOME)/keys/ackeyv3.pem
ACCRTV3 ?= $(HOME)/keys/accrtv3.pem
# FIXME: move this one into the tree once we have it
ACCACRTV3 ?= $(HOME)/keys/accacrtv3.pem
# FIXME: fetch this at runtime
ACCRLV3 ?= $(HOME)/keys/accrlv3.pem

# OVFTool location
OVFTOOL_IN_PATH = $(shell which ovftool)
ifneq ($(OVFTOOL_IN_PATH),)
OVFTOOL	  = $(OVFTOOL_IN_PATH)
endif
OVFTOOL	  ?= $(HOME)/tools/ovftool/ovftool

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
	bin sbin etc/config,700 lib/modules var \
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
NETFLASH_KILL_LIST_$(CONFIG_USER_NETIFD)			+= netifd
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
	-make -C $(ROOTDIR)/user/squashfs/squashfs-tools clean
	-make -C $(ROOTDIR)/user/squashfs-new/squashfs-tools clean
	rm -f mkcramfs mksquashfs mksquashfs7z
	rm -f addr.txt

mkcramfs: $(ROOTDIR)/user/cramfs/mkcramfs.c
	$(HOSTCC) -o $@ $< -lz

.PHONY: mksquashfs
mksquashfs:
	CC=$(HOSTCC) CFLAGS=$(HOSTCFLAGS) EXTRA_CFLAGS= make -C $(ROOTDIR)/user/squashfs-new/squashfs-tools mksquashfs
	ln -fs $(ROOTDIR)/user/squashfs-new/squashfs-tools/mksquashfs .

.PHONY: mksquashfs7z
mksquashfs7z:
	make -C $(ROOTDIR)/user/squashfs/squashfs-tools mksquashfs7z
	ln -fs $(ROOTDIR)/user/squashfs/squashfs-tools/mksquashfs7z .

image.sign-atmel:
	if [ -x $(ROOTDIR)/prop/sign_image/sign_atmel.sh ] ; then \
		$(ROOTDIR)/prop/sign_image/sign_atmel.sh -k "$(if $(SIGNING_KEY),$(SIGNING_KEY),$(ROOTDIR)/prop/sign_image/devkeys/DAL-image/atmel_private.pem)" -a "$(SIGNING_ALG)" $(IMAGE) ; \
	else \
		echo "warning: not signing image" ; \
	fi

# Tags an image with vendor,product,version and adds the checksum
image.tag:
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE)
ifdef CONFIG_USER_NETFLASH_CRYPTO
	if [ -f $(ACKEY) ] ; then \
		$(ROOTDIR)/user/netflash/cryptimage -E -k $(ACKEY) -f $(IMAGE) ; \
		printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE) ; \
	fi
endif
ifdef CONFIG_USER_NETFLASH_SHA256
	cat $(IMAGE) | $(ROOTDIR)/user/netflash/sha256sum -b >> $(IMAGE)
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT) >>$(IMAGE)
endif
	$(ROOTDIR)/bin/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
	if [ -f $(ACKEYV3) ] ; then \
		$(ROOTDIR)/user/netflash/cryptimagev3.sh $(IMAGE) $(ACKEYV3) $(ACCRTV3); \
	fi
endif

image.size.zimage:
	@SIZE=`cat $(ZIMAGE) | wc -c`; \
	if [ "$$SIZE" -gt "$(ZIMAGESIZE)" ]; then \
		echo "Error: $(ZIMAGE) size $$SIZE is greater than $(ZIMAGESIZE)"; \
		exit 1; \
	fi

image.size: image.tag
	@SIZE=`cat $(IMAGE) | wc -c`; \
	if [ "$$SIZE" -gt "$(IMAGESIZE)" ]; then \
		echo "Error: $(IMAGE) size $$SIZE is greater than $(IMAGESIZE)"; \
		exit 1; \
	fi

image.copy:
	@set -e; for i in $(UROMFSIMG) $(UKERNEL) $(IMAGE) $(KERNELZ) $(IMAGEDIR)/sh.mot $(IMAGEDIR)/sh.abs; do \
		[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] && continue; \
		[ -f $$i ] || continue; \
		echo cp $$i /tftpboot; \
		cp $$i /tftpboot; \
	done
	@[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || ( echo cp $(IMAGE) /tftpboot/$(CONFIG_PRODUCT).bin; cp $(IMAGE) /tftpboot/$(CONFIG_PRODUCT).bin )

image.dir:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	rm -rf $(ROMFSDIR)/man[1-9] $(ROMFSDIR)/usr/man

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

image.x86.zimage:
	cp $(ROOTDIR)/$(LINUXDIR)/arch/x86/boot/bzImage $(ZIMAGE)

image.mips.vmlinux:
	cp $(ROOTDIR)/$(LINUXDIR)/vmlinux $(VMLINUX)

image.cramfs: mkcramfs
	rm -f $(ROMFSIMG)
	$(ROOTDIR)/tools/fakeroot-env ./mkcramfs -z -r $(ROMFSDIR) $(ROMFSIMG)

SQUASH_BCJ =
ifeq ($(ARCH),arm)
ifdef CONFIG_XZ_DEC_ARM
SQUASH_BCJ = -Xbcj arm
endif
endif
ifneq ($(filter x86 x86_64,$(ARCH)),)
ifdef CONFIG_XZ_DEC_X86
SQUASH_BCJ = -Xbcj x86
endif
endif

image.squashfs: mksquashfs
	rm -f $(ROMFSIMG); mksquashfs=`pwd`/mksquashfs; cd $(ROMFSDIR); \
	$(ROOTDIR)/tools/fakeroot-env $$mksquashfs . $(ROMFSIMG) \
		$(CONFIG_SQUASHFS_XATTR:y=-xattrs) $(SQUASHFS_COMP) \
		-noappend $(SQUASH_BCJ) $(SQUASH_ENDIAN)

image.squashfs7z: mksquashfs7z
	rm -f $(ROMFSIMG); mksquashfs7z=`pwd`/mksquashfs7z; cd $(ROMFSDIR); \
	$(ROOTDIR)/tools/fakeroot-env $$mksquashfs7z . $(ROMFSIMG) \
		$(CONFIG_SQUASHFS_XATTR:y=-xattrs) \
		-noappend $(SQUASH_ENDIAN)

image.romfs:
	rm -f $(ROMFSIMG)
	$(ROOTDIR)/tools/fakeroot-env genromfs -f $(ROMFSIMG) -d $(ROMFSDIR)

# Create (possibly) mbr + cramfs + zimage/linuz
image.bin:
	cat $(MBRIMG) $(ROMFSIMG) $(SHIM) $(ZIMAGE) >$(IMAGE)

# Create an image.bin with ukernel.bin attached
image.ukernel.bin:
	cat $(ROMFSIMG) $(UKERNEL) > $(IMAGE)

# Create image for the X86 platforms
image.x86.bin:
	cp $(ROMFSIMG) $(IMAGE)

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
		mkdir -p $(ROMFSDIR)/$${i%,*}; \
		[ "$${i##*,}" = "$$i" ] || chmod $${i##*,} $(ROMFSDIR)/$${i%,*}; \
	done

romfs.symlinks:
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
ifndef CONFIG_DEVTMPFS_MOUNT
	$(ROMFSINST) -s /var/tmp/log /dev/log
endif
	[ -d $(ROMFSDIR)/sbin ] || $(ROMFSINST) -s bin /sbin

# Override this if necessary
VENDOR_ROMFS_DIR ?= ..

romfs.default:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs /
ifndef CONFIG_USER_FLATFSD_ETC_CONFIG
	-mv $(ROMFSDIR)/etc/default/passwd $(ROMFSDIR)/etc/passwd
	-mv $(ROMFSDIR)/etc/default/group $(ROMFSDIR)/etc/group
	-mv $(ROMFSDIR)/etc/default/pam.conf $(ROMFSDIR)/etc/pam.conf
	-mv $(ROMFSDIR)/etc/default/login.defs $(ROMFSDIR)/etc/login.defs
	-rm -f $(ROMFSDIR)/etc/default/ip-up
	-rm -f $(ROMFSDIR)/etc/default/ip-down
	-rm -f $(ROMFSDIR)/etc/default/ip-pre-up
else
	-rm -f $(ROMFSDIR)/etc/ppp/ip-up
	-rm -f $(ROMFSDIR)/etc/ppp/ip-down
	-rm -f $(ROMFSDIR)/etc/ppp/ip-pre-up
endif
ifdef CONFIG_PROP_CONFIG_ACTIOND
	mkdir -p $(ROMFSDIR)/var/run
ifdef CONFIG_USER_INETD_INETD
	$(ROMFSINST) -s ../var/run/inetd.conf /etc/inetd.conf
endif
ifdef CONFIG_USER_INIT_INIT
	$(ROMFSINST) -s ../var/run/inittab /etc/inittab
	mkdir -p $(ROMFSDIR)/etc/inittab.d
ifdef CONFIG_USER_FLATFSD_FLATFSD
	echo ":unknown:/bin/flatfsd" > $(ROMFSDIR)/etc/inittab.d/flatfsd
endif
endif
endif
ifneq ($(CONFIG_USER_DHCPCD_DHCPCD)$(CONFIG_USER_DHCPCD_NEW_DHCPCD),)
	-chmod 755 $(ROMFSDIR)/etc/default/dhcpcd-change
endif
	-chmod 755 $(ROMFSDIR)/etc/default/ip-*
ifeq ($(CONFIG_LIBCDIR),glibc)
	$(ROMFSINST) -p 400 /etc/nsswitch.conf
endif
ifdef CONFIG_USER_NETFLASH_NETFLASH
	rm -f $(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE)
	for p in $(sort $(NETFLASH_KILL_LIST_y)) ; do echo $$p >> $(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE); done
endif
# Setup symlinks for DigiRM directories defined in prop/config/cc_acl/app/filesystem_service.c
ifdef CONFIG_PROP_ANALYZER
	$(ROMFSINST) -s /etc/config/analyzer /analyzer
endif
ifdef CONFIG_PROP_AWUSBD
	$(ROMFSINST) -s /etc/config/usbtrace /usbtrace
endif
ifdef CONFIG_PROP_CONFIG_SERIALD
	$(ROMFSINST) -s /etc/config/serial /serial
endif
ifdef CONFIG_USER_COOVACHILLI
	$(ROMFSINST) -s /etc/config/hotspot /hotspots
endif
ifdef CONFIG_PROP_XBEE
	$(ROMFSINST) -s /etc/config/xbee-profiles /xbee-profiles
endif
	$(ROMFSINST) -s /etc/config/scripts /applications
	$(ROMFSINST) -s /var/log /logs

romfs.recover:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs.recover /

romfs.factory:
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs/etc/services /etc/services

# Just install the static rc file
romfs.rc romfs.rc.static:
	if [ -f rc-$(CONFIG_LANGUAGE) ]; then \
		$(ROMFSINST) -p 555 /etc/rc-$(CONFIG_LANGUAGE) /etc/rc; \
	else \
		$(ROMFSINST) -p 555 /etc/rc; \
	fi
	[ ! -f filesystems ] || $(ROMFSINST) /etc/filesystems

romfs.emc:
	$(ROMFSINST) -p 555 /etc/emc;

romfs.inittab:
	[ ! -f inittab ] || echo "*** Warning: Static inittab file exists, but using dynamic inittab file"
	$(ROMFSINST) -e CONFIG_USER_INETD_INETD -a "inet:unknown:/bin/inetd" /etc/inittab

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
	if [ -f $(ACKEY) ] ; then \
		openssl rsa -in $(ACKEY) -pubout > $(ROMFSDIR)/etc/publickey.pem ; \
	fi
endif
ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
	if [ -f $(ACCACRTV3) ] ; then \
		$(ROMFSINST) $(ACCACRTV3) /etc/ca.crt ; \
	fi
	if [ -f $(ACCRLV3) ] ; then \
		$(ROMFSINST) $(ACCRLV3) /etc/crl.crt ; \
	fi
endif

romfs.skeleton:
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	set -x;for i in $(VENDOR_ROMFS_DIR)/romfs-skeleton/*; do \
		case "$$i" in \
		*/etc) \
			for j in $$i/*; do \
				if [ -f $$j ]; then \
					$(ROMFSINST) $$j /etc/default/`basename $$j`; \
					$(ROMFSINST) -s /etc/config/`basename $$j` /etc/`basename $$j`; \
				else \
					$(ROMFSINST) $$j /etc/`basename $$j`; \
				fi; \
			done; \
			;; \
		*)  $(ROMFSINST) `pwd`/$$i /`basename $$i` ;; \
		esac; \
	done
else
	$(ROMFSINST) $(VENDOR_ROMFS_DIR)/romfs-skeleton /
endif

romfs.nooom:
	[ ! -x $(ROMFSDIR)/bin/no_oom ] || ( ( cd $(ROMFSDIR) && mkdir -p .no_oom ) && for i in `echo ${CONFIG_USER_NOOOM_BINARIES}` ; do [ -x $(ROMFSDIR)/$$i ] && [ x`readlink $(ROMFSDIR)/$$i` != x/bin/no_oom ] && mv $(ROMFSDIR)/$$i $(ROMFSDIR)/.no_oom/`basename $$i` && ln -s /bin/no_oom $(ROMFSDIR)/$$i || "NOTICE: $$i not present in romfs" ; done )

romfs.cleanup:
	$(ROMFSINST) -R /share
	$(ROMFSINST) -R /usr/share/man
	$(ROMFSINST) -R /etc/usb.ids.gz
	$(ROMFSINST) -R /bin/gettextize
	$(ROMFSINST) -R /bin/iconv
	$(ROMFSINST) -R /lib/pkgconfig
	$(ROMFSINST) -R /lib/gio
	$(ROMFSINST) -R /lib/dbus-1.0
	$(ROMFSINST) -R /lib/cmake
	$(ROMFSINST) -R /var/run
	$(ROMFSINST) -R /usr/lib/pkconfig
	for i in $(ROMFSDIR)/lib/modules/*/*; do [ ! -f "$$i.bin" ] || rm -f "$$i"; done
	$(ROOTDIR)/tools/libclean.sh $(ROMFSDIR)
ifdef CONFIG_PROP_CONFIG_ACTIOND
ifdef CONFIG_USER_INETD_INETD
	# in this configuration,  there is no /etc/default/inetd.conf,
	# everything is config driven,  deal with it.
	$(ROMFSINST) -R /etc/default/inetd.conf
endif
endif

romfs.post:: romfs.nooom

#LIST_SOFTWARE = $(ROOTDIR)/tools/list-software
#$(IMAGEDIR)/sbom.cdx.json: $(IMAGEDIR)/romfs-inst.log $(LIST_SOFTWARE)
#	 cd $(ROOTDIR) && $(LIST_SOFTWARE) > $@
#romfs.post:: $(IMAGEDIR)/sbom.cdx.json

# OVF generation for VM targets
ovf:
	if [ -x $(OVFTOOL) ] ; then	\
		rm -rf $(IMAGEDIR)/$(HDDBASE)-ovf;	\
		mkdir $(IMAGEDIR)/$(HDDBASE)-ovf;	\
		(cd $(IMAGEDIR)/$(HDDBASE)-ovf; $(OVFTOOL) ../$(HDDBASE)/$(IMGBASE).vmx $(IMGBASE).ovf);	\
		rm -f $(IMAGEDIR)/$(HDDBASE)-ovf.zip;	\
		(cd $(IMAGEDIR)/$(HDDBASE)-ovf; zip -r ../$(HDDBASE)-ovf.zip .); \
	fi

# Generates bootloader update package
# It expects the following variables to be set:
#  - BLOADER_IMG:           bootloader image
#  - BLOADER_IMG_VERSION:   version of the bootloader image. If it is not a
#                           pre-built bootloader, but a freshly built one, set
#                           it to "$(VERSIONPKG)" to tag the bootloader with the
#                           current tree's version
#  - BLOADER_UPDATE_SCRIPT: (OPTIONAL) bootloader update script. It is run after
#                           a successful bootloader image update, and can be
#                           used to set different U-Boot ENV variables, etc.
bloader.pack.prop:
	@[ "$(BLOADER_IMG)" ] || { \
		echo "Bootloader image is not set"; \
		exit 1; \
	}

	@[ "$(BLOADER_IMG_VERSION)" ] || { \
		echo "Bootloader image version is not set"; \
		exit 1; \
	}

	@echo "Creating bootloader update package with:"
	@echo " - Bootloader:         $(BLOADER_IMG)"
	@echo " - Version:            $(BLOADER_IMG_VERSION)"
	@echo " - Post-update script: $(if $(BLOADER_UPDATE_SCRIPT),$(BLOADER_UPDATE_SCRIPT),<none>)"

	rm -Rf $(IMAGEDIR)/bloader_pack
	mkdir -p $(IMAGEDIR)/bloader_pack

	@# Creating version number file
	echo $(BLOADER_IMG_VERSION) >> $(IMAGEDIR)/bloader_pack/version

	@# Creating signed bootloader to write with netflash
	if [ "$(SIGNING_ALG)" ]; then \
		$(ROOTDIR)/prop/sign_image/sign_atmel.sh \
			-k "$(if $(SIGNING_KEY),$(SIGNING_KEY),$(ROOTDIR)/prop/sign_image/devkeys/DAL-image/atmel_private.pem)" \
			-a "$(SIGNING_ALG)" $(BLOADER_IMG) \
			$(IMAGEDIR)/bloader_pack/bloader.img; \
	else \
		echo "warning: not signing bootloader"; \
		cp $(BLOADER_IMG) $(IMAGEDIR)/bloader_pack/bloader.img; \
	fi

	@# Adding tag
	printf '\0%s\0%s\0%s' $(BLOADER_IMG_VERSION) $(HW_VENDOR) $(HW_PRODUCT) >> $(IMAGEDIR)/bloader_pack/bloader.img
	$(ROOTDIR)/bin/cksum -b -o 2 $(IMAGEDIR)/bloader_pack/bloader.img >> $(IMAGEDIR)/bloader_pack/bloader.img

	if [ "$(BLOADER_UPDATE_SCRIPT)" ]; then \
		cp "$(BLOADER_UPDATE_SCRIPT)" $(IMAGEDIR)/bloader_pack/update.sh; \
		chmod +x $(IMAGEDIR)/bloader_pack/update.sh; \
	fi

	@# Compress files into the update package. cd into the directory to omit
	@# the './' prefix
	cd $(IMAGEDIR)/bloader_pack && tar zcf $(IMAGEDIR)/bloader.bin *

	rm -Rf $(IMAGEDIR)/bloader_pack

bloader.pack:
	@if [ -d $(ROOTDIR)/prop ] ; then \
		make bloader.pack.prop ; \
	else \
		echo "warning: skipping boot loader packaging" ; \
	fi
