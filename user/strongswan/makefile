
VERSION = 5.9.7
URL = http://download.strongswan.org/strongswan-$(VERSION).tar.bz2
strongswan-$(VERSION)_ROMFS = strongswan_romfs

CONFOPTS := 
ifdef CONFIG_LIB_OPENSSL
CONFOPTS += --enable-openssl
endif
ifdef CONFIG_LIB_LIBUNBOUND
CONFOPTS += --enable-unbound
endif
ifdef CONFIG_LIB_LIBCURL
CONFOPTS += --enable-curl
endif
ifdef CONFIG_DEFAULTS_LIBC_UCLIBC
ifndef CONFIG_UCLIBC_HAS_GLIBC_CUSTOM_STREAMS
# sshkey plugin requires fmemopen()
CONFOPTS += --disable-sshkey
endif
endif

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFDIRBASE = etc/config
else
CONFDIRBASE = etc
endif

INSTALL_DIR = build/strongswan-$(VERSION)-install/

MAKEVARS = 

AUTOMAKE_PRESTAGE = build/strongswan-$(VERSION)-prestage

include $(ROOTDIR)/tools/automake.inc

build/strongswan-$(VERSION)-prestage: build/strongswan-$(VERSION)-installed
	rm -rf $(INSTALL_DIR)/$(CONFDIRBASE)/ipsec.conf
	rm -rf $(INSTALL_DIR)/$(CONFDIRBASE)/ipsec.d
	rm -rf $(INSTALL_DIR)/$(CONFDIRBASE)/ipsec.secrets
	rm -rf $(INSTALL_DIR)/$(CONFDIRBASE)/strongswan.d/charon.conf
	$(AT)touch $@

.PHONY: strongswan_romfs
strongswan_romfs:
ifndef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d -s /var/run/ipsec.conf /$(CONFDIRBASE)/ipsec.conf
	$(ROMFSINST) -d -s /var/run/ipsec.secrets /$(CONFDIRBASE)/ipsec.secrets
	$(ROMFSINST) -d -s /var/run/ipsec.d /$(CONFDIRBASE)/ipsec.d
	$(ROMFSINST) -d -s /var/run/charon.conf /$(CONFDIRBASE)/strongswan.d/charon.conf
endif
