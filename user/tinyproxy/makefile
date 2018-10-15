# Configure options.

CONFOPTS :=	--enable-dependency-tracking		\
		--enable-transparent-proxy
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config --with-config=/etc/config/tinyproxy.conf
else
CONFOPTS += --with-config=/etc/tinyproxy.conf
endif

all: build/build
	$(MAKE) -C build

build/build: makefile
	@chmod u+x configure
	@rm -rf build
	@mkdir build
	(cd build; sh ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	@touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/src/tinyproxy /bin/tinyproxy
