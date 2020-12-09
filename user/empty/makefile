URL = https://sourceforge.net/projects/empty/files/empty/empty-0.6.20b/empty-0.6.20b.tgz
MAKEVARS = CC="$(CC)" PREFIX=""
FINALTARGET = built
ROMFS = my_romfs
include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/empty*/empty /bin/empty

