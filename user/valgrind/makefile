VERSION = 3.22.0
URL = https://sourceware.org/pub/valgrind/valgrind-$(VERSION).tar.bz2

AUTOMAKE_ROMFS = valgrind_romfs

# valgrind butts heads with -fstack-protector
CFLAGS := $(filter-out -fstack-protector%,$(CFLAGS))

# this package includes some host tools
FINALTARGET = host
valgrind-$(VERSION)_CLEAN = host_clean

# Speed up builds by only building the tools requested.
# We disable all EXP_TOOLS (experimental) and ignore
# the lackey (demo) and nulgrind (selftest) tools.
# *note* When upgrading, check Makefile.am for new tools!
TOOLS-$(CONFIG_USER_VALGRIND_CACHEGRIND) += cachegrind
TOOLS-$(CONFIG_USER_VALGRIND_CALLGRIND)  += callgrind
TOOLS-$(CONFIG_USER_VALGRIND_DHAT)       += dhat
TOOLS-$(CONFIG_USER_VALGRIND_DRD)        += drd
TOOLS-$(CONFIG_USER_VALGRIND_HELGRIND)   += helgrind
TOOLS-$(CONFIG_USER_VALGRIND_MASSIF)     += massif
TOOLS-$(CONFIG_USER_VALGRIND_MEMCHECK)   += memcheck
MAKEVARS = TOOLS="$(TOOLS-y)" EXP_TOOLS=""

ifdef CONFIG_USER_VALGRIND_DISERVER
  # The bin/diserver wrapper is used to serve debug symbols
  # to the on-target valgrind tool.
  HOST_TOOLS += $(DISERVER_WRAPPER)
endif

# Valgrind supports these ISAs:
#   x86 amd64 arm arm64 mips32 mips64
#   nanomips ppc32 ppc64be ppc64le s390x
# (No support for: h8300 m68k microblaze riscv sh)
to_valgrind_arch = $(patsubst i386,x86,$(patsubst mips,mips32,$(patsubst x86_64,amd64,$1)))
VG_HOST_ARCH := $(call to_valgrind_arch,$(shell arch))
VG_ARCH       = $(call to_valgrind_arch,$(ARCH))

include $(ROOTDIR)/tools/automake.inc

valgrind_romfs: I=build/$(PKG)-install
valgrind_romfs: A=$(VG_ARCH)-linux
valgrind_romfs:
	$(ROMFSINST) -d $I/bin/valgrind /bin/valgrind
	mkdir -p $(ROMFSDIR)/libexec/valgrind
	$(ROMFSINST) $I/libexec/valgrind/vgpreload_core-$A.so /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_CACHEGRIND \
		$I/libexec/valgrind/cachegrind-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_CALLGRIND \
		$I/libexec/valgrind/callgrind-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_DHAT \
		$I/libexec/valgrind/dhat-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_DHAT \
		$I/libexec/valgrind/vgpreload_dhat-$A.so /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_DRD \
		$I/libexec/valgrind/drd-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_DRD \
		$I/libexec/valgrind/vgpreload_drd-$A.so /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_HELGRIND \
		$I/libexec/valgrind/helgrind-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_HELGRIND \
		$I/libexec/valgrind/vgpreload_helgrind-$A.so /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_MASSIF \
		$I/libexec/valgrind/massif-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_MASSIF \
		$I/libexec/valgrind/vgpreload_massif-$A.so /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_MEMCHECK \
		$I/libexec/valgrind/memcheck-$A /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_MEMCHECK \
		$I/libexec/valgrind/vgpreload_memcheck-$A.so  /libexec/valgrind/
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_VGDB \
		$I/bin/vgdb /bin/vgdb
	$(ROMFSINST) -e CONFIG_USER_VALGRIND_VGDB \
		$I/libexec/valgrind/getoff-$A  /libexec/valgrind/
	set -x; for f in \
		 $I/libexec/valgrind/$(VG_ARCH)-*.xml \
	         $I/libexec/valgrind/*.supp \
	  ; do $(ROMFSINST) $$f /libexec/valgrind/; done

DISERVER = build/$(PKG)-diserver
DISERVER_WRAPPER = $(ROOTDIR)/bin/diserver

$(DISERVER): B=build/$($(PKG)_BUILDDIR)
$(DISERVER): build/$(PKG)-built
	$(HOST_CC) -o $@ \
		-g \
		$B/auxprogs/valgrind-di-server.c \
		-I$B/coregrind \
		-I$B/include \
		-I$B/VEX/pub \
		-DVGO_linux -DVGA_$(VG_HOST_ARCH)

$(DISERVER_WRAPPER): diserver.in $(DISERVER)
	@mkdir -p $(@D)
	sed \
         -e 's;@DEBUGDIR@;$(or $(DEBUGDIR),/usr/lib/debug);g' \
         -e 's;@ROOTDIR@;$(abspath $(ROOTDIR));g' \
         -e 's;@DISERVER@;$(abspath $(DISERVER));g' \
	 <diserver.in >$@
	chmod +x $@

.PHONY: host_clean
host_clean:
	rm -f $(DISERVER_WRAPPER)
build/%-host: build/%-lndir $(HOST_TOOLS)
	@touch $@
