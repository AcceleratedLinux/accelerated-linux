URL = https://w1.fi/releases/wpa_supplicant-2.10.tar.gz
FINALTARGET = my_build
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

build/%-my_build: build/%-patched
	@echo "Building $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)
	$(AT)ln -fs `pwd`/config build/$($*_BUILDDIR)/wpa_supplicant/.config
	$(AT)make -C build/$($*_BUILDDIR)/wpa_supplicant
	$(AT)mkdir -p "$(STAGEDIR)/lib"
	$(AT)ln -sf "`pwd`"/build/$($*_BUILDDIR)/wpa_supplicant/libwpa_client.so "$(STAGEDIR)/lib/libwpa_client.so"
	$(AT)mkdir -p "$(STAGEDIR)/include"
	$(AT)ln -sf "`pwd`"/build/$($*_BUILDDIR)/src/common/wpa_ctrl.h "$(STAGEDIR)/include/wpa_ctrl.h"
	$(AT)touch $@

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_WPA_SUPPLICANT build/$($(PKG)_BUILDDIR)/wpa_supplicant/wpa_supplicant /bin/wpa_supplicant
	$(ROMFSINST) -e CONFIG_USER_WPA_SUPPLICANT build/$($(PKG)_BUILDDIR)/wpa_supplicant/libwpa_client.so /lib/libwpa_client.so
	$(ROMFSINST) -e CONFIG_USER_WPA_PASSPHRASE build/$($(PKG)_BUILDDIR)/wpa_supplicant/wpa_passphrase /bin/wpa_passphrase
	$(ROMFSINST) -e CONFIG_USER_WPA_CLI        build/$($(PKG)_BUILDDIR)/wpa_supplicant/wpa_cli        /bin/wpa_cli
