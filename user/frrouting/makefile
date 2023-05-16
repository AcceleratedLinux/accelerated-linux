VERSION = ddab69d59c11bb6480cb33fea5a27bc87b1157bb # stable/7.5, 22 Apr 2021
#VERSION = 79188bf710e92acf42fb5b9b0a2e9593a5ee9b05 # release 8.2.2 July 2022
URL = https://github.com/FRRouting/frr.git
METHOD = git

SRCDIR = frr
BUILDDIR = frr-build
CONFIGURE = ../$(SRCDIR)/configure
AUTORECONF = ./bootstrap.sh

CONFOPTS =
CONFOPTS += --prefix=
CONFOPTS += --sbindir=/bin
CONFOPTS += --program-transform-name=
CONFOPTS += --disable-user --disable-group
CONFOPTS += --disable-doc
CONFOPTS += --enable-exampledir=/samples

# Additional features that are enabled by default but not required.
CONFOPTS += --disable-babeld
CONFOPTS += --disable-bfdd
CONFOPTS += --disable-eigrpd
CONFOPTS += --disable-fabricd
CONFOPTS += --disable-ldpd
CONFOPTS += --disable-ospfapi
CONFOPTS += --disable-pbrd
CONFOPTS += --disable-pimd
CONFOPTS += --disable-vrrpd
CONFOPTS += --disable-watchfrr

ifndef CONFIG_USER_FRROUTING_NHRPD
CONFOPTS += --disable-nhrpd
endif
ifndef CONFIG_USER_FRROUTING_ZEBRA
CONFOPTS += --disable-zebra
endif
ifndef CONFIG_USER_FRROUTING_BGPD
CONFOPTS += --disable-bgpd
endif
ifndef CONFIG_USER_FRROUTING_ISISD
CONFOPTS += --disable-isisd
endif
ifndef CONFIG_USER_FRROUTING_OSPFD
CONFOPTS += --disable-ospfd
endif
ifndef CONFIG_USER_FRROUTING_OSPF6D
CONFOPTS += --disable-ospf6d
endif
ifndef CONFIG_USER_FRROUTING_RIPD
CONFOPTS += --disable-ripd
endif
ifndef CONFIG_USER_FRROUTING_RIPNGD
CONFOPTS += --disable-ripngd
endif

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
else
CONFOPTS += --sysconfdir=/etc
endif

CURSES = -lncurses

MAKEVARS =

include $(ROOTDIR)/tools/automake.inc

# Use the same python version as the one built for the target.
PYTHON_VERSION := $(shell sed -n 's/^PYTHON_VERSION[[:blank:]]*=[[:blank:]]*\(.*\)$$/\1/g;T;p' $(ROOTDIR)/user/python/makefile)
PYTHON_VERSION_SHORT := $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

PYTHON_HOSTINSTALL := $(ROOTDIR)/user/python/build/Python-Hostinstall
PYTHON := $(PYTHON_HOSTINSTALL)/bin/python$(PYTHON_VERSION_SHORT)

# Make sure we do not lean on system packages.
export PYTHONNOUSERSITE = 1
