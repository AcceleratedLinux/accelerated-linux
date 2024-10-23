URL = https://github.com/jnavila/memtester/archive/refs/tags/v4.5.1.tar.gz

BUILDDIR = memtester-4.5.1
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/$(BUILDDIR)/memtester /bin/memtester
