
URL = ftp://ftp.ch.debian.org/mirror/debian-archive/debian/pool/main/s/sysklogd/sysklogd_1.5.orig.tar.gz
BUILDDIR = sysklogd-1.5

ifdef CONFIG_DEFAULTS_LIBC_MUSL
CFLAGS += -D__GLIBC__=2 -D__GLIBC_MINOR__=1
endif

ifdef CONFIG_USER_SYSKLOGD_LOG_PRIORITY
CFLAGS += -DLOG_PRIORITY
endif

FINALTARGET = built
MAKEVARS = CC="$(CC)"
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/$(sysklogd_1.5.orig_BUILDDIR)/klogd /usr/sbin/klogd
	$(ROMFSINST) build/$(sysklogd_1.5.orig_BUILDDIR)/syslogd /usr/sbin/syslogd
