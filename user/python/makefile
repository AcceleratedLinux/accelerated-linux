
PYTHON_VERSION = 3.6.10
PYTHON_VERSION_SHORT = $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

# build in parallel, faster
MAKEVARS = -j$(HOST_NCPU)

# use host built version for cross compile to ensure we have the right
# version of python on the build host
BUILDVARS = PATH="$(CURDIR)/build/Python-Hostinstall/bin:$$PATH"
CONFVARS = PATH="$(CURDIR)/build/Python-Hostinstall/bin:$$PATH"

COMMON_CONFOPTS += --disable-codecs-cjk
COMMON_CONFOPTS += --disable-curses
COMMON_CONFOPTS += --disable-idle3
COMMON_CONFOPTS += --disable-lib2to3
COMMON_CONFOPTS += --disable-nis
COMMON_CONFOPTS += --disable-ossaudiodev
COMMON_CONFOPTS += --disable-pydoc
COMMON_CONFOPTS += --disable-test-modules
COMMON_CONFOPTS += --disable-tk
COMMON_CONFOPTS += --without-ensurepip
COMMON_CONFOPTS += --with-pymalloc
COMMON_CONFOPTS += --without-system-ffi
ifdef CONFIG_USER_PYTHON_REMOVE_SOURCE
COMMON_CONFOPTS += --disable-pyc-build
endif

PKG_y = Python-$(PYTHON_VERSION) Python-Host

Python-Host_FINALTARGET = extracted
Python-Host_METHOD      = none

ifneq ($(CONFIG_X86)$(CONFIG_MIPS),)
# HACK to fix module builds for now
CFLAGS += -fPIC
endif

Python-$(PYTHON_VERSION)_CONFVARS += ac_cv_prog_READELF=$(CROSS_COMPILE)readelf
CONFVARS += ac_cv_buggy_getaddrinfo=no
CONFVARS += ac_cv_file__dev_ptmx=yes
CONFVARS += ac_cv_file__dev_ptc=no
CONFVARS += ac_cv_have_long_long_format=yes

Python-$(PYTHON_VERSION)_DEP      = Python-Host
Python-$(PYTHON_VERSION)_URL      = http://www.python.org/ftp/python/$(PYTHON_VERSION)/Python-$(PYTHON_VERSION).tar.xz
Python-$(PYTHON_VERSION)_BUILDDIR = Python-$(PYTHON_VERSION)

Python-$(PYTHON_VERSION)_CONFOPTS += $(COMMON_CONFOPTS)
Python-$(PYTHON_VERSION)_CONFOPTS += --enable-shared
Python-$(PYTHON_VERSION)_CONFOPTS += --prefix=/usr

Python-$(PYTHON_VERSION)_AUTORECONF = autoreconf -i

Python-$(PYTHON_VERSION)_ROMFS = python_romfs

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl
CPPFLAGS += -I$(STAGEDIR)/include -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib

include $(ROOTDIR)/tools/automake.inc

# For python host we force install to Python-Hostinstall so it does not
# match the automake *-install processing for installing into romfs,  we do
# not want to be installed in the firmware.

build/Python-$(PYTHON_VERSION)-autoconf: build/Python-Host-build
build/Python-Host-build: build/Python-$(PYTHON_VERSION)-patched
	rm -rf build/Python-Host*
	cp -ra build/Python-$(PYTHON_VERSION) build/Python-Host
	ln -s Python-$(PYTHON_VERSION)-license build/Python-Host-license
	cd build/Python-Host; \
	export PKG_CONFIG_LIBDIR= PKG_CONFIG_PATH= EXTRA_CFLAGS= CPUFLAGS= CPU_CFLAGS= CXXFLAGS= CPPFLAGS= LDLIBS= LIBC= LIBS= CC=gcc CXX=g++ LD=gcc AR= RANLIB= CFLAGS= LDFLAGS= ; \
	$(CONFVARS) autoreconf -i; \
	$(CONFVARS) ./configure \
		--prefix=`pwd`/../Python-Hostinstall \
		$(COMMON_CONFOPTS) \
		--disable-shared \
		|| exit 1;  \
	make $(MAKEVARS) || exit 1; \
	make Parser/pgen || exit 1; \
	make install $(MAKEVARS) || exit 1
	[ ! -f build/Python-Host/Parser/pgen ] || cp build/Python-Host/Parser/pgen build/Python-Hostinstall/bin/pgen$(PYTHON_VERSION_SHORT)
	touch $@

python_romfs:
	$(ROMFSINST) -s python$(PYTHON_VERSION_SHORT) /usr/bin/python
	# Symlink dynamic library under '/lib' (otherwise uclibc dynamic loader does not find it)
	if [ -f "$(ROMFSDIR)/usr/lib/libpython$(PYTHON_VERSION_SHORT)m.so.1.0" ]; then \
		$(ROMFSINST) -s ../usr/lib/libpython$(PYTHON_VERSION_SHORT)m.so /lib/libpython$(PYTHON_VERSION_SHORT)m.so; \
		$(ROMFSINST) -s ../usr/lib/libpython$(PYTHON_VERSION_SHORT)m.so.1.0 /lib/libpython$(PYTHON_VERSION_SHORT)m.so.1.0; \
	fi
	# Remove not needed static libraries, Windows EXE binaries and header files
	if [ -d "$(ROMFSDIR)" ]; then \
		rm -f $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/distutils/command/*.exe; \
		find $(ROMFSDIR) -type f -name 'libpython*.a' -delete; \
		find $(ROMFSDIR)/usr/include/python$(PYTHON_VERSION_SHORT)m -type f -not -name pyconfig.h -delete; \
	fi
