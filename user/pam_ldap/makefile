VERSION = 185
URL = https://www.padl.com/download/pam_ldap-$(VERSION).tar.gz

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS  = --with-ldap-conf-file=/etc/config/ldap.conf
CONFOPTS += --with-ldap-secret-file=/etc/config/ldap.secret
else
CONFOPTS  = --with-ldap-conf-file=/etc/ldap.conf
CONFOPTS += --with-ldap-secret-file=/etc/ldap.secret
endif

# padl.com uses Let's Encrypt which many Linux distros don't trust the certs
# of, even though major web browsers accept LE certs.
WGETOPTS = --no-check-certificate

include $(ROOTDIR)/tools/automake.inc
