PKG = gdb-9.1
URL = https://ftp.gnu.org/gnu/gdb/$(PKG).tar.xz

SRCDIR = $(PKG)
BUILDDIR = $(PKG)/$(TARGETARCH)
CONFIGURE = ../configure

MAKEVARS = MAKEINFO=true

CONFOPTS += --with-system-readline

include $(ROOTDIR)/tools/automake.inc

