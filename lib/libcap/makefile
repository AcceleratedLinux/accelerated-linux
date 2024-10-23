VERSION = libcap-2.70
URL = git://git.kernel.org/pub/scm/libs/libcap/libcap.git
METHOD = git

export ACL_LICENSE = BSD/GPLv2

# Copy uapi capabilities.h from current kernel so the build can process it
BUILDVARS = cp -f --remove-destination $(ROOTDIR)/linux/include/uapi/linux/capability.h \
	build/$($*_BUILDDIR)/libcap/include/uapi/linux/capability.h;

MAKEVARS  = BUILD_CC="$(BUILD_CC)"
MAKEVARS += CC="$(CC)"
MAKEVARS += prefix=/
MAKEVARS += lib=lib
MAKEVARS += USE_GPERF=no
MAKEVARS += PAM_CAP=no
MAKEVARS += RAISE_SETFCAP=no
MAKEVARS += GOLANG=no

include $(ROOTDIR)/tools/automake.inc
