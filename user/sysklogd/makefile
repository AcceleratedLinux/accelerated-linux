VERSION=2.3.0
URL = https://github.com/troglobit/sysklogd/releases/download/v$(VERSION)/sysklogd-$(VERSION).tar.gz

ifdef CONFIG_USER_SYSKLOGD_LOG_PRIORITY
CFLAGS += -DLOG_PRIORITY
endif

CONFOPTS += --without-logger

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/sysklogd-$(VERSION)-install/sbin/syslogd /usr/sbin/syslogd
