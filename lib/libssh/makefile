
CFLAGS   := $(CFLAGS:-fno-common=) -save-temps -D_GNU_SOURCE=1

# VERSION   = 0.7.6
# 0.8.4 requires cmake3
VERSION   = 0.8.4

URL       = https://git.libssh.org/projects/libssh.git/snapshot/libssh-$(VERSION).tar.gz
SRCDIR   := libssh-$(VERSION)
BUILDDIR := libssh-$(VERSION)-build
CMAKE_OPTS += -DOPENSSL_INCLUDE_DIR=$(STAGEDIR)
CMAKE_OPTS += -DWITH_SERVER=OFF
CMAKE_OPTS += -DWITH_GCRYPT=OFF
CMAKE_OPTS += -DWITH_GSSAPI=OFF
CMAKE_OPTS += -DWITH_NACL=OFF
CMAKE_OPTS += -DWITH_EXAMPLES=OFF
CMAKE_OPTS += -DWITH_INTERNAL_DOC=ON
CMAKE_OPTS += -DWITH_STACK_PROTECTOR=OFF

include $(ROOTDIR)/tools/cmake.inc
