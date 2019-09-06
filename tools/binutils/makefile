PKG = binutils-2.32
URL = https://ftp.gnu.org/gnu/binutils/$(PKG).tar.xz

CONFOPTS += --target=$(TARGET)

CONFIGURE = ../configure

SRCDIR = $(PKG)
BUILDDIR = $(PKG)/$(TARGET)

MAKEVARS += -j$(HOST_NCPU)

include $(ROOTDIR)/tools/hostauto.inc

