VERSION = 1.3
URL = https://www.lysator.liu.se/~nisse/misc/argp-standalone-$(VERSION).tar.gz

INSTALLTARGET = my_installed
include $(ROOTDIR)/tools/automake.inc

# - fPIC: build it as position-independent code, if it is linked in a static
#  library
# - fgnu89-inline: fix for GCC7
CFLAGS += -fPIC -fgnu89-inline

build/%-my_installed: build/%-$(BUILDTARGET)
		$(AT)mkdir -p build/$*-install/lib/
		cp build/$($*_BUILDDIR)/libargp.a build/$*-install/lib/
		$(AT)mkdir -p build/$*-install/include/
		cp build/$($*_BUILDDIR)/argp.h build/$*-install/include/
