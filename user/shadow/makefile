VERSION=4.6
URL = https://github.com/shadow-maint/shadow/releases/download/$(VERSION)/shadow-$(VERSION).tar.gz

CONFOPTS += --enable-dependency-tracking
CONFOPTS += --disable-largefile
CONFOPTS += --disable-man
CONFOPTS += --disable-nls
CONFOPTS += --without-selinux
CONFOPTS += --without-audit
CONFOPTS += --without-skey
CONFOPTS += --without-libcrack

ifdef CONFIG_USER_SHADOW_PAM
CONFOPTS += --with-libpam
# The link test for this fails due to not linking with -lpam, so override
CONFVARS += ac_cv_lib_pam_misc_misc_conv=yes
endif
ifdef CONFIG_USER_SHADOW_SHAREDLIB
CONFOPTS += --enable-shared
endif

ifdef CONFIG_DEFAULTS_LIBC_UCLIBC_NG
LDFLAGS += -lrt
endif

# Have to specify the correct paths to various files
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CFLAGS += -DPASSWD_FILE='\"/etc/config/passwd\"'
CFLAGS += -DGROUP_FILE='\"/etc/config/group\"'
CFLAGS += -DSHADOW_FILE='\"/etc/config/shadow\"'
CFLAGS += -DSGROUP_FILE='\"/etc/config/gshadow\"'
CFLAGS += -DLOGINDEFS='\"/etc/config/login.defs\"'
endif

AUTOMAKE_ROMFS = shadow_romfs

include $(ROOTDIR)/tools/automake.inc

BINPROGS := login su groups

SBINPROGS := sulogin

USRBINPROGS := chage chfn chsh expiry faillog gpasswd groups id
USRBINPROGS += lastlog newgrp passwd

USRSBINPROGS := chpasswd groupadd groupdel groupmod grpck grpconv
USRSBINPROGS += grpunconv logoutd newusers nologin pwck pwconv pwunconv
USRSBINPROGS += useradd userdel usermod vipw

BINPATH = $(CURDIR)/build/shadow-$(VERSION)-install/bin
SBINPATH = $(CURDIR)/build/shadow-$(VERSION)-install/sbin

shadow_romfs:
ifdef CONFIG_USER_SHADOW_SHAREDLIB
	$(ROMFSINST) build/lib/.libs/libshadow.so.0.0.0 /lib/libshadow.so.0.0.0
	$(ROMFSINST) -s libshadow.so.0.0.0 /lib/libshadow.so.0
	$(ROMFSINST) -s libshadow.so.0.0.0 /lib/libshadow.so
endif
	@for f in $(BINPROGS);						\
	do								\
		ev="CONFIG_USER_SHADOW_`echo $$f | tr a-z A-Z`";	\
		$(ROMFSINST) -e "$$ev" $(BINPATH)/$$f /bin/$$f;		\
	done
	@for f in $(SBINPROGS);						\
	do								\
		ev="CONFIG_USER_SHADOW_`echo $$f | tr a-z A-Z`";	\
		$(ROMFSINST) -e "$$ev" $(SBINPATH)/$$f /sbin/$$f;	\
	done
	@for f in $(USRBINPROGS);					\
	do								\
		ev="CONFIG_USER_SHADOW_`echo $$f | tr a-z A-Z`";	\
		$(ROMFSINST) -e "$$ev" $(BINPATH)/$$f /usr/bin/$$f;	\
	done
	@for f in $(USRSBINPROGS);					\
	do								\
		ev="CONFIG_USER_SHADOW_`echo $$f | tr a-z A-Z`";	\
		$(ROMFSINST) -e "$$ev" $(SBINPATH)/$$f /usr/sbin/$$f;	\
	done

