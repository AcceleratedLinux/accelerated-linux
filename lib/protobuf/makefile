VERSION = 3.21.12
URL = https://github.com/protocolbuffers/protobuf/archive/refs/tags/v$(VERSION).tar.gz
AUTORECONF:= ./autogen.sh
LDFLAGS += -latomic
MAKEVARS = -j$(HOST_NCPU)

PKG_y += protobuf-$(VERSION)
PKG_y += protobuf-Host

protobuf-Host_FINALTARGET = build
protobuf-Host_METHOD = none

all: build/protobuf-Host-build build/protobuf-$(VERSION)

.PRECIOUS: build/protobuf-Host-build
build/protobuf-Host-build: build/protobuf-$(VERSION)-patched
	rm -rf build/protobuf-Host*
	cp -ra build/protobuf-$(VERSION) build/protobuf-Host
	ln -s protobuf-$(VERSION)-license build/protobuf-Host-license
	cd build/protobuf-Host; \
		unset PKG_CONFIG_LIBDIR PKG_CONFIG_PATH PKG_CONFIG_SYSROOT_DIR EXTRA_CFLAGS CPUFLAGS CPU_CFLAGS CXXFLAGS CPPFLAGS LDLIBS LIBC LIBS CC CXX LD AR NM STRIP RANLIB CFLAGS LDFLAGS ; \
		make clean ; \
		$(CONFVARS) autoreconf -i; \
		$(CONFVARS) ./configure \
			--prefix=`pwd`/../protobuf-Hostinstall --enable-shared=protoc \
			$(COMMON_CONFOPTS) \
			|| exit 1;  \
		make $(MAKEVARS) || exit 1; \
		make install $(MAKEVARS) || exit 1
	touch $@

include $(ROOTDIR)/tools/automake.inc

