URL = https://web.mit.edu/kerberos/dist/krb5/1.17/krb5-1.17.tar.gz

BUILDDIR = krb5-1.17/src
PATCH_LEVEL = -p2

CONFOPTS += --disable-thread-support
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
endif

CONFVARS += ac_cv_printf_positional=yes
CONFVARS += ac_cv_func_regcomp=yes
CONFVARS += krb5_cv_attr_constructor_destructor=yes,yes

include $(ROOTDIR)/tools/automake.inc
