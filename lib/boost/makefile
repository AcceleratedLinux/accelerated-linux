#
# ./bootstrap.sh
#
# Edit Modify the configuration file (project-config.jam) to use the ARM
# toolchain by replacing the line with “using gcc” by:
#
# using gcc : arm : arm-linux-gnueabihf-g++ ;
#
# Build and install the boost libraries:
#
# ./bjam install toolset=gcc-arm --prefix=/usr/local/boost
#

URL := http://downloads.sourceforge.net/project/boost/boost/1.73.0/boost_1_73_0.tar.bz2

AUTORECONF = CC="$(HOSTCC)" CXX="$(HOSTCXX)" CXXFLAGS= \
	     CPUFLAGS= CFLAGS= LIBS= LDFLAGS= \
	     ./bootstrap.sh \
		--prefix=$(CURDIR)/build/boost_1_73_0-install \
		--with-toolset=gcc \
		--without-icu \
		--without-libraries=python \
		; \
	     mkdir -p ../bin; \
	     printf '\043!/bin/sh\nexec $(CXX) $(CFLAGS) $(CXXFLAGS) "\044@"\n' > ../bin/gcc_build; \
	      \
	     chmod +x ../bin/gcc_build; \
	     \
	     sed -i "s|using gcc|using gcc : : gcc_build |" project-config.jam

CONFIGURE  = not-a-real-configure-script

NODEFCONF = 1

BUILDTARGET = custombuild
INSTALLTARGET = custominstall

BUILDVARS = PATH="`pwd`/build/bin:$$PATH"

include $(ROOTDIR)/tools/automake.inc

build/boost_1_73_0-custombuild: build/boost_1_73_0-configured
	export PATH="`pwd`/build/bin:$$PATH"; cd build/boost_1_73_0; \
		./b2 -j$(HOST_NCPU) --no-cmake-config install
	touch $@

build/boost_1_73_0-custominstall: build/boost_1_73_0-custombuild
	[ -d build/boost_1_73_0-install ] || exit 1
	touch $@

