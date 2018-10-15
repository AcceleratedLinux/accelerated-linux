
URL := ftp://ftp.mozilla.org/pub/mozilla.org/security/nss/releases/NSS_3_17_4_RTM/src/nss-3.17.4-with-nspr-4.10.7.tar.gz
BUILDDIR = nss-3.17.4

CONFIGURE = :

# NSS executables need more than a 4K stack; is this enough?
export FLTFLAGS = -s 65536

ifeq ($(findstring __uClinux__,$(CFLAGS))/$(findstring m68k,$(ARCH)),__uClinux__/m68k)
PATCH_SERIES = m68knommu-series
endif

include $(ROOTDIR)/tools/automake.inc

# Some of the patches alter configure.in, re-generate configure.
build/nss-3.17.4-with-nspr-4.10.7-configured: build/nspr-autoconf
build/nspr-autoconf: build/nss-3.17.4-with-nspr-4.10.7-patched
	cd build/$(BUILDDIR)/nspr && autoconf
	touch $@

