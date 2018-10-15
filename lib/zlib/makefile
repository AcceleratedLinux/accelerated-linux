URL = http://www.zlib.net/zlib-1.2.11.tar.gz

zlib-1.2.11_NODEFCONF = 1
zlib-1.2.11_CONFOPTS += --prefix=

ifeq ($(CONFIG_MMU),)
zlib-1.2.11_CONFOPTS += --static
endif

include $(ROOTDIR)/tools/automake.inc
