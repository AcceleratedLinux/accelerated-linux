VERSION = 2.4.9
URL = https://github.com/paulusmack/ppp/archive/ppp-$(VERSION).tar.gz
BUILDDIR = ppp-ppp-$(VERSION)
INSTALL_DIR = build/$(PKG)-install

export ACL_PKG=ppp-$(VERSION)
export ACL_URL=https://github.com/paulusmack/ppp/commit/8d45443bb5c9372b4c6a362ba2f443d41c5636af

CONFOPTS=
UCFRONT_ENV=
MAKEVARS += COPTS="$(CFLAGS)"
AUTOMAKE_ROMFS = pppd_romfs

ifdef CONFIG_LIB_OPENSSL
#
# If we have openssl then use its crypto.
# At first we just needed this for newer glibc versions. But it just makes
# more sense to use it always if we have it.
#
CONFIG_USER_PPPD_USE_OPENSSL=y
endif

include $(ROOTDIR)/tools/automake.inc

pppd_romfs:
	$(ROMFSINST) -e CONFIG_USER_PPPD_PPPD_PPPD $(INSTALL_DIR)/sbin/pppd /bin/pppd
	$(ROMFSINST) -e CONFIG_USER_PPPD_PPPD_PPPD $(INSTALL_DIR)/sbin/pppdump /bin/pppdump
	$(ROMFSINST) -e CONFIG_USER_CHAT_CHAT $(INSTALL_DIR)/sbin/chat /bin/chat
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_DYNAMIC_PLUGINS -f $(INSTALL_DIR)/lib/pppd /lib/pppd

ifdef CONFIG_USER_PPPD_WITH_RADIUS
	[ -d $(ROMFSDIR)/etc/radiusclient ] || mkdir $(ROMFSDIR)/etc/radiusclient
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary /etc/radiusclient/dictionary
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.ms /etc/radiusclient/dictionary.ms
	$(ROMFSINST) -e CONFIG_USER_PPPD_WITH_RADIUS build/$(BUILDDIR)/scripts/radiusclient/dictionary.sg /etc/radiusclient/dictionary.sg
endif
