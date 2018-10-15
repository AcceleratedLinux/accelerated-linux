CONFOPTS=--disable-shared --with-tcl=$(ROOTDIR)/staging/lib --with-tclinclude=$(ROOTDIR)/user/tcl/build/tcl*/generic/

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; sh ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/expect /bin/expect
