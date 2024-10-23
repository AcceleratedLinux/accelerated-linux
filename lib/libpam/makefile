
VER := 1.6.1
URL := https://github.com/linux-pam/linux-pam/releases/download/v$(VER)/Linux-PAM-$(VER).tar.xz

AUTORECONF = aclocal

CONFOPTS += --disable-nls
CONFOPTS += --disable-largefile
CONFOPTS += --disable-doc
CONFOPTS += --includedir=/include/security

# pam_env.so reads pam_env.conf from this directory
CONFOPTS += --enable-sconfigdir=/etc/security

ifeq ($(CONFIG_LIB_LIBPAM_PAM_UNIX),y)
  CONFOPTS += --enable-unix --disable-nis
else
  CONFOPTS += --disable-unix
endif

ifeq ($(CONFIG_LIB_LIBSELINUX),y)
  CONFOPTS += --enable-selinux
else
  CONFOPTS += --disable-selinux
endif

AUTOMAKE_ROMFS = libpam_romfs
MAKEVARS := # defeat libtool override
BUILDVARS := LDFLAGS= # don't interfere with libtool

include $(ROOTDIR)/tools/automake.inc

libpam_romfs: I=build/$(PKG)-install
libpam_romfs:
	$(ROMFSINST) $(I)/lib/libpam.so  	/lib/libpam.so.0
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_LIBPAMC	$(I)/lib/libpamc.so	/lib/libpamc.so.0
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_LIBPAM_MISC	$(I)/lib/libpam_misc.so	/lib/libpam_misc.so.0
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_CONF $(I)/conf/pam.conf /etc/default/pam.conf
	mkdir -p $(ROMFSDIR)/lib/security
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_ACCESS	$(I)/lib/security/pam_access.so		/lib/security/pam_access.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_DEBUG	$(I)/lib/security/pam_debug.so		/lib/security/pam_debug.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_DENY	$(I)/lib/security/pam_deny.so		/lib/security/pam_deny.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_ECHO	$(I)/lib/security/pam_echo.so		/lib/security/pam_echo.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_ENV	$(I)/lib/security/pam_env.so		/lib/security/pam_env.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_EXEC	$(I)/lib/security/pam_exec.so		/lib/security/pam_exec.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_FAILDELAY	$(I)/lib/security/pam_faildelay.so	/lib/security/pam_faildelay.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_FAILLOCK	$(I)/lib/security/pam_faillock.so	/lib/security/pam_faillock.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_FAILLOCK	$(I)/sbin/faillock			/sbin/faillock
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_FILTER	$(I)/lib/security/pam_filter.so		/lib/security/pam_filter.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_FTP	$(I)/lib/security/pam_ftp.so		/lib/security/pam_ftp.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_GROUP	$(I)/lib/security/pam_group.so		/lib/security/pam_group.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_ISSUE	$(I)/lib/security/pam_issue.so		/lib/security/pam_issue.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_KEYINIT	$(I)/lib/security/pam_keyinit.so	/lib/security/pam_keyinit.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_LASTLOG	$(I)/lib/security/pam_lastlog.so	/lib/security/pam_lastlog.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_LIMITS	$(I)/lib/security/pam_limits.so		/lib/security/pam_limits.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_LISTFILE	$(I)/lib/security/pam_listfile.so	/lib/security/pam_listfile.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_LOCALUSER	$(I)/lib/security/pam_localuser.so	/lib/security/pam_localuser.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_LOGINUID	$(I)/lib/security/pam_loginuid.so	/lib/security/pam_loginuid.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_MAIL	$(I)/lib/security/pam_mail.so		/lib/security/pam_mail.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_MKHOMEDIR	$(I)/lib/security/pam_mkhomedir.so	/lib/security/pam_mkhomedir.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_MKHOMEDIR	$(I)/sbin/mkhomedir_helper		/sbin/mkhomedir_helper
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_MOTD	$(I)/lib/security/pam_motd.so		/lib/security/pam_motd.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_NAMESPACE	$(I)/lib/security/pam_namespace.so	/lib/security/pam_namespace.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_NAMESPACE	$(I)/sbin/pam_namespace_helper		/sbin/pam_namespace_helper
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_NOLOGIN	$(I)/lib/security/pam_nologin.so	/lib/security/pam_nologin.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_PERMIT	$(I)/lib/security/pam_permit.so		/lib/security/pam_permit.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_PWHISTORY	$(I)/lib/security/pam_pwhistory.so	/lib/security/pam_pwhistory.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_PWHISTORY	$(I)/sbin/pwhistory_helper		/sbin/pwhistory_helper
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_RHOSTS	$(I)/lib/security/pam_rhosts.so		/lib/security/pam_rhosts.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_ROOTOK	$(I)/lib/security/pam_rootok.so		/lib/security/pam_rootok.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SECURETTY	$(I)/lib/security/pam_securetty.so	/lib/security/pam_securetty.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SELINUX	$(I)/lib/security/pam_selinux.so	/lib/security/pam_selinux.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SEPERMIT	$(I)/lib/security/pam_sepermit.so	/lib/security/pam_sepermit.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SETQUOTA	$(I)/lib/security/pam_setquota.so	/lib/security/pam_setquota.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SHELLS	$(I)/lib/security/pam_shells.so		/lib/security/pam_shells.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_STRESS	$(I)/lib/security/pam_stress.so		/lib/security/pam_stress.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_SUCCEED_IF $(I)/lib/security/pam_succeed_if.so	/lib/security/pam_succeed_if.so
#	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TALLY	$(I)/lib/security/pam_tally.so		/lib/security/pam_tally.so
#	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TALLY	$(I)/bin/pam_tally			/bin/pam_tally
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TIME	$(I)/lib/security/pam_time.so		/lib/security/pam_time.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TIMESTAMP	$(I)/lib/security/pam_timestamp.so	/lib/security/pam_timestamp.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TIMESTAMP	$(I)/sbin/pam_timestamp_check		/sbin/pam_timestamp_check
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_TTY_AUDIT	$(I)/lib/security/pam_tty_audit.so	/lib/security/pam_tty_audit.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_UMASK	$(I)/lib/security/pam_umask.so		/lib/security/pam_umask.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_UNIX	$(I)/lib/security/pam_unix.so		/lib/security/pam_unix.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_UNIX	$(I)/sbin/unix_chkpwd			/sbin/unix_chkpwd
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_UNIX \
		     -e CONFIG_LIB_LIBSELINUX		$(I)/sbin/unix_update			/sbin/unix_update
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_USERDB	$(I)/lib/security/pam_userdb.so		/lib/security/pam_userdb.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_USERTYPE	$(I)/lib/security/pam_usertype.so	/lib/security/pam_usertype.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_WARN	$(I)/lib/security/pam_warn.so		/lib/security/pam_warn.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_WHEEL	$(I)/lib/security/pam_wheel.so		/lib/security/pam_wheel.so
	$(ROMFSINST) -e CONFIG_LIB_LIBPAM_PAM_XAUTH	$(I)/lib/security/pam_xauth.so		/lib/security/pam_xauth.so


