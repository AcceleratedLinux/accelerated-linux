GLIBC_VERSION = 2.39

URL = https://ftp.gnu.org/gnu/glibc/glibc-$(GLIBC_VERSION).tar.xz

CONFOPTS += --without-selinux
CONFOPTS += --disable-sanity-checks
CONFOPTS += --enable-obsolete-rpc
CONFOPTS += --with-headers=$(STAGEDIR)/include
CONFOPTS += --prefix=

CFLAGS =
LDFLAGS =

# i386 is deprecated in glibc, so choose new 686 instead
ifeq ($(TARGETARCH),i386-linux)
override TARGETARCH = i686-linux
CFLAGS += -march=i686 -O2
endif

# Override the default to remove "--target" not wanted by glibc
override CONFIGURE_OPTS = --host=$(TARGETARCH) --build=$(HOSTARCH)

CONFVARS += libc_cv_ssp=no
CONFVARS += CC="$(subst ucfront-gcc,,$(CC))"
CONFVARS += CXX="$(subst ucfront-g++,,$(CXX))"

glibc-$(GLIBC_VERSION)_CONFIGURE = ../glibc-$(GLIBC_VERSION)/configure
glibc-$(GLIBC_VERSION)_SRCDIR = glibc-$(GLIBC_VERSION)
glibc-$(GLIBC_VERSION)_BUILDDIR = glibc-$(GLIBC_VERSION)-build
glibc-$(GLIBC_VERSION)_BUILD = all
glibc-$(GLIBC_VERSION)_INSTALL = MAKEFILE_LIST= install

# hack to reduce environment size or build fails in deep directories
BUILDVARS = env - PATH='$(PATH)' CC='$(CC)' CXX='$(CXX)'

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
