MUSL_VERSION = 1.1.22
MUSL = musl-$(MUSL_VERSION)
URL = http://www.musl-libc.org/releases/$(MUSL).tar.gz

CONFVARS += CC="$(subst ucfront-gcc,,$(CC))"
CONFVARS += CXX="$(subst ucfront-g++,,$(CXX))"
CONFVARS += LD="$(subst ucfront-ld,,$(LD))"

# Carry out any translations of kernel ARCH to MUSL expected ARCH
MUSLARCH = $(subst arm64,aarch64,$(ARCH))
MAKEVARS += ARCH=$(MUSLARCH)

CFLAGS =
LDFLAGS =

$(MUSL)_CONFIGURE = ./configure
$(MUSL)_ROMFS = musl_romfs

include $(ROOTDIR)/tools/automake.inc

# Do not use any of the automake.inc CONFOPTS, they disable static libs
CONFOPTS := --prefix=

complete_target:
	rm -f install
	ln -sf build/$(MUSL)-install install

LDSOGCC := $(shell $(CC) $(LIBGCC_CFLAGS) -v -Wl,-verbose 2>&1 | sed -n 's/.*-dynamic-linker *\([^ ]*\) .*$$/\1/p')

musl_romfs:
	set -e; if [ -x $(ROMFSDIR)/lib/libc.so ] ; then \
		$(ROMFSINST) -s /lib/libc.so $(LDSOGCC) ; \
	fi

