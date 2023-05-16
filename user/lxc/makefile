VERSION = lxc-4.0.12
URL = https://github.com/lxc/lxc.git
METHOD = git

AUTORECONF = ./autogen.sh

LIBTOOL = $(ROOTDIR)/user/lxc/build/$($(PKG)_BUILDDIR)/libtool

AUTOMAKE_ROMFS = lxc_romfs
INSTALLDIR = build/lxc-install

CFLAGS += -g0
LDFLAGS += -g0
SECCOMP_CFLAGS = -I$(ROOTDIR)/staging/include

CONFOPTS := --datarootdir=/etc \
	--localstatedir=/opt \
	--runstatedir=/var/run \
	--with-distro=unknown

CONFOPTS += --disable-doc \
	--disable-api-docs \
	--disable-examples \
	--disable-bash \
	--disable-apparmor

include $(ROOTDIR)/tools/automake.inc

lxc_romfs:
	$(ROMFSINST) -v -f $(INSTALLDIR)/bin /bin
	$(ROMFSINST) -v -f $(INSTALLDIR)/sbin /sbin
	$(ROMFSINST) -v -f $(INSTALLDIR)/lib /lib
	$(ROMFSINST) -v -f $(INSTALLDIR)/etc/lxc /etc/lxc
	$(ROMFSINST) -v -f $(INSTALLDIR)/libexec /libexec
	$(ROMFSINST) -R /bin/lxc-cgroup
	$(ROMFSINST) -R /bin/lxc-checkconf
	$(ROMFSINST) -R /bin/lxc-checkpoint
	$(ROMFSINST) -R /bin/lxc-config
	$(ROMFSINST) -R /bin/lxc-console
	$(ROMFSINST) -R /bin/lxc-copy
	$(ROMFSINST) -R /bin/lxc-create
	$(ROMFSINST) -R /bin/lxc-destroy
	$(ROMFSINST) -R /bin/lxc-device
	$(ROMFSINST) -R /bin/lxc-freeze
	$(ROMFSINST) -R /bin/lxc-snapshot
	$(ROMFSINST) -R /bin/lxc-top
	$(ROMFSINST) -R /bin/lxc-unfreeze
	$(ROMFSINST) -R /bin/lxc-unshare
	$(ROMFSINST) -R /bin/lxc-update-config
	$(ROMFSINST) -R /bin/lxc-usernsexec
	$(ROMFSINST) -R /bin/lxc-wait
