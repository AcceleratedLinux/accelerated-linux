
CONFOPTS :=	--enable-ssl --enable-virus-scan		\
		--disable-ntp
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS +=	--enable-configfile=/etc/config/frox.conf
else
CONFOPTS +=	--enable-configfile=/etc/frox.conf
endif

LDFLAGS += -Wl,--allow-multiple-definition

all: Makefile
	$(MAKE) -f Makefile top_builddir=`pwd`

Makefile: makefile
	@chmod u+x configure
	sh configure $(CONFIGURE_OPTS) $(CONFOPTS)

makefile:
	@echo dummy target to avoid % catching us

clean:
	$(MAKE) -f Makefile distclean

romfs:
	$(ROMFSINST) src/frox /bin/frox
	$(ROMFSINST) -d src/frox.conf /etc/clamav/frox.conf

%: Makefile
	$(MAKE) -f Makefile $@
