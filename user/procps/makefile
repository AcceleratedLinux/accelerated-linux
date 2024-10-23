VERSION = 3.3.15
URL = https://downloads.sourceforge.net/project/procps-ng/Production/procps-ng-$(VERSION).tar.xz

CONFVARS += ac_cv_func_malloc_0_nonnull=yes
CONFVARS += ac_cv_func_realloc_0_nonnull=yes
CONFOPTS += --without-systemd

LDFLAGS += -ltinfo
ifeq ($(CONFIG_DEFAULTS_LIBC_UCLIBC)$(CONFIG_DEFAULTS_LIBC_UCLIBC_NG),y)
LDFLAGS += -lintl
endif

LIBTOOL = $(ROOTDIR)/user/procps/build/$($(PKG)_BUILDDIR)/libtool

AUTOMAKE_ROMFS = procps_romfs

include $(ROOTDIR)/tools/automake.inc

procps_romfs:
	cd build/procps-ng-$(VERSION)-install/lib ; \
	$(ROMFSINST) -d /lib/libprocps.so.7.1.0
	$(ROMFSINST) -s libprocps.so.7.1.0 /lib/libprocps.so.7
	$(ROMFSINST) -s libprocps.so.7.1.0 /lib/libprocps.so
	cd build/procps-ng-$(VERSION)-install/sbin ; \
	$(ROMFSINST) -d -e CONFIG_USER_PROCPS_SYSCTL /sbin/sysctl
	cd build/procps-ng-$(VERSION)-install/bin ; \
	$(ROMFSINST) -d -e CONFIG_USER_PROCPS_FREE /bin/free ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_KILL /bin/kill ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PGREP /bin/pgrep ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PIDOF /bin/pidof ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PKILL /bin/pkill ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PMAP /bin/pmap ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PS /bin/ps ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_PWDX /bin/pwdx ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_SLABTOP /bin/slabtop ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_TLOAD /bin/tload ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_TOP /bin/top ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_UPTIME /bin/uptime ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_VMSTAT /bin/vmstat ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_W /bin/w ; \
	$(ROMFSINST) -e CONFIG_USER_PROCPS_WATCH /bin/watch
