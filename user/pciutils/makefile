PCIUTILS_VERSION = 3.4.0
URL = https://www.kernel.org/pub/software/utils/pciutils/pciutils-$(PCIUTILS_VERSION).tar.xz

MAKEVARS = HOST="$(ARCH)-linux"
MAKEVARS += CC="$(CC)"
MAKEVARS += AR="$(AR)"
MAKEVARS += RANLIB="$(RANLIB)" 
MAKEVARS += PREFIX=""
MAKEVARS += SHAREDIR="/usr/share"
MAKEVARS += STRIP="-s --strip-program='$(STRIP)'"

include $(ROOTDIR)/tools/automake.inc

