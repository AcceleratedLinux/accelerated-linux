URL = https://github.com/paulusmack/ppp/archive/ppp-2.4.7.tar.gz
BUILDDIR = ppp-ppp-2.4.7
INSTALL_DIR = build/$(PKG)-install

CONFOPTS=
UCFRONT_ENV=

ifdef CONFIG_DEFAULTS_LIBC_MUSL
CFLAGS += -D__GLIBC__=2 -D__GLIBC_MINOR__=1
endif

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -e CONFIG_USER_PPPD_PPPD_PPPD $(INSTALL_DIR)/sbin/pppd /bin/pppd
	$(ROMFSINST) -e CONFIG_USER_CHAT_CHAT $(INSTALL_DIR)/sbin/chat /bin/chat
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_DYNAMIC_PLUGINS -e CONFIG_USER_PPPD_WITH_PPPOE -d $(INSTALL_DIR)/lib/pppd/2.4.7/rp-pppoe.so /etc/ppp/plugins/rp-pppoe.so
	$(ROMFSINST) -d -s /var/run/pppoe.conf /etc/ppp/pppoe.conf

ifdef CONFIG_USER_PPPD_WITH_RADIUS
	[ -d $(ROMFSDIR)/etc/radiusclient ] || mkdir $(ROMFSDIR)/etc/radiusclient
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary /etc/radiusclient/dictionary
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.ms /etc/radiusclient/dictionary.ms
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.sg /etc/radiusclient/dictionary.sg
endif
