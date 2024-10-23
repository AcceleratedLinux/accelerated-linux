VERSION = 1310.100.10
URL = https://opensource.apple.com/tarballs/mDNSResponder/mDNSResponder-$(VERSION).tar.gz

# Don't use "BUILDDIR" -- the package uses it in some parts of the build, but
# not all, which causes failures.
mDNSResponder-$(VERSION)_SRCDIR = mDNSResponder-mDNSResponder-$(VERSION)
mDNSResponder-$(VERSION)_BUILDDIR = mDNSResponder-mDNSResponder-$(VERSION)/mDNSEmbedLib

ifdef CONFIG_DEFAULTS_LIBC_UCLIBC
MAKEVARS = os=linux-uclibc
else
MAKEVARS = os=linux
endif

INSTBASE = `pwd`/build/$*-install
MAKEVARS += INSTBASE=$(INSTBASE)

mDNSResponder-$(VERSION)_INSTALLVARS = \
	mkdir -p $(INSTBASE)/lib $(INSTBASE)/include $(INSTBASE)/sbin $(INSTBASE)/bin;

include $(ROOTDIR)/tools/automake.inc
