export ACL_LICENSE = BSD (3 clause)

URL = https://zlib.net/fossils/zlib-1.2.11.tar.gz
DOWNLOADHASH = c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1

zlib-1.2.11_NODEFCONF = 1
zlib-1.2.11_CONFOPTS += --prefix=

ifeq ($(CONFIG_MMU),)
zlib-1.2.11_CONFOPTS += --static
endif

include $(ROOTDIR)/tools/automake.inc
