
VERSION = 5.7.1
URL = http://download.strongswan.org/strongswan-$(VERSION).tar.bz2
strongswan-$(VERSION)_ROMFS = strongswan_romfs

CONFOPTS :=
ifdef CONFIG_LIB_LIBSSL
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

MAKEVARS = 

include $(ROOTDIR)/tools/automake.inc

.PHONY: strongswan_romfs
strongswan_romfs:
	$(ROMFSINST) -R /etc/ipsec.conf
	$(ROMFSINST) -R /etc/ipsec.d
	$(ROMFSINST) -R /etc/ipsec.secrets
	$(ROMFSINST) -s /var/run/ipsec.conf /etc/ipsec.conf
	$(ROMFSINST) -s /var/run/ipsec.secrets /etc/ipsec.secrets
	$(ROMFSINST) -s /var/run/ipsec.d /etc/ipsec.d
	sed -i 's/^\(charon.*\)$$/\1\n\tinstall_routes = no\n\ti_dont_care_about_security_and_use_aggressive_mode_psk = yes\n/' $(ROMFSDIR)/etc/strongswan.conf
	sed -i 's/^.*#.*\(keep_alive\).*$$/keep_alive = 40s\n/' $(ROMFSDIR)/etc/strongswan.d/charon.conf
	sed -ri "s/(# )?make_before_break = no/make_before_break = yes/" $(ROMFSDIR)/etc/strongswan.d/charon.conf
	mv $(ROMFSDIR)/etc/strongswan.d/charon.conf $(ROMFSDIR)/etc/strongswan.d/charon.conf.template
	$(ROMFSINST) -s /var/run/charon.conf /etc/strongswan.d/charon.conf
	sed -i 's?^\(charon.*\)$$?include /var/run/charon-logging.conf\n\1?' \
		$(ROMFSDIR)/etc/strongswan.d/charon-logging.conf

