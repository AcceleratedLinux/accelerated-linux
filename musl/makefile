MUSL_VERSION = 1.2.0
MUSL = musl-$(MUSL_VERSION)
URL = http://www.musl-libc.org/releases/$(MUSL).tar.gz

CONFVARS += CROSS_COMPILE="$(CROSS_COMPILE)"

# Carry out any translations of kernel MACHINE to MUSL expected ARCH
MUSLARCH = $(subst arm64,aarch64,$(MACHINE))
MAKEVARS += ARCH=$(MUSLARCH)

CFLAGS =
LDFLAGS =

PATCHES = sys-cdefs.patch sys-queue.patch
ifdef CONFIG_LIB_MUSL_HAS_TZ_FILE
PATCHES += tz-file-path.patch
CFLAGS += -DTZ_FILE_PATH='\"$(CONFIG_LIB_MUSL_TZ_FILE_PATH)\"'
endif

$(MUSL)_CONFIGURE = ./configure
$(MUSL)_ROMFS = musl_romfs

include $(ROOTDIR)/tools/automake.inc

# Do not use any of the automake.inc CONFOPTS, they disable static libs
CONFOPTS := --prefix=
CONFOPTS += --target=$(MUSLARCH)

complete_target:
	rm -rf install
	ln -sf build/$(MUSL)-install install

LDSOGCC := $(shell $(CC) $(LIBGCC_CFLAGS) -v -Wl,-verbose 2>&1 | sed -n 's/.*-dynamic-linker *\([^ ]*\) .*$$/\1/p')

musl_romfs:
	set -e; if [ -x $(ROMFSDIR)/lib/libc.so ] ; then \
		$(ROMFSINST) -s /lib/libc.so $(LDSOGCC) ; \
	fi

