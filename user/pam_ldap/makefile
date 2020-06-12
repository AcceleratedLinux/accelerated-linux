METHOD  = git
URL     = https://github.com/PADL/pam_ldap.git
VERSION = 6082484b2b85626f31e998557323503cd03de32f

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS  = --with-ldap-conf-file=/etc/config/ldap.conf
CONFOPTS += --with-ldap-secret-file=/etc/config/ldap.secret
else
CONFOPTS  = --with-ldap-conf-file=/etc/ldap.conf
CONFOPTS += --with-ldap-secret-file=/etc/ldap.secret
endif

BUILDVARS = touch build/pam_ldap/Makefile;

include $(ROOTDIR)/tools/automake.inc
