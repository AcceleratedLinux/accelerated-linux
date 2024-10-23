VERSION = 31ed3dd753d62b5d8916998bc32814007e91364b
URL = https://github.com/FRRouting/frr.git
METHOD = git

SRCDIR = frr
BUILDDIR = frr-build
CONFIGURE = ../$(SRCDIR)/configure
AUTORECONF = ./bootstrap.sh
PROTOC=$(ROOTDIR)/lib/protobuf/build/protobuf-Hostinstall/bin/protoc
PROTOC_C=$(ROOTDIR)/lib/protobuf-c/build/protobuf-c-Hostinstall/bin/protoc-c

CONFOPTS =
CONFOPTS += --prefix=
CONFOPTS += --sbindir=/bin
CONFOPTS += --program-transform-name=
CONFOPTS += --disable-user --disable-group
CONFOPTS += --disable-doc

# Additional features that are enabled by default but not required.
CONFOPTS += --disable-babeld
CONFOPTS += --disable-bfdd
CONFOPTS += --disable-eigrpd
CONFOPTS += --disable-fabricd
CONFOPTS += --disable-ldpd
CONFOPTS += --disable-ospfapi
CONFOPTS += --disable-pbrd
CONFOPTS += --disable-pimd
CONFOPTS += --disable-pim6d
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

# this takes a while; parallelize it.
MAKEVARS = -j$(HOST_NCPU)

include $(ROOTDIR)/tools/automake.inc
CFLAGS += -g0
# Use the same python version as the one built for the target.
PYTHON_VERSION := $(shell sed -n 's/^PYTHON_VERSION[[:blank:]]*=[[:blank:]]*\(.*\)$$/\1/g;T;p' $(ROOTDIR)/user/python/makefile)
PYTHON_VERSION_SHORT := $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

PYTHON_HOSTINSTALL := $(ROOTDIR)/user/python/build/Python-Hostinstall
PYTHON := $(PYTHON_HOSTINSTALL)/bin/python$(PYTHON_VERSION_SHORT)

# Make sure we do not lean on system packages.
export PYTHONNOUSERSITE = 1
