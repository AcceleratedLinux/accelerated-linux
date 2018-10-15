CONFOPTS += --without-libcap --prefix=

all: smcroute/Makefile
	$(MAKE) -C smcroute

smcroute/Makefile: smcroute/configure
	cd smcroute;./configure $(CONFIGURE_OPTS) $(CONFOPTS)

smcroute/configure: makefile
	cd smcroute;./autogen.sh

clean:
	$(MAKE) -C smcroute clean
	rm smcroute/configure
	rm smcroute/Makefile
	rm smcroute/config.status

romfs:
	$(ROMFSINST) smcroute/src/smcrouted /sbin/smcrouted
	$(ROMFSINST) smcroute/src/smcroutectl /sbin/smcroutectl

