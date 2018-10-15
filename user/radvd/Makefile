
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
endif

all: build/build
	$(MAKE) -C build

build/build: Makefile
	touch gram.y
	touch aclocal.m4
	touch Makefile.in
	rm -rf build
	mkdir build
	(cd build; CFLAGS="$(CFLAGS) -D_GNU_SOURCE=1" sh ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) etc /etc
	$(ROMFSINST) -e CONFIG_USER_RADVD_RADVD build/radvd /bin/radvd
	$(ROMFSINST) -e CONFIG_USER_RADVD_RADVD build/radvdump /bin/radvdump
