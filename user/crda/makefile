ifdef CONFIG_USER_CRDA

CRDA_VERSION = 3.18
CRDA = crda-$(CRDA_VERSION)
$(CRDA)_URL = https://mirrors.edge.kernel.org/pub/software/network/crda/$(CRDA).tar.gz
$(CRDA)_LICENSE = copyleft-next-0.3.0

ifdef CONFIG_LIB_LIBSSL
export PKG_CONFIG_PATH := $(ROOTDIR)/lib/libssl/build:$(PKG_CONFIG_PATH)
$(CRDA)_MAKEVARS = USE_OPENSSL=1
endif
$(CRDA)_MAKEVARS += PREFIX=/ RUNTIME_PUBKEY_DIR=/lib/crda/pubkeys \
	UDEV_RULE_DIR=/libexec/udev/rules.d/
$(CRDA)_BUILD = all_noverify

AUTOMAKE_y += $(CRDA)
endif

###############################################################################
ifdef CONFIG_USER_CRDA_REGDB

REGDB_VERSION = 2018.09.07
REGDB = wireless-regdb-$(REGDB_VERSION)
$(REGDB)_URL = https://mirrors.edge.kernel.org/pub/software/network/wireless-regdb/$(REGDB).tar.gz
$(REGDB)_LICENSE = ISC

$(REGDB)_MAKEVARS = PREFIX=/

ifndef CONFIG_USER_CRDA_REGDB_UNRESTRICTED_WORLD
# Don't rebuild. The downloaded regulatory.bin has been signed by the
# wireless-regdb maintiner, Seth Forshee.
$(REGDB)_BUILD = nop
endif

AUTOMAKE_y += $(REGDB)
endif

###############################################################################
include $(ROOTDIR)/tools/automake.inc
