ACL_LICENSE = Digi
ACL_URL     = http://www.digi.com/
ACL_PKG     = Digi Accelerated Linux
export ACL_PKG ACL_URL ACL_LICENSE

#
# build using configure as best we can, should work for most targets
# David McCullough <davidm@snapgear.com>
#

all: build

build/makefile: makefile makefile.in
	rm -rf build
	mkdir build
	chmod u+x configure
	( \
		cd build; \
		CC="$(CC) $(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		LIBS="" \
		/bin/bash ../configure $(CONFIGURE_OPTS) --prefix= ; \
	)

.PHONY: build
build: build/makefile
	$(MAKE) -C build all install

.PHONY: romfs
romfs:
	$(ROMFSINST) build/libopengear.so /lib/libopengear.so
ifdef CONFIG_PROP_LH5_NODE
	$(ROMFSINST) -d build/libopengear_lua.so /lib/lua/libopengear_lua.so
endif

.PHONY: clean
clean:
	rm -rf build

.PHONY: check
check: build
	$(MAKE) -C build check
