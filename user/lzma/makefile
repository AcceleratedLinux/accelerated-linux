
CONFIGURE = :
BUILDDIR = C/Util/Lzma
MAKEVARS = -f makefile.gcc CXX="$(CC) $(CFLAGS) $(LDLIBS)"
INSTALL = all
AUTOMAKE_ROMFS = my_romfs
URL = https://sourceforge.net/projects/sevenzip/files/LZMA%20SDK/lzma922.tar.bz2
include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/$(BUILDDIR)/lzma /bin/lzma


