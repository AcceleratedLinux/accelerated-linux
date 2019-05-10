URL = https://w1.fi/releases/hostapd-2.7.tar.gz
FINALTARGET = my_build
AUTOMAKE_ROMFS = my_romfs
include $(ROOTDIR)/tools/download.inc

build/%-my_build: build/%-patched
	@echo "Building $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)
	$(AT)ln -fs `pwd`/config build/$($*_BUILDDIR)/hostapd/.config
	$(AT)make -C build/$($*_BUILDDIR)/hostapd
	$(AT)touch $@

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_HOSTAPD_HOSTAPD build/$($(PKG)_BUILDDIR)/hostapd/hostapd /bin/hostapd
	$(ROMFSINST) -e CONFIG_USER_HOSTAPD_HOSTAPD_CLI build/$($(PKG)_BUILDDIR)/hostapd/hostapd_cli /bin/hostapd_cli
