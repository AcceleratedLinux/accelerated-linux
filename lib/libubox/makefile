URL      = https://git.openwrt.org/project/libubox.git
VERSION   = d2223ef9da7172a84d1508733dc58840e1381e3c
METHOD   = git
CMAKE_OPTS = -DBUILD_LUA=0

include $(ROOTDIR)/tools/cmake.inc

romfs_post:
	mkdir -p $(ROMFSDIR)/usr/share
	cp -afL $(ROMFSDIR)/share/libubox $(ROMFSDIR)/usr/share/.
	rm -rf $(ROMFSDIR)/share/libubox 

