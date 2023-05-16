VERSION = 0.60.3
URL = https://github.com/mesonbuild/meson/releases/download/$(VERSION)/meson-$(VERSION).tar.gz
DOWNLOADHASH = 87ca5fa9358a01864529392bd64e027158eb94afca7c7766b1866ef27eccb98e

# Building it for host only
FINALTARGET = my_built

HOST_BUILDDIR = build/install

include $(ROOTDIR)/tools/hostauto.inc

build/%-my_built: build/%-$(CONFIGTARGET)
	$(AT)mkdir -p $(HOST_BUILDDIR)
	build/meson-$(VERSION)/packaging/create_zipapp.py build/meson-$(VERSION) --outfile $(HOST_BUILDDIR)/meson.pyz
	$(AT)touch $@

