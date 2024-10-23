VERSION = 2.03.05
URL = https://sourceware.org/pub/lvm2/releases/LVM2.$(VERSION).tgz

CONFOPTS = --with-lvm1=none --disable-nls
# uClibc cannot handle symbol versioning, which causes segfault
CONFOPTS += --disable-symvers
CONFVARS = ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes

INSTALLTARGET = my_installed

include $(ROOTDIR)/tools/automake.inc

build/%-my_installed: build/%-$(BUILDTARGET)
	@echo "Installing only the libraries..."
	$(AT)mkdir -p build/$*-install
	$(UCFRONT_ENV) $(MAKE) -C build/$($*_BUILDDIR)/libdm install DESTDIR=`pwd`/build/$*-install
	$(UCFRONT_ENV) $(MAKE) -C build/$($*_BUILDDIR)/tools install_device-mapper DESTDIR=`pwd`/build/$*-install
	$(AT)touch $@

