PKG = gcc-8.3.0
URL = https://ftp.gnu.org/gnu/gcc/$(PKG)/$(PKG).tar.xz

CONFOPTS += --target=$(TARGET)

ifdef PASS1
# GCC pass 1 needs minimal configuration
CONFOPTS += --disable-libatomic
CONFOPTS += --disable-libgomp
CONFOPTS += --disable-libmudflap
CONFOPTS += --disable-libquadmath
CONFOPTS += --disable-libsanitizer
CONFOPTS += --disable-libssp
CONFOPTS += --disable-threads
CONFOPTS += --disable-shared
CONFOPTS += --without-headers
CONFOPTS += --enable-languages=c
else
# GCC pass 2 is full build with g++
#CONFOPTS += --with-sysroot=$(STAGEDIR)
CONFOPTS += --disable-libsanitizer
CONFOPTS += --enable-languages=c,c++
endif

#ifdef ENDIAN
#CONFOPTS += --with-endian=$(ENDIAN)
#endif
#ifdef FLOAT
#CONFOPTS += --with-float=$(FLOAT)
#endif

CONFOPTS += --disable-libmpx
CONFOPTS += --disable-multilib
CONFOPTS += --with-system-zlib

CONFVARS = cd .. ; sh contrib/download_prerequisites ; cd $(TARGET) ;
CONFIGURE = ../configure

SRCDIR = $(PKG)
BUILDDIR = $(PKG)/$(TARGET)

MAKEVARS += -j$(HOST_NCPU)

include $(ROOTDIR)/tools/hostauto.inc

