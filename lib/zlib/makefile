export ACL_LICENSE = BSD (3 clause)

VERSION = 1.3.1
URL = https://zlib.net/zlib-$(VERSION).tar.xz
DOWNLOADHASH = 38ef96b8dfe510d42707d9c781877914792541133e1870841463bfa73f883e32

zlib-$(VERSION)_NODEFCONF = 1
zlib-$(VERSION)_CONFOPTS += --prefix=

ifeq ($(CONFIG_MMU),)
zlib-$(VERSION)_CONFOPTS += --static
endif

include $(ROOTDIR)/tools/automake.inc
