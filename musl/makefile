MUSL_VERSION = 1.1.19
MUSL = musl-$(MUSL_VERSION)
URL = http://www.musl-libc.org/releases/$(MUSL).tar.gz

CONFVARS += CC="$(subst ucfront-gcc,,$(CC))"
CONFVARS += CXX="$(subst ucfront-g++,,$(CXX))"
CONFVARS += LD="$(subst ucfront-ld,,$(LD))"

CFLAGS =
LDFLAGS =

$(MUSL)_CONFIGURE = ./configure
$(MUSL)_ROMFS = musl_romfs

include $(ROOTDIR)/tools/automake.inc

complete_target:
	rm -f install
	ln -sf build/$(MUSL)-install install

LDSOGCC := $(shell $(CC) $(LIBGCC_CFLAGS) -v -Wl,-verbose 2>&1 | sed -n 's/.*-dynamic-linker *\([^ ]*\) .*$$/\1/p')

musl_romfs:
	set -e; if [ -x $(ROMFSDIR)/lib/libc.so ] ; then \
		$(ROMFSINST) -s /lib/libc.so $(LDSOGCC) ; \
	fi

