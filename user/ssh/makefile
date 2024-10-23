OPENSSH_VERSION = 9.8p1
URL = https://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-$(OPENSSH_VERSION).tar.gz

CONFOPTS  = --libexecdir=/usr/libexec
CONFOPTS += --sbindir=/bin
CONFOPTS += --disable-etc-default-login
INSTALL = install-nokeys

ifdef HAS_SHADOW
CONFOPTS += --with-shadow
else
CONFOPTS += --without-shadow
endif

# CONFOPTS += --disable-shared
# CONFOPTS += --disable-static

CONFOPTS += --without-stackprotect --disable-lastlog --disable-wtmp

ifdef CONFIG_USER_FLATFSD_FLATFSD
SYSCONFDIR = /etc/config
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
SYSDEFDIR = /etc/default
CONFOPTS += --sysconfdir=/etc/config
SSHCONFDIR = /etc/config
SSHDEFDIR = /etc/default
else
SYSDEFDIR = /etc
CONFOPTS += --sysconfdir=/etc/ssh
SSHCONFDIR = /etc/ssh
SSHDEFDIR = /etc/ssh
endif
else
SYSCONFDIR = /etc
SYSDEFDIR = /etc
SSHCONFDIR = /etc
SSHDEFDIR = /etc
endif

CONFOPTS-$(CONFIG_LIB_LIBPAM) += --with-pam
CONFOPTS-$(CONFIG_USER_SSH_PKCS11) += --enable-pkcs11
CONFOPTS-$(CONFIG_USER_SSH_SK) += --enable-security-key
CONFOPTS += $(patsubst --with-%,--without-%,\
		$(CONFOPTS-:--enable-%=--disable-%))
CONFOPTS += $(CONFOPTS-y)

FLTFLAGS += -s 65536
ifdef CONFIG_PROP_SSH_SECURITY_COUNT
CFLAGS += -DSECURITY_COUNTS
endif
ifeq ($(CONFIG_USER_SSH_SSHD),y)
CFLAGS += -DINCLUDE_SSHD
endif
ifeq ($(CONFIG_USER_SSH_SSH),y)
CFLAGS += -DINCLUDE_SSH
endif

CONFVARS = INSTALL="install -c --strip-program=$(STRIP)"

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

OPENSSH_INSTALL_DIR = build/openssh-$(OPENSSH_VERSION)-install

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_SSH_SSH $(OPENSSH_INSTALL_DIR)/bin/ssh /bin/ssh
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SSH $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/ssh_config $(SSHDEFDIR)/ssh_config
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD $(OPENSSH_INSTALL_DIR)/bin/sshd /bin/sshd
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SSHD $(OPENSSH_INSTALL_DIR)/usr/libexec/sshd-session /usr/libexec/sshd-session
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN $(OPENSSH_INSTALL_DIR)/bin/ssh-keygen /bin/ssh-keygen
	$(ROMFSINST) -e CONFIG_USER_SSH_GEN_KEYS $(OPENSSH_INSTALL_DIR)/bin/gen-keys /bin/gen-keys
	$(ROMFSINST) -e CONFIG_USER_SSH_SCP $(OPENSSH_INSTALL_DIR)/bin/scp /bin/scp
	$(ROMFSINST) -e CONFIG_USER_SSH_SFTP $(OPENSSH_INSTALL_DIR)/bin/sftp /bin/sftp
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SFTP_SERVER $(OPENSSH_INSTALL_DIR)/usr/libexec/sftp-server /usr/libexec/sftp-server
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SSH_KEYSIGN $(OPENSSH_INSTALL_DIR)/usr/libexec/ssh-keysign /usr/libexec/ssh-keysign
	$(ROMFSINST) -d -e CONFIG_USER_SSH_PKCS11 $(OPENSSH_INSTALL_DIR)/usr/libexec/ssh-pkcs11-helper /usr/libexec/ssh-pkcs11-helper
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SK $(OPENSSH_INSTALL_DIR)/usr/libexec/ssh-sk-helper /usr/libexec/ssh-sk-helper
	##
	# Add the appropriate stuff to the start files.
	#
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD_CONFIG $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/sshd_config $(SSHDEFDIR)/sshd_config
	$(ROMFSINST) -e CONFIG_USER_SSH_GEN_KEYS \
		-e CONFIG_USER_FLATFSD_ETC_CONFIG \
		-a "gen-keys &" $(SYSDEFDIR)/start
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
		-A "root:" -a "root:abTxyM1SdxNko:0:0:Root:/:/bin/sh" \
		$(SYSDEFDIR)/passwd
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
		-A "root:" -a "root:x:0:root" $(SYSDEFDIR)/group
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-A "sshd:" -a "sshd:x:100:65534::/var/run/sshd:/bin/false" \
		$(SYSDEFDIR)/passwd
