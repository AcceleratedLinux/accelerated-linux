URL = https://ftp.mozilla.org/pub/nspr/releases/v4.11/src/nspr-4.11.tar.gz

NVR = $(subst -, , $(patsubst %.tar.gz,%,$(notdir $(URL))))
PKG = $(word 1,$(NVR))
VERSION = $(word 2,$(NVR))
SRCDIR = $(PKG)-$(VERSION)
BUILDDIR = $(SRCDIR)/nspr

# $(AUTORECONF) is run from $(SRCDIR), not $(BUILDDIR)
AUTORECONF = cd nspr ; autoconf

include $(ROOTDIR)/tools/automake.inc

# Disable build of shared library.  Should this be controled by
# automake.mk somehow?
MAKEVARS = 'SHARED_LIBRARY='

# When it comes to the definition of build, host, and target, Mozilla
# are still living in the 90's: target->[] build->host host->target.
CONFIGURE_OPTS:=$(patsubst --build=%,--host=%,$(patsubst --host=%,--target=%,$(patsubst --target=%,,$(CONFIGURE_OPTS))))
