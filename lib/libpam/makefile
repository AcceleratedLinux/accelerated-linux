
CONFOPTS := --enable-dependency-tracking
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
DEFAULTDIR = default
CFLAGS += -DCONFIG_USER_FLATFSD_ETC_CONFIG
else
DEFAULTDIR = default
endif
CONFOPTS += --disable-nls --disable-largefile

CC += -DHOST_NAME_MAX=255

CFLAGS += -DOMIT_NIS

ifdef CONFIG_PROP_STATSD_STATSD
CFLAGS += -DPAM_STATS
endif

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/libpamc/.libs/libpamc.so.0.81.0 /lib/libpamc.so.0.81.0
	$(ROMFSINST) -s libpamc.so.0.81.0 /lib/libpamc.so.0
	$(ROMFSINST) -s libpamc.so.0.81.0 /lib/libpamc.so
	$(ROMFSINST) build/libpam/.libs/libpam.so.0.81.2 /lib/libpam.so.0.81.2
	$(ROMFSINST) -s libpam.so.0.81.2 /lib/libpam.so.0
	$(ROMFSINST) -s libpam.so.0.81.2 /lib/libpam.so
	$(ROMFSINST) build/libpam_misc/.libs/libpam_misc.so.0.81.2 /lib/libpam_misc.so.0.81.2
	$(ROMFSINST) -s libpam_misc.so.0.81.2 /lib/libpam_misc.so.0
	$(ROMFSINST) -s libpam_misc.so.0.81.2 /lib/libpam_misc.so
	@find build/modules -name '*.so' | while read t; do \
		name=$$(expr $$t : '.*/\([^/]*\).so'); \
		upp=$$(echo $$name | tr '[a-z]' '[A-Z]'); \
		[ "$(CONFIG_LIB_LIBPAM_ALL_MODULES)" ] && upp=ALL_MODULES; \
		$(ROMFSINST) -v -d -e CONFIG_LIB_LIBPAM_$$upp $$t /lib/security/$$name.so;\
	done
	$(ROMFSINST) build/modules/pam_tally/.libs/pam_tally /bin/pam_tally
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_CONF conf/pam.conf /etc/$(DEFAULTDIR)/pam.conf
