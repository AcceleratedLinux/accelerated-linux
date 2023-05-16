VERSION = 1.0.8
URL = https://sourceware.org/pub/bzip2/bzip2-$(VERSION).tar.gz

CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -fpic -fPIC

MAKEVARS += CC="$(CC)"
MAKEVARS += AR="$(AR)"
MAKEVARS += RANLIB="$(RANLIB)"
MAKEVARS += CFLAGS="$(CFLAGS)"
MAKEVARS += LDFLAGS="$(LDFLAGS)"
MAKEVARS += PREFIX=`pwd`/build/$*-install

AUTOMAKE_ROMFS = bzip2_romfs

include $(ROOTDIR)/tools/automake.inc

bzip2_romfs:
	cd build/bzip2-$(VERSION)-install/lib ; \
	$(ROMFSINST) -d /lib/libbz2.so.$(VERSION) ;
	$(ROMFSINST) -s libbz2.so.$(VERSION) /lib/libbz2.so.1.0
	$(ROMFSINST) -s libbz2.so.$(VERSION) /lib/libbz2.so
	cd build/bzip2-$(VERSION)-install/bin ; \
	$(ROMFSINST) -d -e CONFIG_USER_BZIP2_BZIP2 /usr/bin/bzip2
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzip2 /usr/bin/bunzip2
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzip2 /usr/bin/bzcat
	cd build/bzip2-$(VERSION)-install/bin ; \
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 /usr/bin/bzgrep
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzgrep /usr/bin/bzegrep
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzgrep /usr/bin/bzfgrep
	cd build/bzip2-$(VERSION)-install/bin ; \
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 /usr/bin/bzdiff
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzdiff /usr/bin/bzcmp
	cd build/bzip2-$(VERSION)-install/bin ; \
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 /usr/bin/bzmore
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 -s bzmore /usr/bin/bzless
	cd build/bzip2-$(VERSION)-install/bin ; \
	$(ROMFSINST) -e CONFIG_USER_BZIP2_BZIP2 /usr/bin/bzip2recover
