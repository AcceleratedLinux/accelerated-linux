##########################################################################
#
# This build supports either version 3.0 or legacy 1.1.
#
##########################################################################
ifdef CONFIG_LIB_OPENSSL3
##########################################################################

VERSION = 3.0.7
URL = https://www.openssl.org/source/openssl-$(VERSION).tar.gz

openssl-$(VERSION)_CONFIGURE = ./config
openssl-$(VERSION)_NODEFCONF = y

CONFVARS += CROSS_COMPILE=""

TARGET = linux-generic32
ifeq ($(MACHINE),arm)
TARGET = linux-armv4
endif
ifeq ($(MACHINE),arm64)
TARGET = linux-aarch64
endif
ifeq ($(MACHINE),mips)
TARGET = linux-mips32
endif
ifeq ($(MACHINE),mips64)
TARGET = linux64-mips64
endif
ifeq ($(MACHINE),riscv64)
TARGET = linux64-riscv64
endif
ifeq ($(MACHINE),i386)
TARGET = linux-x86
endif
ifeq ($(MACHINE),x86_64)
TARGET = linux-x86_64
endif

openssl-$(VERSION)_CONFOPTS += --prefix=/
openssl-$(VERSION)_CONFOPTS += --libdir=/lib
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
openssl-$(VERSION)_CONFOPTS += --openssldir=/etc/config/ssl
else
openssl-$(VERSION)_CONFOPTS += --openssldir=/etc/ssl
endif

openssl-$(VERSION)_CONFOPTS += $(TARGET)

ifdef CONFIG_LIB_OPENSSL_FIPS
openssl-$(VERSION)_CONFOPTS += enable-fips
else
openssl-$(VERSION)_CONFOPTS += no-dso
endif

openssl-$(VERSION)_CONFOPTS += no-rc2
openssl-$(VERSION)_CONFOPTS += no-rc5
openssl-$(VERSION)_CONFOPTS += no-md2
openssl-$(VERSION)_CONFOPTS += no-mdc2
openssl-$(VERSION)_CONFOPTS += no-idea
#openssl-$(VERSION)_CONFOPTS += no-pem
#openssl-$(VERSION)_CONFOPTS += no-des
#openssl-$(VERSION)_CONFOPTS += no-bn
#openssl-$(VERSION)_CONFOPTS += no-ecdh
#openssl-$(VERSION)_CONFOPTS += no-ecdsa
#openssl-$(VERSION)_CONFOPTS += no-ec
#openssl-$(VERSION)_CONFOPTS += no-dsa
#openssl-$(VERSION)_CONFOPTS += no-dh
#openssl-$(VERSION)_CONFOPTS += no-engine
#openssl-$(VERSION)_CONFOPTS += no-buffer
#openssl-$(VERSION)_CONFOPTS += no-bio
#openssl-$(VERSION)_CONFOPTS += no-stack
#openssl-$(VERSION)_CONFOPTS += no-lhash
#openssl-$(VERSION)_CONFOPTS += no-rand
openssl-$(VERSION)_CONFOPTS += no-err
#openssl-$(VERSION)_CONFOPTS += no-evp
#openssl-$(VERSION)_CONFOPTS += no-asn1
#openssl-$(VERSION)_CONFOPTS += no-x509
#openssl-$(VERSION)_CONFOPTS += no-x509v3
#openssl-$(VERSION)_CONFOPTS += no-txt_db
#openssl-$(VERSION)_CONFOPTS += no-pkcs7
#openssl-$(VERSION)_CONFOPTS += no-pkcs12
#openssl-$(VERSION)_CONFOPTS += no-comp
#openssl-$(VERSION)_CONFOPTS += no-ocsp
#openssl-$(VERSION)_CONFOPTS += no-ui
#openssl-$(VERSION)_CONFOPTS += no-store
#openssl-$(VERSION)_CONFOPTS += no-pqueue
#openssl-$(VERSION)_CONFOPTS += no-tlsext
#openssl-$(VERSION)_CONFOPTS += no-ssl2
#openssl-$(VERSION)_CONFOPTS += no-ssl3

AUTOMAKE_ROMFS = openssl_romfs

