LIBVIRT_VERSION = 1.2.20
URL = https://libvirt.org/sources/libvirt-${LIBVIRT_VERSION}.tar.gz

MAKEVARS =
CONFOPTS = --datarootdir=/usr/share --without-dtrace

# The configure script makes some decisions based on finding executables in the
# host machine's PATH. These may be misplaced or non-existent on the target.
# Override them here:
DNSMASQ = /sbin/dnsmasq
ADDR2LINE =
DMIDECODE =
IP_PATH = /bin/ip
EBTABLES_PATH = /bin/ebtables
CONFOPTS += --without-pm-utils

# These storage backends require LVM2's 'device-mapper-dev' libdevmapper,
CONFOPTS += --without-storage-mpath --without-storage-lvm

# VM backends
ifdef CONFIG_USER_QEMU
CONFOPTS += --with-qemu
endif

# Don't use backends that are included by default rather than checked.
# The checks sometimes don't work on account of PATH issues too.
CONFOPTS += --without-vmware
CONFOPTS += --without-vbox

AUTOMAKE_ROMFS = libvirt_romfs

include $(ROOTDIR)/tools/automake.inc

libvirt_romfs:
	$(ROMFSINST) -d build/libvirt-$(LIBVIRT_VERSION)-install/bin /bin
	$(ROMFSINST) -d build/libvirt-$(LIBVIRT_VERSION)-install/sbin /sbin
	$(ROMFSINST) -f -d build/libvirt-$(LIBVIRT_VERSION)-install/lib /lib
	$(ROMFSINST) -d build/libvirt-$(LIBVIRT_VERSION)-install/etc /etc
	# /usr/share/libvirt/cpu_map.xml
	$(ROMFSINST) -d build/libvirt-$(LIBVIRT_VERSION)-install/usr/share/libvirt/cpu_map.xml \
			/usr/share/libvirt/cpu_map.xml
