METHOD      = git
PKG         = ovs
URL         = https://github.com/openvswitch/ovs.git
VERSION     = d3d320770212132630ef942e7d2bde094f8250be

AUTORECONF  = ./boot.sh
CONFVARS    += ac_cv_search___atomic_load_8=no
CONFOPTS    += --with-linux=$(ROOTDIR)/$(LINUXDIR)
CONFOPTS    += --datarootdir="/usr/share"
ifdef CONFIG_USER_DPDK
CONFOPTS    += --with-dpdk=$(ROOTDIR)/user/dpdk/build/dpdk-stable-16.11.1/build
endif
MAKEVARS    = 
$(PKG)_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(MAKE) -C $(ROOTDIR)/$(LINUXDIR) \
		M=`pwd`/build/$(PKG)/datapath/linux \
		modules_install \
		INSTALL_MOD_CMD="$(ROMFSINST) -S -r \"\"" \
		INSTALL_MOD_PATH=$(ROMFSDIR) \
		DEPMOD=$(ROOTDIR)/tools/depmod.sh
	$(ROMFSINST) -R /etc/openvswitch
	$(ROMFSINST) -s /var/run/openvswitch /etc/openvswitch

