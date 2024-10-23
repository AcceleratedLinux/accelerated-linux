
URL = https://nuttcp.net/nuttcp/nuttcp-6.1.2.tar.bz2
MAKEVARS = CC="$(CC)" CFLAGS="$(CFLAGS)"
FINALTARGET = built
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/nuttcp-6.1.2/nuttcp-6.1.2 /usr/bin/nuttcp

