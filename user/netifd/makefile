URL      = https://github.com/AcceleratedLinux/netifd.git
METHOD   = git
VERSION  = b71e916ce0267ad94285f2755289207768c2d473

include $(ROOTDIR)/tools/cmake.inc

romfs_post:
	mkdir -p $(ROMFSDIR)/lib/netifd
	$(ROMFSINST) build/netifd/scripts/netifd-proto.sh /lib/netifd/netifd-proto.sh
	$(ROMFSINST) build/netifd/scripts/netifd-wireless.sh /lib/netifd/netifd-wireless.sh
	$(ROMFSINST) build/netifd/scripts/utils.sh /lib/netifd/utils.sh

# $(ROMFSINST) build/netifd/dummy/proto/ppp.sh /lib/netifd/proto/ppp.sh
# $(ROMFSINST) build/netifd/dummy/proto/pptp.sh /lib/netifd/proto/pptp.sh

