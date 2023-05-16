
URL = http://micropython.org/resources/source/micropython-1.19.1.tar.xz

MAKEVARS = CC="$(CC)"
MAKEVARS += V=1
MAKEVARS += PREFIX=/usr
MAKEVARS += MICROPY_PY_BTREE=0
MAKEVARS += MICROPY_PY_FFI=0 MICROPY_PY_SOCKET=0
MAKEVARS += MICROPY_PY_THREAD=0
MAKEVARS += MICROPY_PY_TERMIOS=0
MAKEVARS += MICROPY_PY_USSL=0
MAKEVARS += MICROPY_USE_READLINE=0

micropython-1.19.1_BUILDDIR = micropython-1.19.1/ports/unix
micropython-1.19.1_AUTORECONF = cd ../../mpy-cross ; unset CC CXX LD STRIP CFLAGS CXXFLAGS LDFLAGS ; make V=1 CC=$(HOST_CC) STRIP="strip" SIZE="size"

include $(ROOTDIR)/tools/automake.inc
