PYTHON_VERSION ?= 2.7.18
PYTHON_VERSION_SHORT := $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

# NOTE: This makefile is based heavily on user/python/makefile.

# We "install" Python 2.x to /usr/local/python2, so that an LXC container
# can be created with "Clone DAL" enabled, without stomping on the installation
# of Python 2.x at /usr/bin etc. This strategy also means that
# we don't have to try to include files like libc.so in the container ourselves.
INSTALL_PREFIX = usr/local/python2

# build in parallel, faster
MAKEVARS = -j$(HOST_NCPU)

# use host built version for cross compile to ensure we have the right
# version of python on the build host
PYTHON_HOSTINSTALL := $(CURDIR)/build/Python-$(PYTHON_VERSION)-Hostinstall
BUILDVARS = PATH="$(PYTHON_HOSTINSTALL)/bin:$$PATH"
CONFVARS = PATH="$(PYTHON_HOSTINSTALL)/bin:$$PATH"

COMMON_CONFOPTS += --disable-codecs-cjk
COMMON_CONFOPTS += --disable-lib2to3
COMMON_CONFOPTS += --disable-nis
COMMON_CONFOPTS += --disable-ossaudiodev
COMMON_CONFOPTS += --disable-pydoc
COMMON_CONFOPTS += --disable-test-modules
COMMON_CONFOPTS += --disable-tk
COMMON_CONFOPTS += --disable-dbm
COMMON_CONFOPTS += --without-ensurepip
COMMON_CONFOPTS += --with-pymalloc
COMMON_CONFOPTS += --disable-pyo-build

# if we are here we are build everything
PKG_$(CONFIG_USER_PYTHON2)      += Python-$(PYTHON_VERSION)
PKG_$(CONFIG_USER_PYTHON2_HOST) += Python-$(PYTHON_VERSION)-Host

Python-$(PYTHON_VERSION)-Host_FINALTARGET = build
Python-$(PYTHON_VERSION)-Host_METHOD      = none

Python-$(PYTHON_VERSION)_CONFVARS += ac_cv_prog_READELF=$(CROSS_COMPILE)readelf
Python-$(PYTHON_VERSION)_CONFVARS += HOST_PYTHON_FOR_BUILD="$(CURDIR)/build/Python-$(PYTHON_VERSION)-Hostinstall/bin/python"
CONFVARS += ac_cv_buggy_getaddrinfo=no
CONFVARS += ac_cv_file__dev_ptmx=yes
CONFVARS += ac_cv_file__dev_ptc=no
CONFVARS += ac_cv_have_long_long_format=yes

Python-$(PYTHON_VERSION)_PATCH_SERIES = series-$(PYTHON_VERSION)
Python-$(PYTHON_VERSION)_DEP      = Python-$(PYTHON_VERSION)-Host
Python-$(PYTHON_VERSION)_URL      = http://www.python.org/ftp/python/$(PYTHON_VERSION)/Python-$(PYTHON_VERSION).tar.xz
Python-$(PYTHON_VERSION)_BUILDDIR = Python-$(PYTHON_VERSION)

Python-$(PYTHON_VERSION)_CONFOPTS += $(COMMON_CONFOPTS)
Python-$(PYTHON_VERSION)_CONFOPTS += --prefix=/$(INSTALL_PREFIX)

Python-$(PYTHON_VERSION)_AUTORECONF = autoreconf -i

AUTOMAKE_ROMFS = my_romfs

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl

CPPFLAGS += -I$(STAGEDIR)/include -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib
LDFLAGS += -lffi

ifdef CONFIG_USER_PYTHON2
Python-$(PYTHON_VERSION)_FINALTARGET = lxc.tgz
else
# nothing to be installed for python
Python-$(PYTHON_VERSION)_FINALTARGET = autoconf
endif

.DEFAULT_GOAL := all

PYTHON_INSTALLDIR_ROOT := $(CURDIR)/build/Python-$(PYTHON_VERSION)-install

include $(CURDIR)/modules.inc
include $(ROOTDIR)/tools/automake.inc

# For python host we force install to Python-2.7.18-Hostinstall so it does not
# match the automake *-install processing for installing into romfs,  we do
# not want to be installed in the firmware.

.PRECIOUS: build/Python-$(PYTHON_VERSION)-Host-build
build/Python-$(PYTHON_VERSION)-Host-build: build/Python-$(PYTHON_VERSION)-patched
	rm -rf build/Python-$(PYTHON_VERSION)-Host*
	cp -ra build/Python-$(PYTHON_VERSION) build/Python-$(PYTHON_VERSION)-Host
	ln -s Python-$(PYTHON_VERSION)-license build/Python-$(PYTHON_VERSION)-Host-license
	cd build/Python-$(PYTHON_VERSION)-Host; \
		export PKG_CONFIG_LIBDIR= PKG_CONFIG_PATH= PKG_CONFIG_SYSROOT_DIR= EXTRA_CFLAGS= CPUFLAGS= CPU_CFLAGS= CXXFLAGS= CPPFLAGS= LDLIBS= LIBC= LIBS= CC=gcc CXX=g++ LD=gcc AR= RANLIB= CFLAGS= LDFLAGS= ; \
		$(CONFVARS) autoreconf -i; \
		$(CONFVARS) ./configure \
			--prefix=`pwd`/../Python-$(PYTHON_VERSION)-Hostinstall \
			$(COMMON_CONFOPTS) \
			|| exit 1;  \
		make $(MAKEVARS) || exit 1; \
		make install $(MAKEVARS) || exit 1
	touch $@

# TODO(LCG-128): Include Python compatibility code
build/Python-$(PYTHON_VERSION)-lxc.tgz: build/Python-$(PYTHON_VERSION)-installed generate_container.py $(PYTHON2_MODULES_INSTALLED_TARGET)
	# Strip pythonX.X binary, and all .so files.
	$(STRIP) --strip-unneeded \
		$(PYTHON_INSTALLDIR_ROOT)/$(INSTALL_PREFIX)/bin/python$(PYTHON_VERSION_SHORT) \
		$$(find $(PYTHON_INSTALLDIR_ROOT)/$(INSTALL_PREFIX)/lib -type f -name '*.so')
	-rm -f $@
	"./build/Python-$(PYTHON_VERSION)-Hostinstall/bin/python2" \
		"./generate_container.py" \
		--output-file="./$@" \
		--python-build="./build/Python-$(PYTHON_VERSION)-install"
	@# Hardlink the tgz file as a simpler filename. (See DAL-7617)
	ln -f $@ ./build/python_$(subst .,_,$(PYTHON_VERSION_SHORT))_lxc.tgz

# This package does not put anything into romfs.
# It exists only to create the LXC container with Python (plus any modules).
my_romfs:
