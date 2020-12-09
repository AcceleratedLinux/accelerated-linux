
URL = http://download.rsyslog.com/libfastjson/libfastjson-0.99.8.tar.gz
DOWNLOADHASH = 3544c757668b4a257825b3cbc26f800f59ef3c1ff2a260f40f96b48ab1d59e07

LIBTOOL = $(abspath $(shell pwd)/build/$(PKG)/libtool)

include $(ROOTDIR)/tools/automake.inc

$(PKG)_CONFOPTS =
#$(PKG)_CONFOPTS += ac_cv_func_malloc_0_nonnull=yes

$(PKG)_CONFVARS = LIBS="-lm"
