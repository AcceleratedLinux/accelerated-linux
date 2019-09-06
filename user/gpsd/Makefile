VERSION = 3.18
URL = http://download.savannah.gnu.org/releases/gpsd/gpsd-$(VERSION).tar.gz
BUILDTARGET = my_built
INSTALLTARGET = my_installed
include $(ROOTDIR)/tools/automake.inc

SCONS_OPTIONS = \
	minimal=yes \
	ipv6=yes \
	nmea0183=yes \
	socket_export=yes \
	ublox=yes

SCONS = \
	$(BUILDVARS) \
	$($*_BUILDVARS) \
	$(UCFRONT_ENV) scons -C build/$($*_BUILDDIR) prefix=$(CURDIR)/build/$*-install/usr/ $(SCONS_OPTIONS)

build/%-my_built: build/%-configured
	@echo "Building $* ..."
	$(AT)$(SCONS)
	$(AT)touch $@

build/%-my_installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install/usr/
	$(AT)$(SCONS) install
	$(AT)touch $@
