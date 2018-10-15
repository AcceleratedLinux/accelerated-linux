#
# Toolchain issues prevent use of new glibc on most targets currently.
# So select an appropriate version for some architectures. Right now we
# only want to user a newer glibc on arm64 targets.
#
ifeq ($(TARGETARCH),aarch64-linux)
GLIBC_VERSION = 2.26
else
GLIBC_VERSION = 2.23
endif

URL = http://ftp.gnu.org/gnu/glibc/glibc-$(GLIBC_VERSION).tar.bz2

CONFOPTS += --disable-sanity-checks
CONFOPTS += --enable-obsolete-rpc
CONFOPTS += --with-headers=$(STAGEDIR)/include
CONFOPTS += --prefix=

CFLAGS =
LDFLAGS =

# i386 is depracated in glibc, so choose new 686 instead
ifeq ($(TARGETARCH),i386-linux)
TARGETARCH = i686-linux
CFLAGS += -march=i686 -O2
CONFIGURE_OPTS = --host=$(TARGETARCH) --build=$(HOSTARCH)
endif

CONFVARS += libc_cv_ssp=no
CONFVARS += CC="$(subst ucfront-gcc,,$(CC))"
CONFVARS += CXX="$(subst ucfront-g++,,$(CXX))"

glibc-$(GLIBC_VERSION)_CONFIGURE = ../glibc-$(GLIBC_VERSION)/configure
glibc-$(GLIBC_VERSION)_BUILDDIR = glibc-$(GLIBC_VERSION)-build
glibc-$(GLIBC_VERSION)_BUILD = all
glibc-$(GLIBC_VERSION)_INSTALL = MAKEFILE_LIST= install

include $(ROOTDIR)/tools/automake.inc

complete_target:
	rm -f install
	ln -sf build/glibc-$(GLIBC_VERSION)-install install
	find build/glibc-$(GLIBC_VERSION)-install/. -type f -name *.so* | \
	while read t ; do \
		if file "$$t" | grep -i ascii > /dev/null; then \
			echo "Fixing $$t ..."; \
			sed 's?\([      (]\)\(/lib/[^   )]*\)?\1'"`pwd`/install"'\2?g' < "$$t" > "$$t.fixup"; \
			cp "$$t.fixup" "$$t"; \
			rm -f "$$t.fixup"; \
		fi \
	done
