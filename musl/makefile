MUSL_VERSION = 1.2.5
MUSL = musl-$(MUSL_VERSION)
URL = https://musl.libc.org/releases/$(MUSL).tar.gz

CONFVARS += CROSS_COMPILE="$(CROSS_COMPILE)"

# Carry out any translations of kernel MACHINE to MUSL expected ARCH
MUSLARCH = $(subst arm64,aarch64,$(MACHINE))
MAKEVARS += ARCH=$(MUSLARCH)

CFLAGS =
LDFLAGS =

PATCHES = sys-cdefs.patch sys-queue.patch
PATCHES += sem_clockwait.patch
ifdef CONFIG_LIB_MUSL_HAS_TZ_FILE
PATCHES += tz-file-path.patch
CFLAGS += -DTZ_FILE_PATH='\"$(CONFIG_LIB_MUSL_TZ_FILE_PATH)\"'
endif

$(MUSL)_CONFIGURE = ./configure
$(MUSL)_ROMFS = musl_romfs

ifdef CONFIG_LIBC_NOCRYPT
 CONFIGTARGET = configured-fixup
endif

include $(ROOTDIR)/tools/automake.inc

# When an alternate -lcrypt is available, prevent the installation
# of musl's empty stub libcrypt.a and header include/crypt.h.
# We also prevent musl's src/crypt/*.c from being compiled
# directly into libc.so.
ifdef CONFIG_LIBC_NOCRYPT
 build/%-configured-fixup: build/%-configured
	@echo "Configure-fixup $* ..."
	rm -rf build/$($*_BUILDDIR)/src/crypt
	rm -f  build/$($*_BUILDDIR)/include/crypt.h
	$(AT)touch $@
endif

# Do not use any of the automake.inc CONFOPTS, they disable static libs
CONFOPTS := --prefix=
CONFOPTS += --target=$(MUSLARCH)

# Frame pointers are very helpful when debugging
ifeq ($(CONFIG_LIB_DEBUG),y)
 CONFOPTS += --enable-debug
endif

complete_target:
	rm -rf install
	ln -sf build/$(MUSL)-install install
ifdef CONFIG_LIBC_NOCRYPT
	-rm install/lib/libcrypt.a
endif

LDSOGCC := $(shell $(CC) $(LIBGCC_CFLAGS) -v -Wl,-verbose 2>&1 | sed -n 's/.*-dynamic-linker *\([^ ]*\) .*$$/\1/p')

musl_romfs:
	set -e; if [ -x $(ROMFSDIR)/lib/libc.so ] ; then \
		$(ROMFSINST) -s /lib/libc.so $(LDSOGCC) ; \
	fi

