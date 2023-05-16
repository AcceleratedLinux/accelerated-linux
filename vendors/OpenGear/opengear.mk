#
# This Makefile fragment is included by <product>/Makefile
# and is common to image construction.
#
# The important targets are
#
#   romfs:	- any product-specific changes to $(ROMFSDIR)
#   image:	- create output files under image
#
# Images are presumed to be formed from $IMAGE_PARTS, and
# the command to combine the parts is in $IMAGE_CMD.

VENDORDIR := $(dir $(lastword $(MAKEFILE_LIST)))

clean::

.PHONY: romfs image

#    /                   squashfs ro  $(ROMFSDIR)
#    /var                tmpfs    rw
#    /etc/config         jffs2    rw  (persists)
#    /var/mnt/storage.*  various  rw

IMAGE	?= $(IMAGEDIR)/image.bin
ROMFSIMG ?= $(IMAGEDIR)/rootfs.bin

ROMFS_DIRS += dev etc bin sbin lib var
ROMFS_DIRS += usr/bin usr/sbin
ROMFS_DIRS += etc/config
ROMFS_DIRS += etc/default
ROMFS_DIRS += $(ROMFS_DIRS-y)
ROMFS_DIRS-$(CONFIG_SYSFS) += sys
ROMFS_DIRS-$(CONFIG_PROC_FS) += proc
ROMFS_DIRS-$(CONFIG_USER_UDEV) += lib/udev/devices/pts lib/udev/devices/flash
ROMFS_DIRS-$(CONFIG_MODULES) += lib/modules
ROMFS_DIRS += home/httpd/cgi-bin

image:: $(IMAGE)

$(ROMFSDIR):
	mkdir $@

