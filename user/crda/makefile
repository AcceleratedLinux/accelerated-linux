ifndef ROOTDIR
 ROOTDIR = ../..
 -include $(ROOTDIR)/.config
 -include $(ROOTDIR)/config/.config
 LINUXDIR ?= $(ROOTDIR)/$(CONFIG_LINUXDIR)
 -include $(LINUXDIR)/.config
endif

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

ifndef CONFIG_CFG80211_CRDA_SUPPORT
 $(error Kernel support CFG80211_CRDA_SUPPORT is missing)
endif

# Using OpenSSL-3.0 now, this will error out on warnings
CFLAGS += -Wno-deprecated-declarations
endif

###############################################################################
ifdef CONFIG_USER_CRDA_REGDB

REGDB_VERSION = 2018.09.07
REGDB = wireless-regdb-$(REGDB_VERSION)
$(REGDB)_URL = https://mirrors.edge.kernel.org/pub/software/network/wireless-regdb/$(REGDB).tar.gz
$(REGDB)_LICENSE = ISC

REGDB_AUTHOR = dal
REGDB_PUBCERT = ../../prop/accns_certificates/ssl/certs/regdb.crt.pem
REGDB_PRIVKEY = ../../prop/accns_certificates/ssl/private/regdb.key.pem
REGDB_PUBKEY = $(REGDB_AUTHOR).key.pub.pem

$(REGDB)_MAKEVARS = PREFIX=/
$(REGDB)_BUILDVARS += REGDB_AUTHOR=$(REGDB_AUTHOR)
$(REGDB)_BUILDVARS += REGDB_PRIVKEY=$(abspath $(REGDB_PRIVKEY))
$(REGDB)_BUILDVARS += REGDB_PUBCERT=$(abspath $(REGDB_PUBCERT))
$(REGDB)_BUILDVARS += REGDB_PUBKEY=$(REGDB_PUBKEY)
$(REGDB)_BUILDVARS += REGDB_UPSTREAM_PUBKEY=$(REGDB_PUBKEY)

$(REGDB)_PATCHES =
ifdef CONFIG_USER_CRDA_REGDB_UNRESTRICTED_WORLD
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-world-all-channels.patch
endif
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-parallel-make.patch
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-fix-overlap-frequency-CH-LI.patch
$(REGDB)_PATCHES += wireless-regdb-2018.09.07-no-pem-regen.patch

AUTOMAKE_y += $(REGDB)
endif

###############################################################################

include $(ROOTDIR)/tools/automake.inc

ifdef CONFIG_USER_CRDA_REGDB

# Maintenance targets for (re)creating regdb signing certificates
# that is embedded in the kernel.
#    make REGDB_MAINTAINER=1 maint-all      -- regenerate certificates
#    make REGDB_MAINTAINER=1 maint-clean    -- delete generated certificates
# The generated files should be checked in to source after being regenerated:
#  ./<author>.key.pem
#  ./<author>.crt.pem
#  $(LINUXDIR)/net/wireless/certs/<author>.hex

 ifdef REGDB_MAINTAINER
  $(REGDB_PRIVKEY):
	openssl genrsa -out $@ 2048
  $(REGDB_PUBCERT): $(REGDB_PRIVKEY)
	openssl x509 -new -key $(REGDB_PRIVKEY) -out $@ \
		     -subj /CN=$(REGDB_AUTHOR) -days 36500
  maint-clean:
	rm -f $(REGDB_PRIVKEY) $(REGDB_PUBCERT)
	rm -f $(LINUXDIR)/net/wireless/certs/$(REGDB_AUTHOR).hex
  maint-all: $(REGDB_PUBCERT)
    ifneq ($(CONFIG_CFG80211_EXTRA_REGDB_KEYDIR:"%"=%),)
	openssl x509 -outform der <$(REGDB_PUBCERT) \
	   >$(LINUXDIR)/$(CONFIG_CFG80211_EXTRA_REGDB_KEYDIR:"%"=%)/$(REGDB_PUBCERT).x509
	@echo "Reminder: git add $(LINUXDIR)/$(CONFIG_CFG80211_EXTRA_REGDB_KEYDIR:"%"=%)/$(REGDB_PUBCERT).x509"
    else
	{ echo "/* $(REGDB_AUTHOR) regdb certificate */"; \
	  openssl x509 -outform der |\
	  xxd -i | \
	  sed -e 's/^ *//;$$s/$$/,/' | \
	  fmt -w 52; } \
	<$(REGDB_PUBCERT) >$(LINUXDIR)/net/wireless/certs/$(REGDB_AUTHOR).hex
	@echo "Reminder: git add $(LINUXDIR)/net/wireless/certs/$(REGDB_AUTHOR).hex"
    endif
	@echo "Reminder: git add $(REGDB_PUBCERT)"
	@echo "Reminder: git add $(REGDB_PRIVKEY)"

 endif # REGDB_MAINTAINER
endif # CONFIG_USER_CRDA_REGDB