CFLAGS += -g

include $(ROOTDIR)/tools/automake.inc

OPENSSL_INSTALL = build/openssl-$(VERSION)-install

openssl_romfs:
	cd $(OPENSSL_INSTALL)/lib; \
	for i in *.so*; do \
		if [ -L $$i ]; then \
			$(ROMFSINST) -s `find $$i -printf %l` /lib/$$i; \
		elif [ -f $$i ]; then \
			$(ROMFSINST) /lib/$$i; \
		fi; \
	done
	if [ -f $(OPENSSL_INSTALL)/lib/ossl-modules/fips.so ]; then \
		$(ROMFSINST) -S -d $(OPENSSL_INSTALL)/lib/ossl-modules/fips.so /lib/ossl-modules/fips.so ; \
		$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS -d $(OPENSSL_INSTALL)/etc/ssl/fipsmodule.cnf /etc/ssl/fipsmodule.cnf ; \
	fi
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS $(OPENSSL_INSTALL)/bin/openssl /bin/openssl
	$(ROMFSINST) -e CONFIG_USER_OPENSSL_APPS $(OPENSSL_INSTALL)/bin/c_rehash /bin/c_rehash
ifdef CONFIG_USER_OPENSSL_APPS
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -d $(OPENSSL_INSTALL)/etc/config/ssl/openssl.cnf /etc/default/ssl/openssl.cnf
else
ifdef CONFIG_LIB_OPENSSL_FIPS
	$(ROMFSINST) -d $(OPENSSL_INSTALL)/etc/ssl/openssl.cnf /etc/ssl/openssl.cnf.org
	$(ROMFSINST) -d $(OPENSSL_INSTALL)/etc/ssl/openssl.cnf /etc/ssl/openssl.cnf.fips
	patch -d $(ROMFSDIR)/etc/ssl -p3 < patches/openssl-fips-cnf.patch
	$(ROMFSINST) -s /var/run/openssl.cnf /etc/ssl/openssl.cnf
else
	$(ROMFSINST) -d $(OPENSSL_INSTALL)/etc/ssl/openssl.cnf /etc/ssl/openssl.cnf
endif
endif
endif

##########################################################################
else
##########################################################################

OPENSSL_PKG = openssl-1.1.1s

OPENSSL_INSTALL = $(shell pwd)/build/$(OPENSSL_PKG)-install
OPENSSL_SRC = $(shell pwd)/build/$(OPENSSL_PKG)

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

#$(OPENSSL_PKG)_CONFOPTS += enable-tls1_2
#$(OPENSSL_PKG)_CONFOPTS += enable-tls1_3
$(OPENSSL_PKG)_CONFOPTS += enable-weak-ssl-ciphers

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
ifeq ($(CONFIG_USER_SSH_SSH)$(CONFIG_USER_SSH_SSHD),)
$(OPENSSL_PKG)_CONFOPTS += no-ripemd
$(OPENSSL_PKG)_CONFOPTS += no-cast
$(OPENSSL_PKG)_CONFOPTS += no-rc4
endif

ifeq ($(CONFIG_USER_SSH_SSH)$(CONFIG_PROP_SSCEP_SSCEP)$(CONFIG_USER_SSH_SSHD),)
$(OPENSSL_PKG)_CONFOPTS += no-bf
endif

ifeq ($(CONFIG_USER_OPENVPN_OPENVPN)$(CONFIG_USER_WGET)$(CONFIG_USER_WPA_SUPPLICANT)$(CONFIG_USER_HOSTAPD_HOSTAPD)$(CONFIG_LIB_LIBLDAP)$(CONFIG_USER_RSYNC_RSYNC),)
$(OPENSSL_PKG)_CONFOPTS += no-md4
endif

$(OPENSSL_PKG)_CONFOPTS += --prefix=// --libdir=/lib
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
$(OPENSSL_PKG)_CONFOPTS += --openssldir=/etc/config/ssl
else
$(OPENSSL_PKG)_CONFOPTS += --openssldir=/etc/ssl
endif

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

$(OPENSSL_PKG)_MAKEVARS += BIN_CFLAGS="-fPIC -pie"

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

##########################################################################
endif
