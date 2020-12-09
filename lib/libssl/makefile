###############################################################################
# OpenSSL
###############################################################################

ifndef CONFIG_LIB_LIBSSL_FIPS

OPENSSL_PKG = openssl-1.1.1h

else

# OpenSSL 1.0.2 required for FIPS (not yet supported on 1.1.1)
OPENSSL_PKG = openssl-1.0.2u

# Checksums obtained directly from the OpenSSL FIPs CD distribution
FIPS_PKG = openssl-fips-2.0.10
openssl-fips-2.0.10_DOWNLOADHASH = a42ccf5f08a8b510c0c78da1ba889532a0ce24e772b576604faf09b4d6a0f771

# A fips build with algvs tests and any patches created, this does not
# follow the policy and is here for testing and development only
#BUILD_FIPS_ALGVS = y

endif

###############################################################################

OPENSSL_INSTALL = $(shell pwd)/build/$(OPENSSL_PKG)-install
OPENSSL_SRC = $(shell pwd)/build/$(OPENSSL_PKG)
FIPS_INSTALL = $(shell pwd)/build/$(FIPS_PKG)-install
FIPS_SRC = $(shell pwd)/build/$(FIPS_PKG)

$(OPENSSL_PKG)_URL       = https://www.openssl.org/source/$(OPENSSL_PKG).tar.gz
$(OPENSSL_PKG)_NODEFCONF = 1
$(OPENSSL_PKG)_CONFIGURE = config
$(OPENSSL_PKG)_INSTALL   = install_sw
$(OPENSSL_PKG)_CONFVARS += CROSS_COMPILE=
$(OPENSSL_PKG)_CONFOPTS += shared

#
# Some specific targets cannot be guessed properly by "config". You need
# to explicitly override the guessing and call Configure direct.
# To complicate things even more Configure is a perl script, and will fail
# on our usual automake configure (since we use bash directly).
# So allow it to run the config first time (and get it wrong) then call
# Configure with the correct machine type.
#
ifdef CONFIG_MIPS
ifdef CONFIG_64BIT
$(OPENSSL_PKG)_CONFOPTS = ; $(CONFVARS) $($*_CONFVARS) CFLAGS="$(CFLAGS) $($*_CFLAGS)" ./Configure linux64-mips64
endif
endif

$(OPENSSL_PKG)_CONFOPTS += no-rc2
$(OPENSSL_PKG)_CONFOPTS += no-rc5
$(OPENSSL_PKG)_CONFOPTS += no-md2
$(OPENSSL_PKG)_CONFOPTS += no-mdc2
$(OPENSSL_PKG)_CONFOPTS += no-idea
#$(OPENSSL_PKG)_CONFOPTS += no-pem
#$(OPENSSL_PKG)_CONFOPTS += no-md5
#$(OPENSSL_PKG)_CONFOPTS += no-sha
#$(OPENSSL_PKG)_CONFOPTS += no-hmac
#$(OPENSSL_PKG)_CONFOPTS += no-des
#$(OPENSSL_PKG)_CONFOPTS += no-aes
#$(OPENSSL_PKG)_CONFOPTS += no-bn
#$(OPENSSL_PKG)_CONFOPTS += no-ecdh
#$(OPENSSL_PKG)_CONFOPTS += no-ecdsa
#$(OPENSSL_PKG)_CONFOPTS += no-ec
#$(OPENSSL_PKG)_CONFOPTS += no-rsa
#$(OPENSSL_PKG)_CONFOPTS += no-dsa
#$(OPENSSL_PKG)_CONFOPTS += no-dh
$(OPENSSL_PKG)_CONFOPTS += no-dso
#$(OPENSSL_PKG)_CONFOPTS += no-engine
#$(OPENSSL_PKG)_CONFOPTS += no-buffer
#$(OPENSSL_PKG)_CONFOPTS += no-bio
#$(OPENSSL_PKG)_CONFOPTS += no-stack
#$(OPENSSL_PKG)_CONFOPTS += no-lhash
#$(OPENSSL_PKG)_CONFOPTS += no-rand
$(OPENSSL_PKG)_CONFOPTS += no-err
#$(OPENSSL_PKG)_CONFOPTS += no-evp
#$(OPENSSL_PKG)_CONFOPTS += no-asn1
#$(OPENSSL_PKG)_CONFOPTS += no-x509
#$(OPENSSL_PKG)_CONFOPTS += no-x509v3
#$(OPENSSL_PKG)_CONFOPTS += no-txt_db
#$(OPENSSL_PKG)_CONFOPTS += no-pkcs7
#$(OPENSSL_PKG)_CONFOPTS += no-pkcs12
#$(OPENSSL_PKG)_CONFOPTS += no-comp
#$(OPENSSL_PKG)_CONFOPTS += no-ocsp
#$(OPENSSL_PKG)_CONFOPTS += no-ui
#$(OPENSSL_PKG)_CONFOPTS += no-store
#$(OPENSSL_PKG)_CONFOPTS += no-pqueue
#$(OPENSSL_PKG)_CONFOPTS += no-tlsext
#$(OPENSSL_PKG)_CONFOPTS += no-ssl2
#$(OPENSSL_PKG)_CONFOPTS += no-ssl3

