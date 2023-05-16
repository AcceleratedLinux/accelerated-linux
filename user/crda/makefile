ifdef CONFIG_USER_CRDA

CRDA_VERSION = 3.18
CRDA = crda-$(CRDA_VERSION)
$(CRDA)_URL = https://mirrors.edge.kernel.org/pub/software/network/crda/$(CRDA).tar.gz
$(CRDA)_LICENSE = copyleft-next-0.3.0

ifdef CONFIG_LIB_OPENSSL
export PKG_CONFIG_PATH := $(ROOTDIR)/lib/openssl/build:$(PKG_CONFIG_PATH)
$(CRDA)_MAKEVARS = USE_OPENSSL=1
endif
$(CRDA)_MAKEVARS += PREFIX=/ RUNTIME_PUBKEY_DIR=/lib/crda/pubkeys \
	UDEV_RULE_DIR=/libexec/udev/rules.d/
$(CRDA)_BUILD = all_noverify

AUTOMAKE_y += $(CRDA)

# Using OpenSSL-3.0 now, this will error out on warnings
CFLAGS += -Wno-deprecated-declarations
endif

###############################################################################
ifdef CONFIG_USER_CRDA_REGDB

REGDB_VERSION = 2018.09.07
REGDB = wireless-regdb-$(REGDB_VERSION)
$(REGDB)_URL = https://mirrors.edge.kernel.org/pub/software/network/wireless-regdb/$(REGDB).tar.gz
$(REGDB)_LICENSE = ISC

$(REGDB)_MAKEVARS = PREFIX=/
$(REGDB)_BUILDVARS = REGDB_AUTHOR=$(CONFIG_VENDOR)-$(CONFIG_PRODUCT)

$(REGDB)_PATCHES =
ifdef CONFIG_USER_CRDA_REGDB_UNRESTRICTED_WORLD
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-world-all-channels.patch
endif
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-parallel-make.patch
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-fix-overlap-frequency-CH-LI.patch

AUTOMAKE_y += $(REGDB)
endif

###############################################################################
include $(ROOTDIR)/tools/automake.inc
