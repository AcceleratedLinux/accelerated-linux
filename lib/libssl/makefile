#
# this makefile gets recursed through by various bits of the build
# so we need to only setup some things when invoked from outside
# this directory.
#
# david.mccullough@accelerated.com
#

IN_LIBSSL := true
export IN_LIBSSL

#
# we need to do different things for 64 bit systems
#
ifdef CONFIG_64BIT
MACHINE=uClinux-dist64
else
MACHINE=uClinux-dist
endif

CONFIG_OPTS :=

ifneq ($(filter-out linux-2.0% linux-2.2% linux-2.4%, $(LINUXDIR)),)
ifdef CONFIG_ARM
export UCDIST_ASM=armv4_asm
endif
endif
ifdef CONFIG_X86_64
export UCDIST_ASM=x86_64_asm
export UCDIST_TYP=elf
endif
ifdef CONFIG_MIPS
ifdef CONFIG_64BIT
export UCDIST_ASM=mips64_asm
export UCDIST_TYP=64
CONFIG_OPTS += no-asm
endif
endif
ifdef CONFIG_ARM64
export UCDIST_ASM=aarch64_asm
export UCDIST_TYP=linux64
endif
ifndef UCDIST_ASM
export UCDIST_ASM=no_asm
CONFIG_OPTS += no-asm
endif

CONFIG_OPTS += --prefix=// --install_prefix=$(shell pwd)/build/install

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFIG_OPTS += --openssldir=/etc/config
else
CONFIG_OPTS += --openssldir=/etc
endif
ifdef DISABLE_SHARED_SSL
CONFIG_OPTS += no-shared
else
CONFIG_OPTS += shared
endif

CONFIG_OPTS += no-rc2
CONFIG_OPTS += no-krb5
CONFIG_OPTS += no-rc5
CONFIG_OPTS += no-md2
CONFIG_OPTS += no-mdc2
CONFIG_OPTS += no-idea
#CONFIG_OPTS += no-pem
#CONFIG_OPTS += no-md5
#CONFIG_OPTS += no-sha
#CONFIG_OPTS += no-hmac
#CONFIG_OPTS += no-des
#CONFIG_OPTS += no-aes
#CONFIG_OPTS += no-bn
#CONFIG_OPTS += no-ecdh
#CONFIG_OPTS += no-ecdsa
#CONFIG_OPTS += no-ec
#CONFIG_OPTS += no-rsa
#CONFIG_OPTS += no-dsa
#CONFIG_OPTS += no-dh
CONFIG_OPTS += no-dso
#CONFIG_OPTS += no-engine
#CONFIG_OPTS += no-buffer
#CONFIG_OPTS += no-bio
#CONFIG_OPTS += no-stack
#CONFIG_OPTS += no-lhash
#CONFIG_OPTS += no-rand
CONFIG_OPTS += no-err
#CONFIG_OPTS += no-evp
#CONFIG_OPTS += no-asn1
#CONFIG_OPTS += no-x509
#CONFIG_OPTS += no-x509v3
#CONFIG_OPTS += no-txt_db
#CONFIG_OPTS += no-pkcs7
#CONFIG_OPTS += no-pkcs12
#CONFIG_OPTS += no-comp
#CONFIG_OPTS += no-ocsp
#CONFIG_OPTS += no-ui
#CONFIG_OPTS += no-store
#CONFIG_OPTS += no-pqueue
#CONFIG_OPTS += no-tlsext
#CONFIG_OPTS += no-ssl2
#CONFIG_OPTS += no-ssl3

# REVISIT: It would be better to have OPENSSL config options
# which turn on this support as needed
ifeq ($(CONFIG_USER_NESSUS_NASL)$(CONFIG_USER_SSH_SSH)$(CONFIG_USER_SSH_SSHD),)
CONFIG_OPTS += no-ripemd
CONFIG_OPTS += no-cast
CONFIG_OPTS += no-rc4
endif

ifeq ($(CONFIG_USER_NESSUS_NASL)$(CONFIG_USER_SSH_SSH)$(CONFIG_PROP_SSCEP_SSCEP)$(CONFIG_USER_SSH_SSHD),)
CONFIG_OPTS += no-bf
endif

ifeq ($(CONFIG_USER_OPENVPN_OPENVPN)$(CONFIG_USER_WGET)$(CONFIG_USER_WPA_SUPPLICANT),)
CONFIG_OPTS += no-md4
endif