# REVISIT: It would be better to have OPENSSL config options
# which turn on this support as needed
ifeq ($(CONFIG_USER_NESSUS_NASL)$(CONFIG_USER_SSH_SSH)$(CONFIG_USER_SSH_SSHD),)
$(OPENSSL_PKG)_CONFOPTS += no-ripemd
$(OPENSSL_PKG)_CONFOPTS += no-cast
$(OPENSSL_PKG)_CONFOPTS += no-rc4
endif

ifeq ($(CONFIG_USER_NESSUS_NASL)$(CONFIG_USER_SSH_SSH)$(CONFIG_PROP_SSCEP_SSCEP)$(CONFIG_USER_SSH_SSHD),)
$(OPENSSL_PKG)_CONFOPTS += no-bf
endif

ifeq ($(CONFIG_USER_OPENVPN_OPENVPN)$(CONFIG_USER_WGET)$(CONFIG_USER_WPA_SUPPLICANT)$(CONFIG_USER_HOSTAPD_HOSTAPD),)
$(OPENSSL_PKG)_CONFOPTS += no-md4
endif

$(OPENSSL_PKG)_CONFOPTS += --prefix=// --libdir=/lib
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
$(OPENSSL_PKG)_CONFOPTS += --openssldir=/etc/config/ssl
else
$(OPENSSL_PKG)_CONFOPTS += --openssldir=/etc/ssl
endif

##############################################################################
#################### OpenSSL 1.1.X build options #############################
##############################################################################

ifneq ($(findstring openssl-1.1,$(OPENSSL_PKG)),)

ifdef CONFIG_DEFAULTS_LIBC_UCLIBC
ifndef UCLIBC_HAS_CONTEXT_FUNCS
$(OPENSSL_PKG)_CONFOPTS += no-async
endif
endif
$(OPENSSL_PKG)_CONFOPTS += --with-rand-seed=devrandom
$(OPENSSL_PKG)_CONFOPTS += '-DDEVRANDOM="\"/dev/urandom\", \"/dev/random\", \"/dev/hwrng\", \"/dev/srandom\""'

ifdef CONFIG_OCF_OCF
$(OPENSSL_PKG)_CONFOPTS += enable-devcryptoeng
endif

ifdef CONFIG_MODULES_CRYPTODEV_CRYPTODEV
$(OPENSSL_PKG)_CONFOPTS += enable-devcryptoeng
endif

ifdef CONFIG_ARM64
MACHINE=aarch64
endif

$(OPENSSL_PKG)_CONFVARS += MACHINE=$(MACHINE)

endif

##############################################################################
#################### OpenSSL 1.0.2 build options #############################
##############################################################################

ifneq ($(findstring openssl-1.0.2,$(OPENSSL_PKG)),)
$(OPENSSL_PKG)_PATCHES += openssl-1.0.2.patch

# libcrypto and libssl aren't in the default location when building apps
LDFLAGS += -L$(shell pwd)/build/$(OPENSSL_PKG)

$(OPENSSL_PKG)_CONFOPTS += --install_prefix=$(OPENSSL_INSTALL)

ifdef CONFIG_64BIT
MACHINE=uClinux-dist64
else
MACHINE=uClinux-dist
endif
$(OPENSSL_PKG)_CONFVARS += MACHINE=$(MACHINE)

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
$(OPENSSL_PKG)_CONFOPTS += no-asm
endif
endif
ifdef CONFIG_ARM64
export UCDIST_ASM=aarch64_asm
export UCDIST_TYP=linux64
endif
ifndef UCDIST_ASM
export UCDIST_ASM=no_asm
$(OPENSSL_PKG)_CONFOPTS += no-asm
endif

