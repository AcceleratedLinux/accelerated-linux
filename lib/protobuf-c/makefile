VERSION = 1.5.0
URL = https://github.com/protobuf-c/protobuf-c/releases/download/v$(VERSION)/protobuf-c-$(VERSION).tar.gz
CONFOPTS += --prefix=$(ROOTDIR)/staging
PREFIXDIR = $(ROOTDIR)/staging

PROTOC_HOST_PATH = $(ROOTDIR)/lib/protobuf/build/protobuf-Hostinstall
PROTOC=$(PROTOC_HOST_PATH)/bin/protoc

include $(ROOTDIR)/tools/automake.inc

PKG_y += protobuf-c-$(VERSION)
PKG_y += protobuf-c-Host

protobuf-c-Host_FINALTARGET = build
protobuf-c-Host_METHOD = none

all: build/protobuf-c-Host-build build/protobuf-c-$(VERSION)

.PRECIOUS: build/protobuf-c-Host-build
build/protobuf-c-Host-build: build/protobuf-c-$(VERSION)-patched
	rm -rf build/protobuf-c-Host*
	cp -ra build/protobuf-c-$(VERSION) build/protobuf-c-Host
	ln -s protobuf-c-$(VERSION)-license build/protobuf-c-Host-license
	cd build/protobuf-c-Host; \
		unset PKG_CONFIG_LIBDIR PKG_CONFIG_PATH PKG_CONFIG_SYSROOT_DIR EXTRA_CFLAGS CPUFLAGS CPU_CFLAGS CXXFLAGS CPPFLAGS LDLIBS LIBC LIBS CC CXX LD NM STRIP AR RANLIB CFLAGS LDFLAGS LIBTOOL ; \
		export PROTOC=$(PROTOC) PKG_CONFIG_PATH=$(PROTOC_HOST_PATH)/lib/pkgconfig ; \
		make clean; \
		$(CONFVARS) autoreconf -i --force; \
		$(CONFVARS) ./configure --prefix=`pwd`/../protobuf-c-Hostinstall $(COMMON_CONFOPTS) --enable-shared=no --enable-static=yes || exit 1;  \
		make || exit 1; \
		make install || exit 1
	touch $@
