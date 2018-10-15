
PYTHON_VERSION = 3.6.4
URL = http://www.python.org/ftp/python/$(PYTHON_VERSION)/Python-$(PYTHON_VERSION).tar.xz

PYTHON_VERSION_SHORT = $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

CONFVARS += ac_cv_prog_READELF=$(CROSS_COMPILE)readelf
CONFVARS += ac_cv_buggy_getaddrinfo=no
CONFVARS += ac_cv_file__dev_ptmx=yes
CONFVARS += ac_cv_file__dev_ptc=no
CONFVARS += ac_cv_have_long_long_format=yes

Python-$(PYTHON_VERSION)_CONFOPTS += --disable-codecs-cjk
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-curses
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-idle3
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-lib2to3
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-nis
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-ossaudiodev
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-pydoc
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-test-modules
Python-$(PYTHON_VERSION)_CONFOPTS += --disable-tk
Python-$(PYTHON_VERSION)_CONFOPTS += --enable-shared
Python-$(PYTHON_VERSION)_CONFOPTS += --prefix=/usr
Python-$(PYTHON_VERSION)_CONFOPTS += --without-ensurepip
Python-$(PYTHON_VERSION)_CONFOPTS += --without-system-ffi
Python-$(PYTHON_VERSION)_CONFOPTS += --with-pymalloc

Python-$(PYTHON_VERSION)_ROMFS = python_romfs

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl
CPPFLAGS += -I$(STAGEDIR)/include
LDFLAGS += -L$(STAGEDIR)/lib

AUTORECONF = autoreconf -i

include $(ROOTDIR)/tools/automake.inc

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
