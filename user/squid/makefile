URL = https://www.squid-cache.org/Versions/v2/2.7/squid-2.7.STABLE9.tar.gz

CFLAGS := $(CFLAGS:-O1=-O2)
CXXFLAGS := $(CFLAGS:-O1=-O2)
#CXXFLAGS += -D_GLIBCXX_HAVE_MBSTATE_T=1

CONFVARS += ac_cv_epoll_works=yes
CONFVARS += ac_cv_af_unix_large_dgram=yes
CONFVARS += ac_cv_func_setresuid=yes
CONFVARS += ac_cv_func_va_copy=yes
CONFVARS += ac_cv_func___va_copy=yes

CONFOPTS += --prefix=
CONFOPTS += --datadir=/etc
CONFOPTS += --libexecdir=/bin
CONFOPTS += --enable-err-languages=English
CONFOPTS += --enable-default-err-language=English
CONFOPTS += --enable-linux-netfilter
CONFOPTS += --disable-ident-lookups
CONFOPTS += --enable-icap-support
CONFOPTS += --enable-dependency-tracking
CONFOPTS += --with-maxfd=8192
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
endif

#
# The following is what is required to build a newver v3 squid.
# It is currently disabled - we may want it in the furture - but for now
# we will continue to build the old v2 squid package.
# This v3 source does not build against STLport, it can only be made to
# link against the reall gcc libstdc++.
#
#URL = http://www.squid-cache.org/Versions/v3/3.5/squid-3.5.21.tar.gz
#
#CONFVARS = squid_cv_gnu_atomics=yes
#CONFVARS += ac_cv_lib_ldap_ldap_init=yes
#CONFVARS += ax_cv_cxx_compile_cxx11=no
#CONFOPTS += --disable-auth
#CONFOPTS += --disable-external-acl-helpers
#CONFOPTS += --with-openssl
#CONFOPTS += --disable-icap-client
#CONFOPTS += --disable-loadable-modules
#CONFOPTS += BUILDCXX="g++" BUILDCXXFLAGS="-O2"

include $(ROOTDIR)/tools/automake.inc

