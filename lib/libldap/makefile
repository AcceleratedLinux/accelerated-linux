VERSION = 2.4.48
URL = ftp://ftp.openldap.org/pub/OpenLDAP/openldap-release/openldap-$(VERSION).tgz

CONFOPTS += --without-yielding-select
CONFOPTS += --disable-slapd

ifdef CONFIG_LIB_LIBSSL
CONFOPTS += --with-tls
endif

ifdef CONFIG_LIB_CYRUSSASL
CONFOPTS += --with-cyrus-sasl
endif

ifdef CONFIG_LIB_LIBLDAP_UTILS
INSTALL_PROGRAMS = ldapsearch ldapadd ldapdelete ldapmodify ldappasswd ldapurl ldapcompare ldapexop ldapmodrdn ldapwhoami
else
INSTALL_PROGRAMS = ldapsearch
endif

CONFOPTS += ac_cv_func_memcmp_working=yes

MAKEVARS =

include $(ROOTDIR)/tools/automake.inc

