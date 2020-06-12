URL = http://downloads.sourceforge.net/project/cross-stuff/cross-stuff/1.0/popt-1.7.tar.gz

CONFOPTS += --without-libiconv-prefix --without-libintl-prefix

ifndef CONFIG_DEFAULTS_LIBC_MUSL
LDFLAGS += -lintl
endif

include $(ROOTDIR)/tools/automake.inc
