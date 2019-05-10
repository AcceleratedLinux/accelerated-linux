all:
	$(MAKE) -f Makefile all

clean:
	$(MAKE) -f Makefile clean

romfs:
	$(MAKE) _romfs ACL_LICENSE='$(shell $(ROOTDIR)/bin/license-detect.sh `pwd`)'

_romfs:
	$(ROMFSINST) /bin/ebtables
	$(ROMFSINST) /bin/ebtables-restore
	$(ROMFSINST) /lib/libebtc.so
	cd extensions; \
	for i in *.so ; do \
		$(ROMFSINST) /lib/$$i ; \
	done
