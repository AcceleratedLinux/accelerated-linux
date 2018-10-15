.EXPORT_ALL_VARIABLES:

CONFOPTS = NM=$(CROSS)nm OBJDUMP=$(CROSS)objdump --without-afs --without-libpam
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config/
endif

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
	$(ROMFSINST) -d build/src/.libs/pam_krb5.so /lib/security/pam_krb5.so
