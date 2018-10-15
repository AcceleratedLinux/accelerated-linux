
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS = --with-configfile=/etc/config/ips.cfg
else
CONFOPTS = --with-configfile=/etc/ips.cfg
endif

all: build/Makefile
	$(MAKE) -C build

build/Makefile: makefile
	rm -rf build
	mkdir build
	( \
		cd build; \
		CC="$(CC) $(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="$(LDLIBS)" \
			sh ../configure $(CONFIGURE_OPTS) \
			--disable-dietlibc --enable-release \
			$(CONFOPTS) \
			--enable-dependency-tracking \
			--with-username=root \
			; \
	)

romfs:
	$(ROMFSINST) build/src/ip-sentinel /bin/ip-sentinel

clean:
	rm -rf build

