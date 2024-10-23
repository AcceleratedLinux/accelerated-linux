VERSION = v6.0.1
URL = https://github.com/lxc/lxc.git
METHOD = git

LIBTOOL = $(ROOTDIR)/user/lxc/build/$($(PKG)_BUILDDIR)/libtool

AUTOMAKE_ROMFS = lxc_romfs
INSTALLDIR = build/lxc-install

SECCOMP_CFLAGS = -I$(ROOTDIR)/staging/include

CONFOPTS += -Dlocalstatedir=/opt
CONFOPTS += -Ddatadir=/etc
CONFOPTS += -Druntime-path=/var/run
CONFOPTS += -Dinit-script=sysvinit
CONFOPTS += -Ddbus=false
CONFOPTS += -Dman=false
CONFOPTS += -Dexamples=false
CONFOPTS += -Dapparmor=false
CONFOPTS += -Db_lto=true
CONFOPTS += -Dc_args="-fPIC"
CONFOPTS += -Dc_link_args="-fPIC"

include $(ROOTDIR)/tools/meson.inc

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