ifdef CONFIG_OCF_OCF
CONFIG_OPTS += -DHAVE_CRYPTODEV
#CONFIG_OPTS += -DUSE_CRYPTODEV_DIGESTS
endif

ifdef CONFIG_MODULES_CRYPTODEV_CRYPTODEV
CONFIG_OPTS += -DHAVE_CRYPTODEV
CONFIG_OPTS += -DUSE_CRYPTODEV_DIGESTS
YOU_WANT_DYNAMIC_HW_ENGINES_ENABLED = y
endif

# this fixes the engines install (openssl-1 and later),  even though we
# explicitly state no-dso meaning we don't want them
CONFIG_OPTS += -DMY_DSO_DLFCN_HACK

#
# if you want engines (they are dl loaded),  a few things
# need to be setup,  you will also need to mod everything
# to link against -ldl if it uses libcrypto.  By default we
# disable it (cryptodev suport is still included).
#
ifdef YOU_WANT_DYNAMIC_HW_ENGINES_ENABLED
LIBSSL_dlfcn = dlfcn
LDLIBS += -ldl
else
CONFIG_OPTS += no-hw
LIBSSL_dlfcn =
endif
export LIBSSL_dlfcn

#
# our libs aren't in the default location yet
#
LDFLAGS += -L$(ROOTDIR)/lib/libssl/build
export LDFLAGS

all: $(FIPS_TARGET) build/configured
	$(MAKE) -j1 -C build
	$(MAKE) -j1 -C build install_sw

############################################################################
############################################################################
#
# FIPS support
#
############################################################################
############################################################################
ifdef CONFIG_LIB_LIBSSL_FIPS
############################################################################
# FIPs settings
# -------------
# There are two options to customise the FIP's build here.  Firstly the
# source verison to use and second,  the target to build (compliant or not)
#
# Comment in the required build settings.

#FIPS_SRC_DIR = openssl-fips-2.0.12
FIPS_SRC_DIR = openssl-fips-2.0.10

#FIPS_TARGET = build-fips-algvs
FIPS_TARGET  = build-fips

# End of FIPs settings
############################################################################

# A list of possible fips source distributions to use.  These checksums
# were obtained directly from the OpenSSL FIPs CD distribution provided

openssl-fips-2.0.10-sha256sum = a42ccf5f08a8b510c0c78da1ba889532a0ce24e772b576604faf09b4d6a0f771
openssl-fips-2.0.10-url       = https://www.openssl.org/source/openssl-fips-2.0.10.tar.gz

openssl-fips-2.0.12-sha256sum = ab5af8f4c542ce3fae31fbd99b029fdc37da5e655191f3c4bfdaa8b0d6dc10b7
openssl-fips-2.0.12-url       = https://www.openssl.org/source/openssl-fips-2.0.12.tar.gz

FIPS_PATH    = $(shell pwd)
FIPS_BIN     = $(FIPS_PATH)/fips-bin
FIPS_INSTALL = $(FIPS_PATH)/fips-install
FIPS_SIG     = $(FIPS_PATH)/$(FIPS_SRC_DIR)/util/incore

FIPS_PATCH = $(wildcard patches/$(FIPS_SRC_DIR).patch)

FIPS_ENV    = export CC=gcc CROSS_COMPILE=cross-; \
	      export PATH="$(PATH):$(FIPS_BIN)" ; \
	      export DESTDIR=$(FIPS_INSTALL); \
	      export FIPSDIR="/"; \
	      export INSTALL_PREFIX=$(FIPS_INSTALL)

FIPS_ROMFS   = romfs-fips
FIPS_CLEAN   = clean-fips

export FIPS_SIG

# some extra openssl build opts to use our fips module
CONFIG_OPTS += fips
CONFIG_OPTS += --with-fipslibdir=$(FIPS_INSTALL)/lib/

CFLAGS  += -I$(FIPS_INSTALL)/include
LDFLAGS += -L$(FIPS_INSTALL)/lib

