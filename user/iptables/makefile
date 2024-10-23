URL = https://www.netfilter.org/projects/iptables/files/iptables-1.8.10.tar.xz
IPTABLES_INSTALL_DIR = build/iptables-$(IPTABLES_VERSION)-install
AUTORECONF = autoreconf -i
MAKEVARS =
UCFRONT_ENV = 

CONFOPTS += --disable-xml

ifdef CONFIG_USER_IPTABLES_SHARED
CONFOPTS += --enable-shared --disable-static
else
CONFOPTS += --disable-shared --enable-static
endif
ifndef CONFIG_USER_IPTABLES_IPTABLES
CONFOPTS += --disable-ipv4
endif
ifndef CONFIG_USER_IPTABLES_IP6TABLES
CONFOPTS += --disable-ipv6
endif
ifdef CONFIG_IP_NF_QUEUE
CONFOPTS += --enable-libipq
endif
ifndef CONFIG_USER_NFTABLES
CONFOPTS += --disable-nftables
endif

CONFOPTS += --enable-bpf-compiler

CFLAGS += -D_GNU_SOURCE

include $(ROOTDIR)/tools/automake.inc
