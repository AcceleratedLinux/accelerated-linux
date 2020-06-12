URL = http://doolittle.icarus.com/ntpclient/ntpclient_2015_365.tar.gz

BUILDDIR = ntpclient-2015
MAKEVARS = ntpclient adjtimex
ntpclient_2015_365_INSTALLVARS = :
ntpclient_2015_365_ROMFS = ntpclient_romfs

include $(ROOTDIR)/tools/automake.inc

ntpclient_romfs:
	$(ROMFSINST) -e CONFIG_USER_NTPCLIENT_NTPCLIENT build/$(BUILDDIR)/ntpclient /bin
	$(ROMFSINST) -e CONFIG_USER_NTPCLIENT_ADJTIMEX build/$(BUILDDIR)/adjtimex /bin

