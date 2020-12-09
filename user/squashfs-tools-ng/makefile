PKG = squashfs-tools-ng
VERSION = 1.0.2
URL = https://github.com/AgentD/squashfs-tools-ng/archive/v$(VERSION).tar.gz
DOWNLOADNAME = $(PKG)-$(VERSION).tar.gz
BUILDDIR = $(PKG)-$(VERSION)

AUTORECONF = ./autogen.sh

CONFOPTS = --with-xz
ifndef USER_SQUASHFS_TOOLS_NG_TOOLS
CONFOPTS += --without-tools
endif

# To be able to purge out unused functions when static linking with the library
CFLAGS += -ffunction-sections -fdata-sections

include $(ROOTDIR)/tools/automake.inc
