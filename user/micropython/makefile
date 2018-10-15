
URL = http://micropython.org/resources/source/micropython-1.9.2.tar.xz
MAKEVARS = CC="$(CC)" V=1 PREFIX=$(shell pwd)/build/micropython-1.9.2-install/ FROZEN_DIR= FROZEN_MPY_DIR= \
           BUILD=build-minimal PROG=micropython \
           MICROPY_PY_BTREE=0 MICROPY_PY_FFI=0 MICROPY_PY_SOCKET=0 \
           MICROPY_PY_THREAD=0 \
           MICROPY_PY_TERMIOS=0 MICROPY_PY_USSL=0 \
           MICROPY_USE_READLINE=0 \
	   COPT="-Os -DNDEBUG" \
	   CFLAGS_EXTRA='-DMP_CONFIGFILE="<mpconfigport_minimal.h>"'
PATCH_LEVEL = -p0 -d .. -b

micropython-1.9.2_BUILDDIR = micropython-1.9.2/unix
micropython-1.9.2_AUTORECONF  = make axtls $(MAKEVARS)
micropython-1.9.2_BUILD  = micropython

include $(ROOTDIR)/tools/automake.inc
