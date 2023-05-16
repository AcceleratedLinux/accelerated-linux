VERSION = libcap-2.25
URL = git://git.kernel.org/pub/scm/linux/kernel/git/morgan/libcap.git
METHOD = git

export ACL_LICENSE = BSD/GPLv2

# Copy uapi capabilities.h from current kernel so the build can process it
BUILDVARS = cp -f --remove-destination $(ROOTDIR)/linux/include/uapi/linux/capability.h \
	build/$($*_BUILDDIR)/libcap/include/uapi/linux/capability.h;

MAKEVARS  = BUILD_CC="$(BUILD_CC)"
MAKEVARS += CC="$(CC)"
MAKEVARS += CFLAGS="$(CFLAGS)"
MAKEVARS += LDFLAGS="$(LDFLAGS)"
MAKEVARS += lib=lib prefix=/
MAKEVARS += BUILD_GPERF=no PAM_CAP=no RAISE_SETFCAP=no

include $(ROOTDIR)/tools/automake.inc
