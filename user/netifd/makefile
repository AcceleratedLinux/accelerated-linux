URL      = git://git.openwrt.org/project/netifd.git
METHOD   = git
VERSION = 5cf30b59baa03db2448570c78e7e92873555d2ec

include $(ROOTDIR)/tools/cmake.inc

romfs_post:
	mkdir -p $(ROMFSDIR)/lib/netifd
	$(ROMFSINST) build/netifd/scripts/netifd-proto.sh /lib/netifd/netifd-proto.sh
	$(ROMFSINST) build/netifd/scripts/netifd-wireless.sh /lib/netifd/netifd-wireless.sh
	$(ROMFSINST) build/netifd/scripts/utils.sh /lib/netifd/utils.sh

# $(ROMFSINST) build/netifd/dummy/proto/ppp.sh /lib/netifd/proto/ppp.sh
# $(ROMFSINST) build/netifd/dummy/proto/pptp.sh /lib/netifd/proto/pptp.sh

