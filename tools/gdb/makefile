PKG = gdb-9.1
URL = https://ftp.gnu.org/gnu/gdb/$(PKG).tar.xz

CONFOPTS += --target=$(TARGET)

CONFIGURE = ../configure

SRCDIR = $(PKG)
BUILDDIR = $(PKG)/$(TARGET)

MAKEVARS += -j$(HOST_NCPU)

include $(ROOTDIR)/tools/hostauto.inc

