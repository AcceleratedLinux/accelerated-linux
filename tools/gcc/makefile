PKG = gcc-13.2.0
URL = https://ftp.gnu.org/gnu/gcc/$(PKG)/$(PKG).tar.xz

CONFOPTS += --target=$(TARGET)

ifdef PASS1
# GCC pass 1 needs minimal configuration
CONFOPTS += --disable-libatomic
CONFOPTS += --disable-libgomp
CONFOPTS += --disable-libmudflap
CONFOPTS += --disable-libquadmath
CONFOPTS += --disable-libssp
CONFOPTS += --disable-threads
CONFOPTS += --disable-shared
CONFOPTS += --disable-bootstrap
CONFOPTS += --without-headers
CONFOPTS += --enable-languages=c
else
# GCC pass 2 is full build with g++
CONFOPTS += --enable-languages=c,c++
endif

#ifdef ENDIAN
#CONFOPTS += --with-endian=$(ENDIAN)
#endif
#ifdef FLOAT
#CONFOPTS += --with-float=$(FLOAT)
#endif
ifdef MABI
CONFOPTS += --with-abi=$(MABI)
endif

CONFOPTS += --disable-libsanitizer
CONFOPTS += --disable-libmpx
CONFOPTS += --disable-multilib
CONFOPTS += --with-system-zlib

CONFVARS = cd .. ; sh contrib/download_prerequisites ; cd $(TARGET) ;
CONFIGURE = ../configure

SRCDIR = $(PKG)
BUILDDIR = $(PKG)/$(TARGET)

MAKEVARS += -j$(HOST_NCPU)

include $(ROOTDIR)/tools/hostauto.inc

