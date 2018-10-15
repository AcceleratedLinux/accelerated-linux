URL      = git://git.openwrt.org/project/libubox.git
VERSION  = e7e554c194d94f43b5a1e885bddbf1f092019ac4
METHOD   = git
CMAKE_OPTS = -DBUILD_LUA=0

include $(ROOTDIR)/tools/cmake.inc

romfs_post:
	mkdir -p $(ROMFSDIR)/usr/share
	cp -afL $(ROMFSDIR)/share/libubox $(ROMFSDIR)/usr/share/.
	rm -rf $(ROMFSDIR)/share/libubox 

