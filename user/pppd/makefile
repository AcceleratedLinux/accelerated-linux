URL = https://github.com/paulusmack/ppp/archive/ppp-2.4.7.tar.gz
BUILDDIR = ppp-ppp-2.4.7
INSTALL_DIR = build/$(PKG)-install

export ACL_PKG=ppp-2.4.8+3
export ACL_URL=https://github.com/paulusmack/ppp/commit/8d45443bb5c9372b4c6a362ba2f443d41c5636af

CONFOPTS=
UCFRONT_ENV=

ifdef CONFIG_DEFAULTS_LIBC_MUSL
CFLAGS += -D__GLIBC__=2 -D__GLIBC_MINOR__=1
endif
ifdef CONFIG_DEFAULTS_LIBC_GLIBC
#
# Newer versions of glibc do not contain the obsolete crypto API functions.
# So we must use openssl to provide crypto functions if using that.
#
CONFIG_USER_PPPD_USE_OPENSSL=y
endif

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -e CONFIG_USER_PPPD_PPPD_PPPD $(INSTALL_DIR)/sbin/pppd /bin/pppd
	$(ROMFSINST) -e CONFIG_USER_CHAT_CHAT $(INSTALL_DIR)/sbin/chat /bin/chat
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_DYNAMIC_PLUGINS -e CONFIG_USER_PPPD_WITH_PPPOE -d $(INSTALL_DIR)/lib/pppd/2.4.8/rp-pppoe.so /etc/ppp/plugins/rp-pppoe.so
	$(ROMFSINST) -d -s /var/run/pppoe.conf /etc/ppp/pppoe.conf

ifdef CONFIG_USER_PPPD_WITH_RADIUS
	[ -d $(ROMFSDIR)/etc/radiusclient ] || mkdir $(ROMFSDIR)/etc/radiusclient
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary /etc/radiusclient/dictionary
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.ms /etc/radiusclient/dictionary.ms
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.sg /etc/radiusclient/dictionary.sg
endif
