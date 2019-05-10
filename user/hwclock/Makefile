
.EXPORT_ALL_VARIABLES:

all clean:
	$(MAKE) -C src $@

romfs:
	$(MAKE) -C src romfs ACL_LICENSE='$(shell $(ROOTDIR)/bin/license-detect.sh .)'

