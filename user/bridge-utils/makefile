CONFIGURE_OPTS += \
	--with-linux-headers=$(STAGEDIR)/include

all: Makefile
	$(MAKE) -f Makefile

Makefile:
	sh configure $(CONFIGURE_OPTS) $(CONFOPTS)

clean:
	$(MAKE) -f Makefile maintainer-clean

romfs:
	$(ROMFSINST) brctl/brctl /bin/brctl
