PYTHON_VERSION = 3.10.13
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
COMMON_CONFOPTS += --disable-dbm
COMMON_CONFOPTS += --without-ensurepip
COMMON_CONFOPTS += --with-pymalloc
ifdef CONFIG_USER_PYTHON_REMOVE_SOURCE
COMMON_CONFOPTS += --disable-pyc-build
endif

# if we are here we are build everything
PKG_y += Python-$(PYTHON_VERSION)
PKG_y += Python-Host

Python-Host_FINALTARGET = build
Python-Host_METHOD      = none

ifneq ($(CONFIG_X86)$(CONFIG_MIPS)$(CONFIG_ARM64),)
# HACK to fix module builds for now
CFLAGS += -fPIC
endif

CFLAGS += -O2 -g0
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

INSTALLTARGET = my_installed

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl

CPPFLAGS += -I$(ROOTDIR)/staging/include -I$(ROOTDIR)/staging/usr/include
CPPFLAGS += -I$(ROOTDIR)/user/python/build/Python-Host/Include/internal
CPPFLAGS += -I$(ROOTDIR)/usr/include/python$(PYTHON_VERSION_SHORT)/internal
LDFLAGS += -L$(ROOTDIR)/staging/lib -L$(ROOTDIR)/staging/usr/lib
LDFLAGS += -lffi

ifndef CONFIG_USER_PYTHON
# nothing to be installed for python
Python-$(PYTHON_VERSION)_FINALTARGET = autoconf
endif

include $(ROOTDIR)/tools/automake.inc

# For python host we force install to Python-Hostinstall so it does not
# match the automake *-install processing for installing into romfs,  we do
# not want to be installed in the firmware.

.PRECIOUS: build/Python-Host-build
build/Python-Host-build: build/Python-$(PYTHON_VERSION)-patched
	rm -rf build/Python-Host*
	cp -ra build/Python-$(PYTHON_VERSION) build/Python-Host
	ln -s Python-$(PYTHON_VERSION)-license build/Python-Host-license
	cd build/Python-Host; \
		export PKG_CONFIG_LIBDIR= PKG_CONFIG_PATH= PKG_CONFIG_SYSROOT_DIR= EXTRA_CFLAGS= CPUFLAGS= CPU_CFLAGS= CXXFLAGS= CPPFLAGS= LDLIBS= LIBC= LIBS= CC=gcc CXX=g++ LD=gcc AR= RANLIB= CFLAGS= LDFLAGS= ; \
		$(CONFVARS) autoreconf -i; \
		$(CONFVARS) ./configure \
			--prefix=`pwd`/../Python-Hostinstall \
			$(COMMON_CONFOPTS) \
			|| exit 1;  \
		make $(MAKEVARS) || exit 1; \
		make install $(MAKEVARS) || exit 1
	touch $@

PYTHON_LIVE_IMAGE_DIR = $(STAGEDIR)/live_images/python/rootfs


build/Python-$(PYTHON_VERSION)-my_installed: build/Python-$(PYTHON_VERSION)-installed
	@# Copy out sysconfig before editing, so we can use it in python-module building
	cp -a build/Python-$(PYTHON_VERSION)-install/usr/lib/python$(PYTHON_VERSION_SHORT)/_sysconfigdata_*.py build/
	@# - Remove host paths
	@# - Change cross-compiler binaries to native
	@# - Remove file-prefix-map compiler argument
	@# - Remove ucfront*
	sed -i 's|$(AR)|ar|g;s|$(CROSS_COMPILE)gcc|gcc|g;s|$(CROSS_COMPILE)g++|g++|g;s|$(CROSS_COMPILE)readelf|readelf|g;s|ucfront-[^ ]*||g;s|$(STAGEDIR)||g;s|$(ROOTDIR)|dal|g;s|-ffile-prefix-map=[^ ]*||g' \
		build/Python-$(PYTHON_VERSION)-install/usr/lib/python$(PYTHON_VERSION_SHORT)/_sysconfigdata_*.py \
		build/Python-$(PYTHON_VERSION)-install/usr/lib/python$(PYTHON_VERSION_SHORT)/config-$(PYTHON_VERSION_SHORT)-*/Makefile \
		build/Python-$(PYTHON_VERSION)-install/usr/bin/python$(PYTHON_VERSION_SHORT)-config
	touch $@

python_romfs:
ifdef CONFIG_USER_PYTHON
	$(ROMFSINST) -s python$(PYTHON_VERSION_SHORT) /usr/bin/python
	# Symlink dynamic library under '/lib' (otherwise uclibc dynamic loader does not find it)
	if [ -f "$(ROMFSDIR)/usr/lib/libpython$(PYTHON_VERSION_SHORT).so.1.0" ]; then \
		$(ROMFSINST) -s ../usr/lib/libpython$(PYTHON_VERSION_SHORT).so /lib/libpython$(PYTHON_VERSION_SHORT).so; \
		$(ROMFSINST) -s ../usr/lib/libpython$(PYTHON_VERSION_SHORT).so.1.0 /lib/libpython$(PYTHON_VERSION_SHORT).so.1.0; \
	fi
	# Remove not needed static libraries, Windows EXE binaries and header files
	if [ -d "$(ROMFSDIR)" ]; then \
		rm -f $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/distutils/command/*.exe; \
		find $(ROMFSDIR) -type f -name 'libpython*.a' -delete; \
		find $(ROMFSDIR)/usr/include/python$(PYTHON_VERSION_SHORT) -type f -not -name pyconfig.h -delete; \
	fi

ifdef CONFIG_USER_PYTHON_LIVE_IMAGE
	# Move Python binary to live image and install our shim on the host.
	mkdir -p $(PYTHON_LIVE_IMAGE_DIR)/usr/bin
	mv $(ROMFSDIR)/usr/bin/python$(PYTHON_VERSION_SHORT) $(PYTHON_LIVE_IMAGE_DIR)/usr/bin
	$(ROMFSINST) -p 755 /bin/mount_python_live_image.sh
	$(ROMFSINST) -p 755 python_live_image.sh /usr/bin/python$(PYTHON_VERSION_SHORT)

	# Copy Python libs to live image. Retain the empty directory and truncated
	# libs in romfs to provide mount points.
	mkdir -p $(PYTHON_LIVE_IMAGE_DIR)/usr/lib

	cp -r $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT) $(PYTHON_LIVE_IMAGE_DIR)/usr/lib/
	rm -fr $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/*

	cp $(ROMFSDIR)/usr/lib/libpython3.so $(PYTHON_LIVE_IMAGE_DIR)/usr/lib
	> $(ROMFSDIR)/usr/lib/libpython3.so

	cp $(ROMFSDIR)/usr/lib/libpython$(PYTHON_VERSION_SHORT).so.1.0 $(PYTHON_LIVE_IMAGE_DIR)/usr/lib
	> $(ROMFSDIR)/usr/lib/libpython$(PYTHON_VERSION_SHORT).so.1.0
endif
endif