ifdef CONFIG_PROP_OGCS
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-a "sshd:x:100:65534::/var/run/sshd:/bin/false" \
		$(SYSDEFDIR)/passwd.d/sshd
endif
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-A "nobody:" -a "nobody:x:65534:" $(SYSDEFDIR)/group
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD \
		-e CONFIG_USER_FLATFSD_ETC_CONFIG \
		-a "#ssh details are now in $(SYSDEFDIR)/inetd.conf" /etc/inetd.conf
	# Install the default sshd config for Opengear builds
	$(ROMFSINST) -e CONFIG_DEFAULTS_OPENGEAR $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/sshd_config /etc/default/sshd_config
ifdef CONFIG_USER_SSH_SSHD
ifdef CONFIG_USER_FLATFSD_FLATFSD
ifndef CONFIG_USER_FLATFSD_ETC_CONFIG
	for i in ssh_host_key ssh_host_rsa_key ssh_host_dsa_key ssh_host_ecdsa_key ssh_host_ed25519_key; do \
		$(ROMFSINST) -s /etc/config/$$i $(SSHDEFDIR)/$$i; \
		$(ROMFSINST) -s /etc/config/$$i.pub $(SSHDEFDIR)/$$i.pub; \
	done
	$(ROMFSINST) -s /etc/config/moduli $(SSHDEFDIR)/moduli
endif
endif
endif
ifdef CONFIG_LIB_OPENSSL_FIPS
	#
	# Install FIPS versions of ssh*_config files
	#
	rm -f $(ROMFSDIR)$(SSHDEFDIR)/ssh_config
	$(ROMFSINST) $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/ssh_config $(SSHDEFDIR)/ssh_config.org
	$(ROMFSINST) $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/ssh_config $(SSHDEFDIR)/ssh_config.fips
	rm -f $(ROMFSDIR)$(SSHDEFDIR)/sshd_config
	$(ROMFSINST) $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/sshd_config $(SSHDEFDIR)/sshd_config.org
	$(ROMFSINST) $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/sshd_config $(SSHDEFDIR)/sshd_config.fips
	patch -d $(ROMFSDIR)$(SSHDEFDIR) --no-backup-if-mismatch -p1 < patches/ssh_config-fips.patch
	$(ROMFSINST) -s /var/run/ssh_config $(SSHCONFDIR)/ssh_config
	$(ROMFSINST) -s /var/run/sshd_config $(SSHCONFDIR)/sshd_config
endif
	##
	# Optionally add the startup stuff to the /etc/default-static dir.
	#
	if [ -d $(ROMFSDIR)/etc/default-static ] ; then \
		$(ROMFSINST) -e CONFIG_USER_SSH_GEN_KEYS \
			-a "gen-keys &" /etc/default-static/start ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
			-a "root:abTxyM1SdxNko:0:0:Root:/:/bin/sh" -A "root:"\
			/etc/default-static/passwd ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
			-A "root:" -a "root:x:0:root" /etc/default-static/group ; \
		$(ROMFSINST) -e CONFIG_USER_SSH_SSH /etc/default-static/ssh_config ; \
		$(ROMFSINST) -e CONFIG_USER_SSH_SSHD_CONFIG /etc/default-static/sshd_config ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
			-A "sshd:" -a "sshd:x:100:65534::/var/run/sshd:/bin/false" \
			/etc/default-static/passwd ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
			-A "nobody:" -a "nobody:x:65534:" /etc/default-static/group ;\
	fi

