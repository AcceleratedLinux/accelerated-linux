URL = https://sourceforge.net/projects/ipsec-tools/files/ipsec-tools/0.8.2/ipsec-tools-0.8.2.tar.bz2

CONFOPTS = \
      --enable-hybrid \
      --without-libpam \
      --disable-gssapi \
      --with-kernel-headers=$(STAGEDIR)/include

CFLAGS += -Wno-error

ifeq ($(CONFIG_USER_IPSEC_TOOLS_ADMINPORT),y)
CONFOPTS += --enable-adminport
else
CONFOPTS += --disable-adminport
endif

ifeq ($(CONFIG_USER_IPSEC_TOOLS_NATT),y)
CONFOPTS += --enable-natt
else
CONFOPTS += --disable-natt
endif

ifeq ($(CONFIG_USER_IPSEC_TOOLS_FRAG),y)
CONFOPTS += --enable-frag
else
CONFOPTS += --disable-frag
endif

ifeq ($(CONFIG_USER_IPSEC_TOOLS_DPD),y)
CONFOPTS += --enable-dpd
else
CONFOPTS += --disable-dpd
endif

ifeq ($(CONFIG_USER_IPSEC_TOOLS_STATS),y)
CONFOPTS += --enable-stats
else
CONFOPTS += --disable-stats
endif

ifeq ($(BR2_INET_IPV6),y)
CONFOPTS += --enable-ipv6
else
CONFOPTS += --disable-ipv6
endif

ifneq ($(CONFIG_USER_IPSEC_TOOLS_READLINE),y)
CONFOPTS += --without-readline
endif

ifeq ($(CONFIG_USER_IPSEC_SECCTX_DISABLE),y)
CONFOPTS += --enable-security-context=no
endif
ifeq ($(CONFIG_USER_IPSEC_SECCTX_ENABLE),y)
CONFOPTS += --enable-security-context=yes
endif
ifeq ($(CONFIG_USER_IPSEC_SECCTX_KERNEL),y)
CONFOPTS += --enable-security-context=kernel
endif

MAKEVARS = 


include $(ROOTDIR)/tools/automake.inc

