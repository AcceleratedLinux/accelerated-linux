VERSION = 3.18
URL = http://download.savannah.gnu.org/releases/gpsd/gpsd-$(VERSION).tar.gz
BUILDTARGET = my_built
INSTALLTARGET = my_installed
AUTOMAKE_ROMFS = my_romfs
include $(ROOTDIR)/tools/automake.inc

SCONS_OPTIONS = \
	minimal=yes \
	control_socket=yes \
	ipv6=yes \
	nmea0183=yes \
	socket_export=yes \
	controlsend=yes \
	gpsd_group=tty \
	gpsdclients=yes \
	udevdir=$(STAGEDIR)/libexec/udev \
	ublox=yes

SCONS = \
	$(BUILDVARS) \
	$($*_BUILDVARS) \
	$(UCFRONT_ENV) scons -C build/$($*_BUILDDIR) prefix=$(CURDIR)/build/$*-install/usr/ $(SCONS_OPTIONS)

build/%-my_built: build/%-configured
	@echo "Building $* ..."
	$(AT)ln -sf `pwd`/gpsd.rules.in build/$($*_BUILDDIR)/gpsd.rules.in
	$(AT)$(SCONS)
	$(AT)touch $@

build/%-my_installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install/usr/
	$(AT)$(SCONS) udev-uninstall
	$(AT)$(SCONS) udev-install
	$(AT)touch $@

my_romfs:
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsd /usr/sbin/gpsd
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsdecode /usr/bin/gpsdecode
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsdctl /usr/bin/gpsdctl
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gps2udp /usr/bin/gps2udp
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsctl /usr/bin/gpsctl
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpspipe /usr/bin/gpspipe
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsd.hotplug /libexec/udev/gpsd.hotplug
	chmod +x $(ROMFSDIR)/libexec/udev/gpsd.hotplug
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)/gpsd.rules /libexec/udev/rules.d/25-gpsd.rules
