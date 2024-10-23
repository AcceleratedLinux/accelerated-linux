URL = https://www.corpit.ru/mjt/udns/udns-0.4.tar.gz
BUILDTARGET = my_built
INSTALLTARGET = my_installed
include $(ROOTDIR)/tools/automake.inc
CONFIGURE_OPTS=
CONFOPTS=--enable-ipv6

build/%-my_built: build/%-configured
	@echo "Building $* ..."
	$(AT)$(BUILDVARS) \
	$($*_BUILDVARS) \
		$(UCFRONT_ENV) $(MAKE) -C build/$($*_BUILDDIR) $($*_BUILD) $(MAKEVARS) $($*_MAKEVARS) static shared
	$(AT)touch $@

build/%-my_installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install/lib/
	cp build/$($*_BUILDDIR)/libudns.a build/$*-install/lib/
	cp build/$($*_BUILDDIR)/libudns.so.0 build/$*-install/lib/
	ln -sf libudns.so.0 build/$*-install/lib/libudns_s.so
	$(AT)mkdir -p build/$*-install/include/
	cp build/$($*_BUILDDIR)/udns.h build/$*-install/include/
	$(AT)mkdir -p build/$*-install/bin/
	cp build/$($*_BUILDDIR)/dnsget build/$*-install/bin/
	$(AT)touch $@
