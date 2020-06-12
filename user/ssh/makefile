
CONFOPTS  = --libexecdir=/usr/libexec
CONFOPTS += --sbindir=/bin
CONFOPTS += --disable-etc-default-login
CONFOPTS += --without-pie
INSTALL = install-nokeys

ifdef HAS_SHADOW
CONFOPTS += --with-shadow
else
CONFOPTS += --without-shadow
endif

# CONFOPTS += --disable-shared
# CONFOPTS += --disable-static

CONFOPTS += --without-stackprotect

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

# If we're building with libpam, we add this capability to ssh
ifdef CONFIG_LIB_LIBPAM
NEED_LIB_PAM := y
endif
ifdef CONFIG_LIB_LIBPAM_FORCE
NEED_LIB_PAM := y
endif
ifeq ($(NEED_LIB_PAM),y)
CONFOPTS += --with-pam
endif

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

OPENSSH_VERSION = 8.2p1
URL = http://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-$(OPENSSH_VERSION).tar.gz

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

OPENSSH_INSTALL_DIR = build/openssh-$(OPENSSH_VERSION)-install

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_SSH_SSH $(OPENSSH_INSTALL_DIR)/bin/ssh /bin/ssh
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SSH $(OPENSSH_INSTALL_DIR)$(SSHCONFDIR)/ssh_config $(SSHDEFDIR)/ssh_config
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD $(OPENSSH_INSTALL_DIR)/bin/sshd /bin/sshd
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN $(OPENSSH_INSTALL_DIR)/bin/ssh-keygen /bin/ssh-keygen
	$(ROMFSINST) -e CONFIG_USER_SSH_GEN_KEYS $(OPENSSH_INSTALL_DIR)/bin/gen-keys /bin/gen-keys
	$(ROMFSINST) -e CONFIG_USER_SSH_SCP $(OPENSSH_INSTALL_DIR)/bin/scp /bin/scp
	$(ROMFSINST) -e CONFIG_USER_SSH_SFTP $(OPENSSH_INSTALL_DIR)/bin/sftp /bin/sftp
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SFTP_SERVER $(OPENSSH_INSTALL_DIR)/usr/libexec/sftp-server /usr/libexec/sftp-server
	$(ROMFSINST) -d -e CONFIG_USER_SSH_SSH_KEYSIGN $(OPENSSH_INSTALL_DIR)/usr/libexec/ssh-keysign /usr/libexec/ssh-keysign
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
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-A "nobody:" -a "nobody:x:65534:" $(SYSDEFDIR)/group
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD \
		-e CONFIG_USER_FLATFSD_ETC_CONFIG \
		-a "#ssh details are now in $(SYSDEFDIR)/inetd.conf" /etc/inetd.conf
ifdef CONFIG_USER_SSH_SSHD
ifdef CONFIG_USER_FLATFSD_FLATFSD
ifndef CONFIG_USER_FLATFSD_ETC_CONFIG
	for i in ssh_host_key ssh_host_rsa_key ssh_host_dsa_key; do \
		$(ROMFSINST) -s /etc/config/$$i $(SSHDEFDIR)/$$i; \
		$(ROMFSINST) -s /etc/config/$$i.pub $(SSHDEFDIR)/$$i.pub; \
	done
endif
endif
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
