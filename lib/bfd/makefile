CONFOPTS = --disable-nls

all: build/build
	$(MAKE) -C build SUBDIRS= MAKEINFO=/bin/true WARN_CFLAGS=

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; sh ../../../user/gdb/bfd/configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