ifdef CONFIG_OCF_OCF
$(OPENSSL_PKG)_CONFOPT += -DHAVE_CRYPTODEV
#$(OPENSSL_PKG)_CONFOPTS+= -DUSE_CRYPTODEV_DIGESTS
endif

ifdef CONFIG_MODULES_CRYPTODEV_CRYPTODEV
$(OPENSSL_PKG)_CONFOPTS += -DHAVE_CRYPTODEV
$(OPENSSL_PKG)_CONFOPTS += -DUSE_CRYPTODEV_DIGESTS
YOU_WANT_DYNAMIC_HW_ENGINES_ENABLED = y
$(OPENSSL_PKG)_PATCHES += openssl-1.0.2-cryptodev-module.patch
endif

# this fixes the engines install (openssl-1 and later),  even though we
# explicitly state no-dso meaning we don't want them
$(OPENSSL_PKG)_CONFOPTS += -DMY_DSO_DLFCN_HACK

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
$(OPENSSL_PKG)_CONFOPTS += no-hw
LIBSSL_dlfcn =
endif
export LIBSSL_dlfcn

$(OPENSSL_PKG)_CONFOPTS += no-krb5

ifdef CONFIG_LIB_LIBSSL_FIPS
$(OPENSSL_PKG)_CONFVARS  += FIPSDIR=$(FIPS_INSTALL)
$(OPENSSL_PKG)_CONFOPTS  += fips --with-fipslibdir=$(FIPS_INSTALL)/lib/
$(OPENSSL_PKG)_CFLAGS    += -I$(FIPS_INSTALL)/include -Wl,-L$(FIPS_INSTALL)/lib
$(OPENSSL_PKG)_BUILDVARS += FIPS_SIG=$(FIPS_SRC)/util/incore
$(OPENSSL_PKG)_DEP       += $(FIPS_PKG)
endif

# Extra post-configure pre-make steps
$(OPENSSL_PKG)_CONFOPTS += ; $(MAKE) depend links

endif # end OpenSSL 1.0.2 build options

###############################################################################
# OpenSSL FIPS
###############################################################################

$(FIPS_PKG)_URL        = https://www.openssl.org/source/$(FIPS_PKG).tar.gz
$(FIPS_PKG)_NODEFCONF  = 1
$(FIPS_PKG)_CONFIGURE  = config
$(FIPS_PKG)_INSTALL    = install_sw
$(FIPS_PKG)_CONFVARS   = CROSS_COMPILE=
$(FIPS_PKG)_CONFVARS  += INSTALL_PREFIX=$(FIPS_INSTALL)
$(FIPS_PKG)_CONFVARS  += FIPSDIR="/"
$(FIPS_PKG)_CONFOPTS  += no-asm
$(FIPS_PKG)_BUILDVARS += FIPS_SIG=$(FIPS_SRC)/util/incore
$(FIPS_PKG)_MAKEVARS  += -j1

ifdef BUILD_FIPS_ALGVS
$(FIPS_PKG)_BUILD = all build_algvs
endif

###############################################################################

AUTOMAKE_$(CONFIG_LIB_LIBSSL_FIPS) += $(FIPS_PKG)
AUTOMAKE_y += $(OPENSSL_PKG)
AUTOMAKE_ROMFS = openssl_romfs

include $(ROOTDIR)/tools/automake.inc

openssl_romfs:
	cd build/$(OPENSSL_PKG)-install/lib; \
	for i in *.so*; do \
		if [ -L $$i ]; then \
			$(ROMFSINST) -s `find $$i -printf %l` /lib/$$i; \
		elif [ -f $$i ]; then \
			$(ROMFSINST) /lib/$$i; \
		fi; \
	done
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS $(OPENSSL_INSTALL)/bin/openssl /bin/openssl
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS -d $(OPENSSL_SRC)/apps/openssl.cnf /etc/default/ssl/openssl.cnf
else
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS -d $(OPENSSL_SRC)/apps/openssl.cnf /etc/ssl/openssl.cnf
endif
	# $(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS $(OPENSSL_INSTALL)/bin/c_rehash /bin/c_rehash

ifdef CONFIG_LIB_LIBSSL_FIPS
openssl_romfs: fips_romfs

fips_romfs:
	$(ROMFSINST) -e BUILD_FIPS_ALGVS $(FIPS_SRC)/test/fips_algvs /bin/fips_algvs
endif
