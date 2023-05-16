URL      = https://git.openwrt.org/project/libubox.git
VERSION  = c83a84afbef2b24f960ddeda0b5e2ab01fba6981
METHOD   = git
CMAKE_OPTS = -DBUILD_LUA=0

include $(ROOTDIR)/tools/cmake.inc

romfs_post:
	mkdir -p $(ROMFSDIR)/usr/share
	cp -afL $(ROMFSDIR)/share/libubox $(ROMFSDIR)/usr/share/.
	rm -rf $(ROMFSDIR)/share/libubox 

