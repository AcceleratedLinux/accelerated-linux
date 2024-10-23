VERSION = 3.25
URL = https://download.savannah.gnu.org/releases/gpsd/gpsd-$(VERSION).tar.gz
BUILDTARGET = my_built
INSTALLTARGET = my_installed
AUTOMAKE_ROMFS = my_romfs
include $(ROOTDIR)/tools/automake.inc

SCONS_OPTIONS = \
	minimal=yes \
	control_socket=yes \
	ipv6=yes \
	ashtech=yes \
	socket_export=yes \
	gpsd_group=tty \
	gpsdclients=yes \
	udevdir=/libexec/udev \
	ublox=yes \
	shm_export=yes \
	nostrip=yes \
	manbuild=no

ifdef CONFIG_USER_GPSD_PYTHON_PACKAGE
include ../python-modules/python-modules.mk

SCONS_OPTIONS += \
	python=yes \
	target_python=$(PYTHON_HOSTINSTALL)/bin/python3 \
	python_shebang='/usr/bin/env python3' \
	python_libdir="/usr/lib/python$(PYTHON_VERSION_SHORT)/site-packages"
else
SCONS_OPTIONS += \
	python=no
endif

SCONS = \
	$(BUILDVARS) \
	$($*_BUILDVARS) \
	$(UCFRONT_ENV) scons -C build/$($*_BUILDDIR) prefix=/usr $(SCONS_OPTIONS)

build/%-my_built: build/%-configured
	@echo "Building $* ..."
	$(AT)ln -sf `pwd`/gpsd.rules.in build/$($*_BUILDDIR)/gpsd.rules.in
	$(AT)$(SCONS)
	$(AT)touch $@

build/%-my_installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install/usr/
	$(AT)DESTDIR=`pwd`/build/$($(PKG)_BUILDDIR)-install $(SCONS) udev-uninstall
	$(AT)DESTDIR=`pwd`/build/$($(PKG)_BUILDDIR)-install $(SCONS) udev-install
	$(AT)touch $@

my_romfs:
	$(ROMFSINST) -d build/$($(PKG)_BUILDDIR)-install/usr/sbin/gpsd /usr/sbin/gpsd
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)-install/usr/sbin/gpsdctl /usr/sbin/gpsdctl
	$(ROMFSINST) -d build/$($(PKG)_BUILDDIR)-install/usr/bin/gpsdecode /usr/bin/gpsdecode
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)-install/usr/bin/gps2udp /usr/bin/gps2udp
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)-install/usr/bin/gpsctl /usr/bin/gpsctl
	$(ROMFSINST) build/$($(PKG)_BUILDDIR)-install/usr/bin/gpspipe /usr/bin/gpspipe
	$(ROMFSINST) -d -p +x build/$($(PKG)_BUILDDIR)-install/libexec/udev/gpsd.hotplug /libexec/udev/gpsd.hotplug
	$(ROMFSINST) -d build/$($(PKG)_BUILDDIR)-install/libexec/udev/rules.d/25-gpsd.rules /libexec/udev/rules.d/25-gpsd.rules
	$(ROMFSINST) -e CONFIG_USER_GPSD_UBXTOOL build/$($(PKG)_BUILDDIR)-install/usr/bin/ubxtool /usr/bin/ubxtool
	$(ROMFSINST) -e CONFIG_USER_GPSD_PYTHON_PACKAGE -d build/$($(PKG)_BUILDDIR)-install/usr/lib/python$(PYTHON_VERSION_SHORT) /usr/lib/python$(PYTHON_VERSION_SHORT)
