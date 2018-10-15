VERSION = 0.99.24
URL = http://download.savannah.gnu.org/releases/quagga/quagga-$(VERSION).tar.gz

CONFOPTS =
CONFOPTS += --prefix=
CONFOPTS += --sbindir=/bin
CONFOPTS += --program-transform-name=
CONFOPTS += --disable-user --disable-group
CONFOPTS += --disable-doc
CONFOPTS += --enable-exampledir=/samples

ifdef CONFIG_USER_QUAGGA_VTYSH_VTYSH
CONFOPTS += --enable-vtysh
endif
ifndef CONFIG_USER_QUAGGA_ZEBRA_ZEBRA
CONFOPTS += --disable-zebra
endif
ifndef CONFIG_USER_QUAGGA_BGPD_BGPD
CONFOPTS += --disable-bgpd
endif
ifndef CONFIG_USER_QUAGGA_RIPD_RIPD
CONFOPTS += --disable-ripd
endif
ifndef CONFIG_USER_QUAGGA_RIPNGD_RIPNGD
CONFOPTS += --disable-ripngd
endif
ifndef CONFIG_USER_QUAGGA_OSPFD_OSPFD
CONFOPTS += --disable-ospfd
endif
ifndef CONFIG_USER_QUAGGA_OSPF6D_OSPF6D
CONFOPTS += --disable-ospf6d
endif
ifndef CONFIG_USER_QUAGGA_OSPFAPI
CONFOPTS += --disable-ospfapi
endif
ifndef CONFIG_USER_QUAGGA_OSPFAPI_OSPFCLIENT
CONFOPTS += --disable-ospfapiclient
endif
ifndef CONFIG_USER_QUAGGA_WATCHQUAGGA_WATCHQUAGGA
CONFOPTS += --disable-watchquagga
endif
ifndef CONFIG_USER_QUAGGA_ISISD_ISISD
CONFOPTS += --disable-isisd
endif
ifndef CONFIG_USER_QUAGGA_BABELD_BABELD
CONFOPTS += --disable-babeld
endif
ifdef CONFIG_USER_QUAGGA_PIMD_PIMD
CONFOPTS += --enable-pimd
endif

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
else
CONFOPTS += --sysconfdir=/etc
endif

CURSES = -lncurses

include $(ROOTDIR)/tools/automake.inc
