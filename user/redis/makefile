VERSION = 4.0.14
URL = https://download.redis.io/releases/redis-$(VERSION).tar.gz

MAKEVARS = USE_JEMALLOC=no PREFIX=$(shell pwd)/build/redis-$(VERSION)-install/usr

ifeq ($(CONFIG_LIBCDIR),uClibc)
LDLIBS += -lubacktrace
endif

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -d build/redis-$(VERSION)/src/redis-server /usr/bin/redis-server
