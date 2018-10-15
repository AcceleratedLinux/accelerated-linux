URL = https://www.kernel.org/pub/linux/utils/net/iproute2/iproute2-3.15.0.tar.xz
MAKEVARS = SHARED_LIBS=n
AUTOMAKE_ROMFS = my_romfs
include $(ROOTDIR)/tools/automake.inc

INSTALL_DIR = build/$(PKG)-install

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_GENL   $(INSTALL_DIR)/sbin/genl /bin/genl
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_IFCFG  $(INSTALL_DIR)/sbin/ifcfg /bin/ifcfg
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_IP     $(INSTALL_DIR)/sbin/ip /bin/ip
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_ROUTEF $(INSTALL_DIR)/sbin/routef /bin/routef
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_ROUTEL $(INSTALL_DIR)/sbin/routel /bin/routel
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTACCT $(INSTALL_DIR)/sbin/rtacct /bin/rtacct
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTMON  $(INSTALL_DIR)/sbin/rtmon /bin/rtmon
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_IP_RTPR   $(INSTALL_DIR)/sbin/rtpr /bin/rtpr
	$(ROMFSINST) -e CONFIG_USER_IPROUTE2_TC_TC     $(INSTALL_DIR)/sbin/tc /bin/tc