romfs:: | $(ROMFSDIR)
	mkdir -p -m 755 $(ROMFS_DIRS:%=$(ROMFSDIR)/%)
	$(ROMFSINST) $(VENDORDIR)/romfs /
	[ ! -d romfs ] || $(ROMFSINST) romfs /
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/run /run
	$(ROMFSINST) -s /etc/config/passwd /etc/passwd
	$(ROMFSINST) -s /etc/config/group /etc/group
	$(ROMFSINST) -s /etc/config/shadow /etc/shadow
	$(ROMFSINST) -s /etc/config/gshadow /etc/gshadow
	$(ROMFSINST) -s /etc/config/bash_profile /.profile
	$(ROMFSINST) -s /etc/config/TZ /etc/TZ
	$(ROMFSINST) -s /etc/config /etc/Wireless
	$(ROMFSINST) -s /etc/config/gai.conf /etc/gai.conf
	$(ROMFSINST) -s /etc/config/pam.d /etc/pam.d

	# correct permissions
	chmod 640 $(ROMFSDIR)/etc/default/config.xml
	-chmod 640 $(ROMFSDIR)/etc/default/pam.d/*
	chmod +x,go-w -R $(ROMFSDIR)/etc/scripts

	# XXX remove things that shouldn't get installed
	rm -rf $(ROMFSDIR)/man[1-9]
	rm -rf $(ROMFSDIR)/usr/man
	rm -rf $(ROMFSDIR)/lib/pkgconfig
	rm -rf $(ROMFSDIR)/usr/share/doc
	rm -rf $(ROMFSDIR)/usr/include
	rm -rf $(ROMFSDIR)/usr/share/locale

	echo "$(VERSIONSTR) -- $(shell date -d "$(BUILDTIMESTAMP)" -u)" \
		> $(ROMFSDIR)/etc/version

	# Syntax check all the shell scripts
	find $(ROMFSDIR)/etc/scripts -type f | \
	  xargs awk '/^#![      ]*\/bin\/(sh|bash)/ {print FILENAME} {nextfile}' | \
	  xargs -n 1 bash -n

# this is a peculiar variant of mksquashfs that inserts the cramfs header
MKSQUASHFS_DIR = $(ROOTDIR)/user/squashfs-new/squashfs-tools
mksquashfs:
	CC="$(HOSTCC)" CFLAGS="$(HOSTCFLAGS)" \
	EXTRA_CFLAGS="-DCONFIG_SQUASHFS_CRAMFS_MAGIC=1" \
	$(MAKE) -C $(MKSQUASHFS_DIR) mksquashfs unsquashfs
	ln -fs $(MKSQUASHFS_DIR)/mksquashfs .
	ln -fs $(MKSQUASHFS_DIR)/unsquashfs .
clean::
	-$(MAKE) -C $(MKSQUASHFS_DIR) clean
	rm -f mksquashfs 

# generate the FSK trailer, minus the trailing 4-byte checksum
FSK_TRAILER_HEAD = \
	printf '\0%s\0%s\0%s' $(VERSIONPKG) $(HW_VENDOR) $(HW_PRODUCT)
# generate the FSK trailer checksum
FSK_TRAILER_CKSUM = \
	$(ROOTDIR)/bin/cksum -b -o 2 
# append a full snapgear-style fsk trailer to $@, required by netflash
FSK_TRAILER_CMD = \
	$(FSK_TRAILER_HEAD) >>$@ && \
	$(FSK_TRAILER_CKSUM) $@ >>$@

# Macro to fail when file $@ is bigger than 
define CHECK_SIZE # $(MAXSIZE)
	@_f () { \
	  sz=$$(wc -c <$@); \
	  if [ $$sz -gt $$1 ]; then \
	     echo "$@: too big ($$sz > $$1)"; \
	     exit 1; \
	  fi; \
	}; _f
endef

# Macro to export files to /tftpboot/
define COPY_INTO_TFTPBOOT # files...
  $(if $(NO_BUILD_INTO_TFTPBOOT),,cp -f $1 /tftpboot/)
endef

MKSQUASHFS_FLAGS += $(MKSQUASHFS_FLAGS-y)
MKSQUASHFS_FLAGS += -noappend 
MKSQUASHFS_FLAGS-$(CONFIG_SQUASHFS_XZ) += -comp xz
MKSQUASHFS_FLAGS-$(CONFIG_XZ_DEC_ARM) += -Xbcj arm
MKSQUASHFS_FLAGS += -all-root
MKSQUASHFS_FLAGS += $(if $(CONFIG_SQUASHFS_XATTR),,-no-xattrs)
MKSQUASHFS_FLAGS += $(SQUASH_ENDIAN)

# apt install squashfs-tools-ng
#	(cd $(ROMFSDIR) && pax -w -M root -f ustar .) | \
#	   tar2sqfs $(TAR2SQFS_FLAGS) $(ROMFSIMG)

$(IMAGEDIR):; mkdir -p $@
$(ROMFSIMG): romfs mksquashfs | $(@D)
	./mksquashfs $(ROMFSDIR) $(ROMFSIMG) $(MKSQUASHFS_FLAGS) 

# Old-style FSK images are a concatenation of 
#   filesystem + kernel + dtb + trailer
IMAGE_PARTS_FSK ?= $(ROMFSIMG) $(ZIMAGE) $(DTB)
define IMAGE_CMD_FSK ?=
	cat $(IMAGE_PARTS) >$@
	$(call FSK_TRAILER_CMD,$@)
	$(if $(IMAGE_MAX_SIZE),$(CHECK_SIZE) $(IMAGE_MAX_SIZE))
	$(call DUMP_DTS,$(filter %.dtb,$(IMAGE_PARTS)))
endef

DUMP_DTS ?= $(if $1,$(ROOTDIR)/$(LINUXDIR)/scripts/dtc/dtc -O dts -s $1 >$(IMAGEDIR)/devicetree.dts)
clean::; rm -f $(IMAGEDIR)/devicetree.dts

IMAGE_PARTS ?= $(IMAGE_PARTS_FSK)
IMAGE_CMD ?= $(IMAGE_CMD_FSK)

$(IMAGE): $(IMAGE_PARTS) | $(IMAGEDIR)
	$(IMAGE_CMD)
	-$(call COPY_INTO_TFTPBOOT,$@)


# Generate list of processes to kill during a netflash upgrade
NETFLASH_KILL_LIST_FILE ?= etc/netflash_kill_list.txt

# Processes that are killed on all platforms
NETFLASH_KILL_LIST-y ?= 
NETFLASH_KILL_LIST-$(CONFIG_PROP_AUTHD_AUTHD)			+= authd
NETFLASH_KILL_LIST-$(CONFIG_USER_ZEBRA_BGPD_BGPD)		+= bgpd
NETFLASH_KILL_LIST-$(CONFIG_USER_CLAMAV_CLAMAV)			+= clamav
NETFLASH_KILL_LIST-$(CONFIG_USER_CLAMAV_CLAMD)			+= clamd
NETFLASH_KILL_LIST-$(CONFIG_USER_CLAMAV_CLAMSMTP)		+= clamsmtpd
NETFLASH_KILL_LIST-$(CONFIG_USER_CRON_CRON)			+= cron
NETFLASH_KILL_LIST-$(CONFIG_USER_DHCPD_DHCPD)			+= dhcpd
NETFLASH_KILL_LIST-$(CONFIG_USER_DHCP_ISC_SERVER_DHCPD)		+= dhcpd
NETFLASH_KILL_LIST-$(CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY)	+= dhcrelay
NETFLASH_KILL_LIST-$(CONFIG_USER_DNSMASQ_DNSMASQ)		+= dnsmasq
NETFLASH_KILL_LIST-$(CONFIG_USER_DNSMASQ2_DNSMASQ2)		+= dnsmasq
NETFLASH_KILL_LIST-$(CONFIG_USER_FLATFSD_FLATFSD)		+= flatfsd
NETFLASH_KILL_LIST-$(CONFIG_USER_FROX_FROX)			+= frox
NETFLASH_KILL_LIST-$(CONFIG_USER_SSH_SSHKEYGEN)			+= gen-keys
NETFLASH_KILL_LIST-$(CONFIG_PROP_HTTPSCERTGEN_HTTPSCERTGEN)	+= https-certgen
NETFLASH_KILL_LIST-$(CONFIG_USER_IDB_IDB)			+= idb
NETFLASH_KILL_LIST-$(CONFIG_USER_BUSYBOX_BUSYBOX)		+= klogd
NETFLASH_KILL_LIST-$(CONFIG_USER_NETIFD)			+= netifd
NETFLASH_KILL_LIST-$(CONFIG_PROP_NFLOGD_NFLOGD)			+= nflogd
NETFLASH_KILL_LIST-$(CONFIG_USER_NTPD_NTPD)			+= ntpd
NETFLASH_KILL_LIST-$(CONFIG_PROP_AUTHD_AUTHD)			+= proxy80
NETFLASH_KILL_LIST-$(CONFIG_USER_ZEBRA_RIPD_RIPD)		+= ripd
NETFLASH_KILL_LIST-$(CONFIG_USER_SIPROXD_SIPROXD)		+= siproxd
NETFLASH_KILL_LIST-$(CONFIG_PROP_AUTHD_AUTHD)			+= sgadnsd
NETFLASH_KILL_LIST-$(CONFIG_USER_SNMPD_SNMPD)			+= snmpd
NETFLASH_KILL_LIST-$(CONFIG_USER_NETSNMP_SNMPD)			+= snmpd
NETFLASH_KILL_LIST-$(CONFIG_USER_SNORT_SNORT)			+= snort
NETFLASH_KILL_LIST-$(CONFIG_USER_SNORT_SNORT)			+= snort-inline
NETFLASH_KILL_LIST-$(CONFIG_USER_SQUID_SQUID)			+= squid
NETFLASH_KILL_LIST-$(CONFIG_USER_SSH_SSHKEYGEN)			+= ssh-keygen
NETFLASH_KILL_LIST-$(CONFIG_PROP_STATSD_STATSD)			+= statsd
NETFLASH_KILL_LIST-$(CONFIG_USER_BUSYBOX_BUSYBOX)		+= syslogd
NETFLASH_KILL_LIST-$(CONFIG_USER_LINUXIGD_LINUXIGD)		+= upnpd
NETFLASH_KILL_LIST-$(CONFIG_USER_ZEBRA_ZEBRA_ZEBRA)		+= zebra

$(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE): .force
	echo $(NETFLASH_KILL_LIST-y) | tr ' ' '\n' >$@
.PHONY: .force

ifeq ($(CONFIG_USER_NETFLASH_NETFLASH),y)
  romfs:: $(ROMFSDIR)/$(NETFLASH_KILL_LIST_FILE)
endif
