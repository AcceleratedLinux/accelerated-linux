PKG = gdb-14.2
URL = https://ftp.gnu.org/gnu/gdb/$(PKG).tar.xz

PKG_$(CONFIG_USER_GDB_HOST)      += $(PKG)-host
PKG_$(CONFIG_USER_GDB_TARGET)    += $(PKG)
PKG_$(CONFIG_USER_GDB_GDBSERVER) += $(PKG)
PKG_$(CONFIG_USER_GDB_GCORE)     += $(PKG)
PKG_y := $(sort $(PKG_y))

AUTOMAKE_ROMFS = gdb_romfs

# Use one source directory with different target and host build dirs
SRCDIR := $(PKG)
CONFIGURE = ../$(SRCDIR)/configure
$(PKG)-host_BUILDDIR = $(PKG)-host-build-$(TARGETARCH)
$(PKG)_BUILDDIR = $(PKG)-build-$(TARGETARCH)

# Common config
CONFOPTS += --target=$(TARGETARCH)
CONFOPTS += --disable-nls

# Target config
$(PKG)_CONFOPTS += --with-system-readline
$(PKG)_CONFOPTS += $(if $(CONFIG_USER_GDB_GDBSERVER),--enable-gdbserver)
$(PKG)_CONFOPTS += --enable-static
LDFLAGS += -ltinfo

# Host config
$(PKG)-host_CONFOPTS = --prefix=
$(PKG)-host_CONFVARS = CC="$(HOST_CC)" CXX="$(HOST_CXX)"
$(PKG)-host_CONFVARS += RANLIB= AR= STRIP= LDFLAGS= CXXFLAGS= CPPFLAGS=
$(PKG)-host_NODEFCONF = true
$(PKG)-host_BUILDVARS = $($(PKG)-host-CONFVARS)
$(PKG)-host_FINALTARGET = hostinstalled

# Avoid building gdb for the common case of just wanting gdbserver.
# This only saves about 10% time.
PARTS_CONFIG = $(strip $(foreach o,TARGET GDBSERVER GCORE,$(CONFIG_USER_GDB_$o:%=$o)))
$(info PARTS_CONFIG is "$(PARTS_CONFIG)")
ifeq ($(PARTS_CONFIG),GDBSERVER)
 $(PKG)_BUILD = all-gdbserver
 $(PKG)_INSTALL = install-gdbserver
endif

# Don't build documentation
MAKEVARS = MAKEINFO=true

include $(ROOTDIR)/tools/automake.inc

# Share source recipes when both host and target are enabled
ifneq ($(and $(filter $(PKG),$(PKG_y)),$(filter $(PKG)-host,$(PKG_y))),)
  build/$(PKG)-host-autoconf: build/$(PKG)-autoconf
	$(AT)touch $@
endif

build/$(PKG)-host-hostinstalled: build/$(PKG)-host-installed
	cp $(abspath build/$(PKG)-host-install)/bin/gdb \
		$(ROOTDIR)/bin/gdb-$(TARGETARCH)
.PHONY: build/$(PKG)-host-hostinstalled

# Target CFLAGS gets leaked into the host gdb configure recipe; defeat that
build/$(PKG)-host-configured: CFLAGS=

gdb_romfs:
	$(ROMFSINST) -e CONFIG_USER_GDB_TARGET    build/$(PKG)-install/bin/gdb /bin/
	$(ROMFSINST) -e CONFIG_USER_GDB_GDBSERVER build/$(PKG)-install/bin/gdbserver /bin/
	$(ROMFSINST) -e CONFIG_USER_GDB_GCORE     build/$(PKG)-install/bin/gcore /bin/
