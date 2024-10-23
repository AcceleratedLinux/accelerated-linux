URL = http://ftp.rpm.org/popt/releases/popt-1.x/popt-1.19.tar.gz

CONFOPTS += --without-libiconv-prefix --without-libintl-prefix

ifndef CONFIG_DEFAULTS_LIBC_MUSL
LDFLAGS += -lintl
endif

include $(ROOTDIR)/tools/automake.inc
