URL = https://w1.fi/releases/hostapd-2.10.tar.gz
FINALTARGET = my_build
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

build/%-my_configured: build/%-patched
	@echo "Configuring $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)
	$(AT)cp `pwd`/config build/$($*_BUILDDIR)/hostapd/.config
ifdef CONFIG_USER_HOSTAPD_QCA_EXTENSIONS
	$(AT)sed -i '/^#CONFIG_DRIVER_NL80211_QCA.*$$/s/^#//g' build/$($*_BUILDDIR)/hostapd/.config
endif
	$(AT)touch $@

build/%-my_build: build/%-my_configured
	@echo "Building $* ..."
	$(AT)make -C build/$($*_BUILDDIR)/hostapd
	$(AT)ln -sf $(shell pwd)/build/$($*_BUILDDIR)/src/drivers/nl80211_copy.h $(STAGEDIR)/include
	$(AT)touch $@

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_HOSTAPD_HOSTAPD build/$($(PKG)_BUILDDIR)/hostapd/hostapd /bin/hostapd
	$(ROMFSINST) -e CONFIG_USER_HOSTAPD_HOSTAPD_CLI build/$($(PKG)_BUILDDIR)/hostapd/hostapd_cli /bin/hostapd_cli
