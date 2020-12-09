ACL_LICENSE = Digi
ACL_URL     = http://www.digi.com/
ACL_PKG     = Digi Accelerated Linux
export ACL_PKG ACL_URL ACL_LICENSE

PKG         = libopengear
METHOD      = git
METHOD_OPTS = archive
SUBDIR      = lib/libopengear
VERSION     = master
URL         = git@bitbucket.org:opengear/opengear.git

PATCH_LEVEL = -p3 -b

BUILDDIR    = lib/libopengear

CFLAGS      += -Iinclude

include $(ROOTDIR)/tools/automake.inc
