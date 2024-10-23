
export ACL_LICENSE = COFFEE-WARE

PKG = input-event-daemon
URL = https://github.com/gandro/input-event-daemon/tarball/v0.1.3
DOWNLOADNAME = $(PKG)-v0.1.3.tar
BUILDDIR = gandro-input-event-daemon-13f4d61
input-event-daemon_ROMFS = add_init_script

include $(ROOTDIR)/tools/automake.inc

add_init_script:
	$(ROMFSINST) /etc/init.d/S99input-event-daemon 

