
ptmalloc3_URL = http://www.malloc.de/malloc/ptmalloc3-current.tar.gz
PACKAGE = ptmalloc3
PKG_y = $(PACKAGE)

MAKEVARS = linux-shared

INSTALLTARGET = installed2

include $(ROOTDIR)/tools/automake.inc

SRCDIR=`pwd`/build/$(PACKAGE)
DESTDIR=`pwd`/build/$(PACKAGE)-install

build/ptmalloc3-installed2: build/$(PACKAGE)-$(BUILDTARGET)
	@echo "Installing $(PACKAGE) ..."
	install -d $(DESTDIR)/lib
	install -m 755 $(SRCDIR)/lib$(PACKAGE).so $(DESTDIR)/lib
