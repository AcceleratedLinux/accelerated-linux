-include ../user.mak

CONFOPTS=--disable-threads

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	touch makefile
	find . -type f | xargs touch -r makefile
	(cd build; ac_cv_func_malloc_0_nonnull=yes LIBS="$(CXXLIBS)" sh ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/src/iperf /bin/iperf
