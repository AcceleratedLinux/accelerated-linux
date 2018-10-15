
BUILD = programs
URL = https://download.libreswan.org/libreswan-3.12.tar.gz
MAKEVARS = NSSFLAGS=-I$(STAGEDIR)/include/nss

CONFIGURE = :

# Pluto and whack need more than the default 4K stack; is this enough?
export FLTFLAGS = -s 262144

ifeq ($(findstring __uClinux__,$(CFLAGS))/$(findstring m68k,$(ARCH)),__uClinux__/m68k)
PATCH_SERIES = m68knommu-series
endif

include $(ROOTDIR)/tools/automake.inc