fips-tools: makefile
	rm -rf $(FIPS_BIN)
	mkdir $(FIPS_BIN)
	echo -e '#!/bin/sh\nexec $(CC) "$$@"\n' > $(FIPS_BIN)/cross-gcc
	echo -e '#!/bin/sh\nexec $(LD) "$$@"\n' > $(FIPS_BIN)/cross-ld
	echo -e '#!/bin/sh\nexec $(AR) "$$@"\n' > $(FIPS_BIN)/cross-ar
	echo -e '#!/bin/sh\nexec $(RANLIB) "$$@"\n' > $(FIPS_BIN)/cross-ranlib
	echo -e '#!/bin/sh\nexec $(CROSS_COMPILE)objdump "$$@"\n' > $(FIPS_BIN)/cross-objdump
	echo -e '#!/bin/sh\nexec $(CROSS_COMPILE)objcopy "$$@"\n' > $(FIPS_BIN)/cross-objcopy
	chmod +x $(FIPS_BIN)/*
	touch $@

#
# the fully compliant build, no patching, no weird stuff
#
build-fips: $(FIPS_SRC_DIR).tar.gz fips-tools
	rm -rf $(FIPS_SRC_DIR)
	$(FIPS_ENV); \
		\
		gunzip -c $(FIPS_SRC_DIR).tar.gz | tar xf -; \
		cd $(FIPS_SRC_DIR);  \
		./config no-asm; \
		make; \
		make install; \
		exit $$?
	touch $@

#
# a fips build with algvs tests and any patches created, this does not
# follow the policy and is here for testing and development only
#
build-fips-algvs: $(FIPS_SRC_DIR).tar.gz fips-tools
	rm -rf $(FIPS_SRC_DIR)
	$(FIPS_ENV); \
		\
		gunzip -c $(FIPS_SRC_DIR).tar.gz | tar xf -; \
		[ "$(FIPS_PATCH)" ] && patch -p0 < $(FIPS_PATCH); \
		cd $(FIPS_SRC_DIR);  \
		./config no-asm; \
		make; \
		make build_algvs || exit 1; \
		make install || exit 1
	touch $@

romfs-fips:
	[ ! -f $(FIPS_SRC_DIR)/test/fips_algvs ] || $(ROMFSINST) $(FIPS_SRC_DIR)/test/fips_algvs /bin/fips_algvs

clean-fips:
	rm -rf $(FIPS_SRC_DIR) $(FIPS_BIN) $(FIPS_INSTALL) build-fips fips-tools

############################################################################
endif # CONFIG_LIB_LIBSSL_FIPS
############################################################################
############################################################################
#
# support for fetching files
#

WGET ?= wget

#
# You can override the downloads cache dir in case you would prefer
# have all the files in tree or elsewhere.  The default is ~/.downloads
#
ifndef UC_DOWNLOADS
UC_DOWNLOADS = $(HOME)/.downloads
endif

%.tar.bz2 %.tar.gz:
	@echo "Downloading $* ..."
	@[ -d "$(UC_DOWNLOADS)" ] || mkdir "$(UC_DOWNLOADS)"
	@if [ ! -f "$(UC_DOWNLOADS)/$@" ]; then \
		(cd "$(UC_DOWNLOADS)"; $(WGET) "$($*-url)"); \
	fi
	@if [ -f "$(UC_DOWNLOADS)/$@" ]; then \
			ln -fs "$(UC_DOWNLOADS)/$@" $@; \
	fi
	@if [ ! -f "$@" ]; then \
		echo "Cannot find download for $@" >&2 ; \
		exit 1; \
	fi
	@if [ "$($*-sha256sum)" -a \
		"$($*-sha256sum)" != "$$(sha256sum $@ | cut -d' ' -f1)" ]; \
	 then \
		echo "$@ has invalid sha256sum" >&2 ; \
		exit 1; \
	 else \
		echo "Using $@ with sha256sum of $($*-sha256sum)" ; \
	 fi

############################################################################
#
# we may not have the library here, so setup for downloading if needed

ifeq ($(wildcard Configure),)

openssl-0.9.8i-url := http://www.openssl.org/source/openssl-0.9.8i.tar.gz
openssl-0.9.8k-url := http://www.openssl.org/source/openssl-0.9.8k.tar.gz
openssl-0.9.8n-url := http://www.openssl.org/source/openssl-0.9.8n.tar.gz
openssl-0.9.8r-url := http://www.openssl.org/source/openssl-0.9.8r.tar.gz
openssl-1.0.1c-url := http://www.openssl.org/source/openssl-1.0.1c.tar.gz
openssl-1.0.1e-url := http://www.openssl.org/source/openssl-1.0.1e.tar.gz
openssl-1.0.1g-url := http://www.openssl.org/source/openssl-1.0.1g.tar.gz
openssl-1.0.1h-url := http://www.openssl.org/source/openssl-1.0.1h.tar.gz
openssl-1.0.1i-url := http://www.openssl.org/source/openssl-1.0.1i.tar.gz
openssl-1.0.1j-url := http://www.openssl.org/source/openssl-1.0.1j.tar.gz
openssl-1.0.2-url  := http://www.openssl.org/source/openssl-1.0.2.tar.gz
openssl-1.0.2c-url := http://www.openssl.org/source/openssl-1.0.2c.tar.gz
openssl-1.0.2d-url := http://www.openssl.org/source/openssl-1.0.2d.tar.gz
openssl-1.0.2e-url := http://www.openssl.org/source/openssl-1.0.2e.tar.gz
openssl-1.0.2g-url := http://www.openssl.org/source/openssl-1.0.2g.tar.gz
openssl-1.0.2h-url := http://www.openssl.org/source/openssl-1.0.2h.tar.gz
openssl-1.0.2j-url := http://www.openssl.org/source/openssl-1.0.2j.tar.gz
openssl-1.0.2o-url := http://www.openssl.org/source/openssl-1.0.2o.tar.gz
openssl-1.0.2p-url := http://www.openssl.org/source/openssl-1.0.2p.tar.gz

SRC_DIR := openssl-1.0.2p
PATCH = $(wildcard patches/$(SRC_DIR).patch)
ifdef CONFIG_MODULES_CRYPTODEV_CRYPTODEV
PATCH_CRYPTODEV_MODULE = $(wildcard patches/$(SRC_DIR)-cryptodev-module.patch)
endif
EXTRA_CLEAN := $(SRC_DIR)

ifneq ($(findstring openssl-0,$(SRC_DIR)),)
CONFIG_OPTS += --with-cryptodev
#CONFIG_OPTS += --with-cryptodev-digests
# CONFIG_OPTS := $(subst -DUSE_CRYPTODEV_DIGESTS,--with-cryptodev-digests,$(CONFIG_OPTS))
CONFIG_OPTS := $(subst -DHAVE_CRYPTODEV,--with-cryptodev,$(CONFIG_OPTS))
endif

$(SRC_DIR)/Configure $(SRC_DIR)/config: makefile $(SRC_DIR).tar.gz $(PATCH)
	rm -rf $(SRC_DIR) build
	gunzip < $(SRC_DIR).tar.gz | tar xf -
ifneq ($(PATCH),)
	patch -p0 < patches/$(SRC_DIR).patch
endif
ifdef CONFIG_MODULES_CRYPTODEV_CRYPTODEV
ifneq ($(PATCH_CRYPTODEV_MODULE),)
	patch -p0 < patches/$(SRC_DIR)-cryptodev-module.patch
endif
endif
	touch $(SRC_DIR)/Configure $(SRC_DIR)/config
	-ln -sf $(SRC_DIR)/make-uclinuxdist $(SRC_DIR)/makefile

else

SRC_DIR := .

endif

############################################################################

build/configured: makefile $(SRC_DIR)/config $(SRC_DIR)/Configure $(FIPS_TARGET)
	rm -rf build
	(cd $(SRC_DIR)/.; find . -type d) > .dirs
	(cd $(SRC_DIR)/.; find . ! -type d) | grep -v ./makefile > .files
	while read t; do mkdir -p build/$$t; done < .dirs
	while read t; do ln -s `pwd`/$(SRC_DIR)/$$t build/$$t; done < .files
	rm -f .dirs .files
	chmod +x build/config
	cd build; FIPSDIR=$(FIPS_INSTALL) CROSS_COMPILE= MACHINE=$(MACHINE) ./config $(CONFIG_OPTS)
	$(MAKE) -C build depend
	$(MAKE) -C build links
	touch build/configured

clean: $(FIPS_CLEAN)
	-rm -rf build $(EXTRA_CLEAN)

romfs: $(FIPS_ROMFS)
	cd build/install/lib; \
	for i in *.so*; do \
		if [ -L $$i ]; then \
			$(ROMFSINST) -s `find $$i -printf %l` /lib/$$i; \
		elif [ -f $$i ]; then \
			$(ROMFSINST) /lib/$$i; \
		fi; \
	done
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS build/install/bin/openssl /bin/openssl
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS build/install/etc/config/openssl.cnf /etc/default/openssl.cnf
else
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS build/install/etc/openssl.cnf /etc/openssl.cnf
endif
	# $(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS build/install/bin/c_rehash /bin/c_rehash


