UCLIBC_VERSION = 1.0.22
UCLIBC = uClibc-ng-$(UCLIBC_VERSION)
URL = http://downloads.uclibc-ng.org/releases/$(UCLIBC_VERSION)/$(UCLIBC).tar.xz

$(UCLIBC)_CONFVARS = cp ../../.config . ;
$(UCLIBC)_CONFVARS += ( cd ../.. ; rm -f lib ; ln -sf build/$(UCLIBC)/lib lib ) ;
$(UCLIBC)_CONFVARS += $(MAKE) oldconfig ; :
$(UCLIBC)_CONFIGURE = Makefile
$(UCLIBC)_ROMFS = uclibc_romfs
$(UCLIBC)_CLEAN = uclibc_clean

include $(ROOTDIR)/tools/automake.inc

oldconfig:

install_headers:
	$(MAKE) -C build/$(UCLIBC) install_headers PREFIX=$(PREFIX)

install:
	$(MAKE) -C build/$(UCLIBC) install PREFIX=$(STAGEDIR)/ DEVEL_PREFIX= RUNTIME_PREFIX=

LDSOGCC := $(shell $(CC) $(LIBGCC_CFLAGS) -v -Wl,-verbose 2>&1 | sed -n 's/.*-dynamic-linker *\([^ ]*\) .*$$/\1/p')

uclibc_romfs:
	set -e; if [ -x lib/ld-uClibc-$(UCLIBC_VERSION).so ] ; then \
		$(ROMFSINST) -p 755 lib/ld-uClibc-$(UCLIBC_VERSION).so /lib; \
		$(ROMFSINST) -s \
			/lib/ld-uClibc-$(UCLIBC_VERSION).so \
			$(LDSOGCC) ; \
	fi
	set -e; if [ -x lib/ld64-uClibc-$(UCLIBC_VERSION).so ] ; then \
		$(ROMFSINST) -p 755 lib/ld64-uClibc-$(UCLIBC_VERSION).so /lib64; \
		$(ROMFSINST) -s \
			/lib/ld64-uClibc-$(UCLIBC_VERSION).so \
			$(LDSOGCC) ; \
	fi

uclibc_clean:
	rm -f lib

