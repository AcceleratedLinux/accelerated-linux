URL = https://fast.dpdk.org/rel/dpdk-16.11.1.tar.xz

dpdk-16.11.1_BUILDDIR = dpdk-stable-16.11.1

#
# The DPDK has a weird initial config mechanism. You need to config with
# tuples that specify archiecture, optimization options and compile type.
# "native" is the default used to build for the host dev machine - this is
# almost never going to be correct for us building cross-compile type
# builds in the dist. So the default config, set as "unknown", is designed
# to fail on purpose.
#
# Typically I would expect board specific configs to be set. So for example
# on the 9400-UA we set DPDK_CONFIG to "x86_64-btver2-linuxapp-gcc". Note
# that I had to add a config file to support this (in the patches).
# I expect this to often be the case.
#
ifeq ($(ARCH),arm)
DPDK_CONFIG ?= arm-armv7a-linuxapp-gcc 
endif
ifeq ($(ARCH),arm64)
DPDK_CONFIG ?= arm64-armv8a-linuxapp-gcc 
endif
DPDK_CONFIG ?= unknown

AUTOCONF = $(MAKE) config T=$(DPDK_CONFIG)

KVERSION=$(shell cat $(ROOTDIR)/$(LINUXDIR)/include/config/kernel.release)
BUILDVARS = RTE_KERNELDIR=$(ROOTDIR)/$(LINUXDIR)
BUILDVARS += kerneldir=/lib/modules/$(KVERSION)/extra/dpdk
BUILDVARS += prefix="/usr"

CC += -D_GNU_SOURCE
ifeq ($(CONFIG_LIBCDIR),uClibc)
LDLIBS += -lubacktrace
endif

include $(ROOTDIR)/tools/automake.inc

