VERSION = 1.4.1
URL = https://github.com/mesonbuild/meson/releases/download/$(VERSION)/meson-$(VERSION).tar.gz
DOWNLOADHASH = 1b8aad738a5f6ae64294cc8eaba9a82988c1c420204484ac02ef782e5bba5f49

# Building it for host only
FINALTARGET = my_built

HOST_BUILDDIR = build/install

include $(ROOTDIR)/tools/hostauto.inc

build/%-my_built: build/%-$(CONFIGTARGET)
	$(AT)mkdir -p $(HOST_BUILDDIR)
	build/meson-$(VERSION)/packaging/create_zipapp.py build/meson-$(VERSION) --outfile $(HOST_BUILDDIR)/meson.pyz
	mkdir -p $(ROOTDIR)/bin && ln -sfr $(HOST_BUILDDIR)/meson.pyz $(ROOTDIR)/bin/meson
	$(AT)touch $@

